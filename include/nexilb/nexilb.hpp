#ifndef NEXILB_NEXILB_HPP
#define NEXILB_NEXILB_HPP

#include "nexilb/nexilb.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace nexilb {

namespace detail {

struct api_state final {
  nexilb_api_v1 api{};  /**< Negotiated table retained for wrapper lifetimes. */
};

using api_state_ptr = std::shared_ptr<const api_state>;

inline nexilb_buffer_t empty_buffer() noexcept {
  nexilb_buffer_t buffer{};
  buffer.struct_size = static_cast<std::uint32_t>(sizeof(buffer));
  buffer.struct_version = NEXILB_STRUCT_VERSION_1;
  return buffer;
}

inline nexilb_string_view_t string_view(std::string_view value) noexcept {
  return {value.data(), static_cast<std::uint64_t>(value.size())};
}

inline void destroy_error(const api_state_ptr &state,
                          nexilb_error_t &handle) noexcept {
  if (handle != nullptr && state && state->api.error_destroy != nullptr) {
    state->api.error_destroy(&handle);
  }
}

} // namespace detail

class library;
class catalog;
template <typename T> class result;

/** @brief Move-only owner of a runtime diagnostic handle. */
class error final {
public:
  /** @brief Construct an empty error owner. */
  error() noexcept = default;
  /** @brief Destroy the owned diagnostic, if any. */
  ~error() noexcept { reset(); }

  /** @brief Error owners cannot be copied. */
  error(const error &) = delete;
  /** @brief Error owners cannot be copy-assigned. */
  error &operator=(const error &) = delete;

  /** @brief Move a diagnostic handle from another owner. @param other Source owner. */
  error(error &&other) noexcept
      : state_(std::move(other.state_)), handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  /** @brief Replace this owner with another diagnostic. @param other Source owner. @return This owner. */
  error &operator=(error &&other) noexcept {
    if (this != &other) {
      reset();
      state_ = std::move(other.state_);
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  /** @brief Test whether a diagnostic is owned. @return True when non-empty. */
  explicit operator bool() const noexcept { return handle_ != nullptr; }
  /** @brief Test whether a diagnostic is owned. @return True when non-empty. */
  bool valid() const noexcept { return handle_ != nullptr; }
  /** @brief Borrow the native diagnostic handle. @return Borrowed handle. */
  nexilb_error_t get() const noexcept { return handle_; }

  /** @brief Borrow the negotiated function table. @return Table or nullptr. */
  const nexilb_api_v1 *api_table() const noexcept {
    return state_ ? &state_->api : nullptr;
  }

  /** @brief Check availability of formatted diagnostic text. @return True when supported. */
  bool can_format() const noexcept {
    return state_ && state_->api.error_format != nullptr;
  }

  /** @brief Check availability of structured diagnostic data. @return True when supported. */
  bool can_inspect() const noexcept {
    return state_ && state_->api.error_info != nullptr;
  }

  /** @brief Check availability of the native destroy slot. @return True when supported. */
  bool can_destroy() const noexcept {
    return state_ && state_->api.error_destroy != nullptr;
  }

  /** @brief Read structured diagnostic information. @return Information or status/error. */
  result<nexilb_error_info_t> info() const noexcept;
  /** @brief Format the diagnostic as UTF-8. @return Text or status/error. */
  result<std::string> format() const noexcept;

  /** @brief Relinquish ownership without destroying. @return Native handle. */
  nexilb_error_t release() noexcept {
    const auto released = handle_;
    handle_ = nullptr;
    state_.reset();
    return released;
  }

  /** @brief Destroy the owned diagnostic and become empty. */
  void reset() noexcept {
    detail::destroy_error(state_, handle_);
    state_.reset();
  }

private:
  error(detail::api_state_ptr state, nexilb_error_t handle) noexcept
      : state_(std::move(state)), handle_(handle) {}

  detail::api_state_ptr state_{};
  nexilb_error_t handle_ = nullptr;

  template <typename T> friend class result;
  friend result<nexilb_error_info_t> error_info(const error &) noexcept;
  friend result<std::string> format_error(const error &) noexcept;
};

/** @brief Move-only status, value, and diagnostic result. @tparam T Success value type. */
template <typename T> class result final {
public:
  /** @brief Results cannot be copied. */
  result(const result &) = delete;
  /** @brief Results cannot be copy-assigned. */
  result &operator=(const result &) = delete;
  /** @brief Move-construct a result. */
  result(result &&) noexcept(std::is_nothrow_move_constructible<T>::value) =
      default;
  /** @brief Move-assign a result. @return This result. */
  result &operator=(result &&) noexcept(
      std::is_nothrow_move_assignable<T>::value) = default;

  /** @brief Create a successful result. @param value Success value. @return Successful result. */
  static result success(T value) {
    return result(NEXILB_STATUS_OK, std::move(value), {});
  }

  /** @brief Create a failed result. @param status Failure status. @param state Function-table lifetime owner. @param diagnostic Optional diagnostic handle. @return Failed result. */
  static result failure(nexilb_status_t status,
                        detail::api_state_ptr state = {},
                        nexilb_error_t diagnostic = nullptr) noexcept {
    return result(status, std::nullopt,
                  error(std::move(state), diagnostic));
  }

  /** @brief Test success. @return True only for NEXILB_STATUS_OK. */
  bool ok() const noexcept { return status_ == NEXILB_STATUS_OK; }
  /** @brief Test success. @return True only for success. */
  explicit operator bool() const noexcept { return ok(); }
  /** @brief Read the native status. @return Status code. */
  nexilb_status_t status() const noexcept { return status_; }

  /** @brief Access the success value. @return Mutable lvalue reference. */
  T &value() & { return *value_; }
  /** @brief Access the success value. @return Const lvalue reference. */
  const T &value() const & { return *value_; }
  /** @brief Move the success value. @return Rvalue reference. */
  T &&value() && { return std::move(*value_); }

  /** @brief Access the diagnostic owner. @return Mutable diagnostic reference. */
  error &diagnostic() & noexcept { return diagnostic_; }
  /** @brief Access the diagnostic owner. @return Const diagnostic reference. */
  const error &diagnostic() const & noexcept { return diagnostic_; }
  /** @brief Move the diagnostic out. @return Diagnostic owner. */
  error take_diagnostic() noexcept { return std::move(diagnostic_); }

private:
  result(nexilb_status_t status, std::optional<T> value,
         error diagnostic) noexcept(
      std::is_nothrow_move_constructible<T>::value)
      : status_(status), value_(std::move(value)),
        diagnostic_(std::move(diagnostic)) {}

  nexilb_status_t status_ = NEXILB_STATUS_INTERNAL_ERROR;
  std::optional<T> value_{};
  error diagnostic_{};
};

/** @brief Move-only status and diagnostic result without a value. */
template <> class result<void> final {
public:
  /** @brief Results cannot be copied. */
  result(const result &) = delete;
  /** @brief Results cannot be copy-assigned. */
  result &operator=(const result &) = delete;
  /** @brief Move-construct a result. */
  result(result &&) noexcept = default;
  /** @brief Move-assign a result. @return This result. */
  result &operator=(result &&) noexcept = default;

  /** @brief Create a successful void result. @return Successful result. */
  static result success() noexcept { return result(NEXILB_STATUS_OK, {}); }

  /** @brief Create a failed void result. @param status Failure status. @param state Function-table lifetime owner. @param diagnostic Optional diagnostic. @return Failed result. */
  static result failure(nexilb_status_t status,
                        detail::api_state_ptr state = {},
                        nexilb_error_t diagnostic = nullptr) noexcept {
    return result(status, error(std::move(state), diagnostic));
  }

  /** @brief Test success. @return True only for NEXILB_STATUS_OK. */
  bool ok() const noexcept { return status_ == NEXILB_STATUS_OK; }
  /** @brief Test success. @return True only for success. */
  explicit operator bool() const noexcept { return ok(); }
  /** @brief Read the native status. @return Status code. */
  nexilb_status_t status() const noexcept { return status_; }

  /** @brief Access the diagnostic owner. @return Mutable diagnostic reference. */
  error &diagnostic() & noexcept { return diagnostic_; }
  /** @brief Access the diagnostic owner. @return Const diagnostic reference. */
  const error &diagnostic() const & noexcept { return diagnostic_; }
  /** @brief Move the diagnostic out. @return Diagnostic owner. */
  error take_diagnostic() noexcept { return std::move(diagnostic_); }

private:
  result(nexilb_status_t status, error diagnostic) noexcept
      : status_(status), diagnostic_(std::move(diagnostic)) {}

  nexilb_status_t status_ = NEXILB_STATUS_INTERNAL_ERROR;
  error diagnostic_{};
};

namespace detail {

template <typename Invoke>
inline result<std::string> read_utf8(const api_state_ptr &state,
                                     Invoke &&invoke) noexcept {
  auto buffer = empty_buffer();
  nexilb_error_t raw_error = nullptr;
  const auto size_status = invoke(&buffer, &raw_error);
  if (size_status != NEXILB_STATUS_OK) {
    return result<std::string>::failure(size_status, state, raw_error);
  }
  destroy_error(state, raw_error);

  if (buffer.required_bytes == 0u) {
    return result<std::string>::success({});
  }
  if (buffer.required_bytes >
      static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    return result<std::string>::failure(NEXILB_STATUS_RESOURCE_LIMIT);
  }

  std::string text;
  try {
    text.resize(static_cast<std::size_t>(buffer.required_bytes));
  } catch (...) {
    return result<std::string>::failure(NEXILB_STATUS_RESOURCE_LIMIT);
  }

  buffer.data = text.data();
  buffer.capacity_bytes = static_cast<std::uint64_t>(text.size());
  raw_error = nullptr;
  const auto read_status = invoke(&buffer, &raw_error);
  if (read_status != NEXILB_STATUS_OK) {
    return result<std::string>::failure(read_status, state, raw_error);
  }
  destroy_error(state, raw_error);

  if (buffer.required_bytes == 0u ||
      buffer.required_bytes > buffer.capacity_bytes ||
      text[static_cast<std::size_t>(buffer.required_bytes - 1u)] != '\0') {
    return result<std::string>::failure(NEXILB_STATUS_INTERNAL_ERROR);
  }
  text.resize(static_cast<std::size_t>(buffer.required_bytes - 1u));
  return result<std::string>::success(std::move(text));
}

} // namespace detail

/** @brief Read structured information from an error. @param value Error owner. @return Information or failure. */
inline result<nexilb_error_info_t> error_info(const error &value) noexcept {
  if (!value.valid()) {
    return result<nexilb_error_info_t>::failure(NEXILB_STATUS_INVALID_STATE);
  }
  const auto *api = value.api_table();
  if (api == nullptr || api->error_info == nullptr) {
    return result<nexilb_error_info_t>::failure(
        NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
  }
  nexilb_error_info_t info{};
  info.struct_size = static_cast<std::uint32_t>(sizeof(info));
  info.struct_version = NEXILB_STRUCT_VERSION_1;
  const auto status = api->error_info(value.get(), &info);
  if (status != NEXILB_STATUS_OK) {
    return result<nexilb_error_info_t>::failure(status);
  }
  return result<nexilb_error_info_t>::success(info);
}

/** @brief Format an error as UTF-8. @param value Error owner. @return Text or failure. */
inline result<std::string> format_error(const error &value) noexcept {
  if (!value.valid()) {
    return result<std::string>::failure(NEXILB_STATUS_INVALID_STATE);
  }
  const auto *api = value.api_table();
  if (api == nullptr || api->error_format == nullptr) {
    return result<std::string>::failure(
        NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
  }
  return detail::read_utf8(
      value.state_, [&value, api](nexilb_buffer_t *buffer,
                                  nexilb_error_t *) noexcept {
        return api->error_format(value.get(), buffer);
      });
}

inline result<nexilb_error_info_t> error::info() const noexcept {
  return error_info(*this);
}

inline result<std::string> error::format() const noexcept {
  return format_error(*this);
}

/** @brief Move-only owner of a library or effective catalog handle. */
class catalog final {
public:
  /** @brief Construct an empty catalog owner. */
  catalog() noexcept = default;
  /** @brief Destroy the owned catalog, if any. */
  ~catalog() noexcept { reset_noexcept(); }

  /** @brief Catalog owners cannot be copied. */
  catalog(const catalog &) = delete;
  /** @brief Catalog owners cannot be copy-assigned. */
  catalog &operator=(const catalog &) = delete;

  /** @brief Move a catalog handle. @param other Source owner. */
  catalog(catalog &&other) noexcept
      : state_(std::move(other.state_)), handle_(other.handle_) {
    other.handle_ = nullptr;
  }

  /** @brief Replace this owner with another catalog. @param other Source owner. @return This owner. */
  catalog &operator=(catalog &&other) noexcept {
    if (this != &other) {
      reset_noexcept();
      state_ = std::move(other.state_);
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  /** @brief Test whether a catalog is owned. @return True when non-empty. */
  explicit operator bool() const noexcept { return handle_ != nullptr; }
  /** @brief Test whether a catalog is owned. @return True when non-empty. */
  bool valid() const noexcept { return handle_ != nullptr; }
  /** @brief Borrow the native catalog handle. @return Borrowed handle. */
  nexilb_catalog_t get() const noexcept { return handle_; }

  /** @brief Borrow the negotiated API table. @return Table or nullptr. */
  const nexilb_api_v1 *api_table() const noexcept {
    return state_ ? &state_->api : nullptr;
  }

  /** @brief Read catalog identity and digest information. @return Information or failure. */
  result<nexilb_catalog_info_t> info() const noexcept {
    if (!valid()) {
      return result<nexilb_catalog_info_t>::failure(
          NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.catalog_info == nullptr) {
      return result<nexilb_catalog_info_t>::failure(
          NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    nexilb_catalog_info_t value{};
    value.struct_size = static_cast<std::uint32_t>(sizeof(value));
    value.struct_version = NEXILB_STRUCT_VERSION_1;
    nexilb_error_t raw_error = nullptr;
    const auto status =
        state_->api.catalog_info(handle_, &value, &raw_error);
    if (status != NEXILB_STATUS_OK) {
      return result<nexilb_catalog_info_t>::failure(status, state_, raw_error);
    }
    detail::destroy_error(state_, raw_error);
    return result<nexilb_catalog_info_t>::success(value);
  }

  /** @brief Count entries of one catalog kind. @param kind Entry kind. @return Count or failure. */
  result<std::uint64_t> count(nexilb_catalog_kind_t kind) const noexcept {
    if (!valid()) {
      return result<std::uint64_t>::failure(NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.catalog_count == nullptr) {
      return result<std::uint64_t>::failure(
          NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    std::uint64_t value = 0u;
    nexilb_error_t raw_error = nullptr;
    const auto status =
        state_->api.catalog_count(handle_, kind, &value, &raw_error);
    if (status != NEXILB_STATUS_OK) {
      return result<std::uint64_t>::failure(status, state_, raw_error);
    }
    detail::destroy_error(state_, raw_error);
    return result<std::uint64_t>::success(value);
  }

  /** @brief Read an entry ID by kind and stable catalog index. @param kind Entry kind. @param index Zero-based index. @return UTF-8 ID or failure. */
  result<std::string> id(nexilb_catalog_kind_t kind,
                         std::uint64_t index) const noexcept {
    if (!valid()) {
      return result<std::string>::failure(NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.catalog_id == nullptr) {
      return result<std::string>::failure(
          NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    return detail::read_utf8(
        state_, [this, kind, index](nexilb_buffer_t *buffer,
                                    nexilb_error_t *raw_error) noexcept {
          return state_->api.catalog_id(handle_, kind, index, buffer,
                                        raw_error);
        });
  }

  /** @brief Read exact descriptor JSON bytes for an entity. @param entity_id Exact entity ID. @return UTF-8 JSON or failure. */
  result<std::string> descriptor_json(std::string_view entity_id) const
      noexcept {
    if (!valid()) {
      return result<std::string>::failure(NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.catalog_descriptor_json == nullptr) {
      return result<std::string>::failure(
          NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    return detail::read_utf8(
        state_, [this, entity_id](nexilb_buffer_t *buffer,
                                  nexilb_error_t *raw_error) noexcept {
          return state_->api.catalog_descriptor_json(
              handle_, detail::string_view(entity_id), buffer, raw_error);
        });
  }

  /** @brief Read a schema document by ID. @param schema_id Exact schema ID. @return UTF-8 JSON or failure. */
  result<std::string> schema_json(std::string_view schema_id) const noexcept {
    if (!valid()) {
      return result<std::string>::failure(NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.catalog_schema_json == nullptr) {
      return result<std::string>::failure(
          NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    return detail::read_utf8(
        state_, [this, schema_id](nexilb_buffer_t *buffer,
                                  nexilb_error_t *raw_error) noexcept {
          return state_->api.catalog_schema_json(
              handle_, detail::string_view(schema_id), buffer, raw_error);
        });
  }

  /** @brief Destroy the catalog with status reporting. @return Success or failure. */
  result<void> close() noexcept {
    if (handle_ == nullptr) {
      return result<void>::success();
    }
    if (!state_ || state_->api.catalog_destroy == nullptr) {
      return result<void>::failure(NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    nexilb_error_t raw_error = nullptr;
    const auto status = state_->api.catalog_destroy(&handle_, &raw_error);
    if (status != NEXILB_STATUS_OK) {
      return result<void>::failure(status, state_, raw_error);
    }
    detail::destroy_error(state_, raw_error);
    state_.reset();
    return result<void>::success();
  }

  /** @brief Relinquish ownership without destroying. @return Native handle. */
  nexilb_catalog_t release() noexcept {
    const auto released = handle_;
    handle_ = nullptr;
    state_.reset();
    return released;
  }

private:
  catalog(detail::api_state_ptr state, nexilb_catalog_t handle) noexcept
      : state_(std::move(state)), handle_(handle) {}

  void reset_noexcept() noexcept {
    if (handle_ != nullptr && state_ && state_->api.catalog_destroy != nullptr) {
      nexilb_error_t raw_error = nullptr;
      (void)state_->api.catalog_destroy(&handle_, &raw_error);
      detail::destroy_error(state_, raw_error);
    }
    state_.reset();
  }

  detail::api_state_ptr state_{};
  nexilb_catalog_t handle_ = nullptr;

  friend class library;
};

/** @brief Shared lifetime owner of a negotiated NexiLB API v1 table. */
class library final {
public:
  /** @brief Construct an empty library wrapper. */
  library() noexcept = default;

  /** @brief Test whether bootstrap succeeded. @return True when a table is owned. */
  bool valid() const noexcept { return static_cast<bool>(state_); }
  /** @brief Test whether bootstrap succeeded. @return True when valid. */
  explicit operator bool() const noexcept { return valid(); }

  /** @brief Borrow the negotiated API table. @return Table or nullptr. */
  const nexilb_api_v1 *api_table() const noexcept {
    return state_ ? &state_->api : nullptr;
  }

  /** @brief Test a selected API function slot. @tparam Function Function-pointer type. @param member Member pointer selecting a slot. @return True when non-NULL. */
  template <typename Function>
  bool function_available(Function nexilb_api_v1::*member) const noexcept {
    static_assert(std::is_pointer<Function>::value,
                  "member must select a function pointer in nexilb_api_v1");
    return state_ && state_->api.*member != nullptr;
  }

  /** @brief Read library identity and capability flags. @return Information or failure. */
  result<nexilb_library_info_t> info() const noexcept {
    if (!valid()) {
      return result<nexilb_library_info_t>::failure(
          NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.library_info == nullptr) {
      return result<nexilb_library_info_t>::failure(
          NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    nexilb_library_info_t value{};
    value.struct_size = static_cast<std::uint32_t>(sizeof(value));
    value.struct_version = NEXILB_STRUCT_VERSION_1;
    nexilb_error_t raw_error = nullptr;
    const auto status = state_->api.library_info(&value, &raw_error);
    if (status != NEXILB_STATUS_OK) {
      return result<nexilb_library_info_t>::failure(status, state_, raw_error);
    }
    detail::destroy_error(state_, raw_error);
    return result<nexilb_library_info_t>::success(value);
  }

  /** @brief Create the process-wide library catalog. @return Owned catalog or failure. */
  result<catalog> create_catalog() const noexcept {
    if (!valid()) {
      return result<catalog>::failure(NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.library_catalog_create == nullptr) {
      return result<catalog>::failure(NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    nexilb_catalog_t raw_catalog = nullptr;
    nexilb_error_t raw_error = nullptr;
    const auto status =
        state_->api.library_catalog_create(&raw_catalog, &raw_error);
    if (status != NEXILB_STATUS_OK) {
      return result<catalog>::failure(status, state_, raw_error);
    }
    detail::destroy_error(state_, raw_error);
    return result<catalog>::success(catalog(state_, raw_catalog));
  }

  /** @brief Count usable runtime devices. @return Device count or failure. */
  result<std::uint32_t> device_count() const noexcept {
    if (!valid()) {
      return result<std::uint32_t>::failure(NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.device_count == nullptr) {
      return result<std::uint32_t>::failure(
          NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    std::uint32_t value = 0u;
    nexilb_error_t raw_error = nullptr;
    const auto status = state_->api.device_count(&value, &raw_error);
    if (status != NEXILB_STATUS_OK) {
      return result<std::uint32_t>::failure(status, state_, raw_error);
    }
    detail::destroy_error(state_, raw_error);
    return result<std::uint32_t>::success(value);
  }

  /** @brief Read one device descriptor as UTF-8 JSON. @param ordinal Zero-based device ordinal. @return JSON or failure. */
  result<std::string> device_info_json(std::uint32_t ordinal) const noexcept {
    if (!valid()) {
      return result<std::string>::failure(NEXILB_STATUS_INVALID_STATE);
    }
    if (state_->api.device_info_json == nullptr) {
      return result<std::string>::failure(
          NEXILB_STATUS_UNSUPPORTED_CAPABILITY);
    }
    return detail::read_utf8(
        state_, [this, ordinal](nexilb_buffer_t *buffer,
                                nexilb_error_t *raw_error) noexcept {
          return state_->api.device_info_json(ordinal, buffer, raw_error);
        });
  }

private:
  explicit library(detail::api_state_ptr state) noexcept
      : state_(std::move(state)) {}

  detail::api_state_ptr state_{};

  friend result<library> bootstrap(std::uint32_t) noexcept;
};

/** @brief Negotiate ABI compatibility and acquire an API table. @param requested_api Requested API version. @return Library wrapper or failure. */
inline result<library> bootstrap(
    std::uint32_t requested_api = NEXILB_API_V1) noexcept {
  nexilb_abi_version_t abi{};
  const auto abi_status = nexilb_get_abi_version(&abi);
  if (abi_status != NEXILB_STATUS_OK) {
    return result<library>::failure(abi_status);
  }
  if (abi.abi_major != NEXILB_ABI_MAJOR || requested_api < abi.api_min ||
      requested_api > abi.api_max) {
    return result<library>::failure(NEXILB_STATUS_UNSUPPORTED_VERSION);
  }
  nexilb_api_v1 api{};
  const auto status =
      nexilb_get_api(requested_api, static_cast<std::uint64_t>(sizeof(api)),
                     &api);
  if (status != NEXILB_STATUS_OK) {
    return result<library>::failure(status);
  }
  if (api.struct_version != NEXILB_STRUCT_VERSION_1 ||
      api.api_version != requested_api ||
      api.struct_size < NEXILB_API_V1_MANDATORY_PREFIX_SIZE) {
    return result<library>::failure(NEXILB_STATUS_UNSUPPORTED_VERSION);
  }
  try {
    auto state = std::make_shared<detail::api_state>();
    state->api = api;
    return result<library>::success(library(std::move(state)));
  } catch (...) {
    return result<library>::failure(NEXILB_STATUS_RESOURCE_LIMIT);
  }
}

} // namespace nexilb

#endif
