#ifndef NEXILB_EXAMPLE_CASE_CONTRACT_HPP
#define NEXILB_EXAMPLE_CASE_CONTRACT_HPP

#include <array>
#include <cstddef>
#include <string_view>

namespace nexilb::example {

template <std::size_t ModelCount, std::size_t CapabilityCount>
struct case_contract final {
  std::string_view case_id;
  std::array<std::string_view, ModelCount> model_ids;
  std::array<std::string_view, CapabilityCount> required_capabilities;
  bool requires_checkpoint_restart;
  bool requires_particle_input;
};

constexpr bool stable_id(std::string_view value) noexcept {
  if (value.empty()) {
    return false;
  }
  for (const char character : value) {
    const bool valid = (character >= 'A' && character <= 'Z') ||
                       (character >= 'a' && character <= 'z') ||
                       (character >= '0' && character <= '9') ||
                       character == '.' || character == '-' || character == '_';
    if (!valid) {
      return false;
    }
  }
  return true;
}

template <std::size_t M, std::size_t C>
constexpr bool valid(const case_contract<M, C> &contract) noexcept {
  if (!stable_id(contract.case_id)) {
    return false;
  }
  for (const auto model : contract.model_ids) {
    if (!stable_id(model)) {
      return false;
    }
  }
  for (const auto capability : contract.required_capabilities) {
    if (!stable_id(capability)) {
      return false;
    }
  }
  return true;
}

} // namespace nexilb::example

#endif

