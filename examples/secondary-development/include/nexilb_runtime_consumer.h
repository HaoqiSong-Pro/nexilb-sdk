#ifndef NEXILB_RUNTIME_CONSUMER_H
#define NEXILB_RUNTIME_CONSUMER_H

#include <nexilb/nexilb_case_sdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
typedef HMODULE nexilb_module_t;
#else
#include <dlfcn.h>
typedef void *nexilb_module_t;
#endif

#define NEXILB_CONSUMER_BLOCKED 64

typedef nexilb_status_t(NEXILB_CALL *nexilb_get_abi_version_proc)(
    nexilb_abi_version_t *);
typedef nexilb_status_t(NEXILB_CALL *nexilb_get_api_proc)(
    uint32_t, uint64_t, nexilb_api_v1 *);

static void nexilb_consumer_blocked(const char *reason) {
  fprintf(stderr, "BLOCKED: %s\n", reason);
}

static nexilb_module_t nexilb_consumer_load(const char *path) {
#if defined(_WIN32)
  return LoadLibraryA(path);
#else
  return dlopen(path, RTLD_NOW | RTLD_LOCAL);
#endif
}

static void nexilb_consumer_unload(nexilb_module_t module) {
#if defined(_WIN32)
  if (module != NULL) FreeLibrary(module);
#else
  if (module != NULL) dlclose(module);
#endif
}

static void *nexilb_consumer_symbol(nexilb_module_t module, const char *name) {
#if defined(_WIN32)
  return (void *)(uintptr_t)GetProcAddress(module, name);
#else
  return dlsym(module, name);
#endif
}

static int nexilb_consumer_relative_path(const char *path) {
  const char *segment;
  const char *cursor;
  if (path == NULL || path[0] == '\0' || path[0] == '/' || path[0] == '\\')
    return 0;
  if (strlen(path) >= 2u && path[1] == ':') return 0;
  if (strchr(path, '\\') != NULL || strchr(path, '"') != NULL) return 0;
  for (cursor = path; *cursor != '\0'; ++cursor) {
    if ((unsigned char)*cursor < 0x20u) return 0;
  }
  segment = path;
  for (cursor = path;; ++cursor) {
    if (*cursor == '/' || *cursor == '\0') {
      size_t length = (size_t)(cursor - segment);
      if (length == 0u || (length == 1u && segment[0] == '.') ||
          (length == 2u && segment[0] == '.' && segment[1] == '.'))
        return 0;
      if (*cursor == '\0') break;
      segment = cursor + 1;
    }
  }
  return 1;
}

static void nexilb_consumer_release_error(const nexilb_api_v1 *api,
                                          const char *operation,
                                          nexilb_status_t status,
                                          nexilb_error_t *error) {
  fprintf(stderr, "BLOCKED: %s returned status %d", operation, (int)status);
  if (error != NULL && *error != NULL && api->error_info != NULL) {
    nexilb_error_info_t info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = NEXILB_STRUCT_VERSION_1;
    if (api->error_info(*error, &info) == NEXILB_STATUS_OK) {
      fprintf(stderr, " (subsystem=%u, entity=%s, pointer=%s)",
              info.subsystem, info.entity_id, info.json_pointer);
    }
  }
  fputc('\n', stderr);
  if (error != NULL && *error != NULL && api->error_destroy != NULL)
    api->error_destroy(error);
}

static int nexilb_consumer_call_ok(const nexilb_api_v1 *api,
                                   const char *operation,
                                   nexilb_status_t status,
                                   nexilb_error_t *error) {
  if (status == NEXILB_STATUS_OK && (error == NULL || *error == NULL)) return 1;
  nexilb_consumer_release_error(api, operation, status, error);
  return 0;
}

static int nexilb_consumer_catalog_has_model(const nexilb_api_v1 *api,
                                             nexilb_catalog_t catalog,
                                             const char *model_id) {
  uint64_t count = 0u;
  uint64_t index;
  nexilb_error_t error = NULL;
  if (!nexilb_consumer_call_ok(
          api, "catalog_count",
          api->catalog_count(catalog, NEXILB_CATALOG_MODEL, &count, &error),
          &error))
    return 0;
  if (count > UINT64_C(100000)) {
    nexilb_consumer_blocked("catalog model count exceeds the consumer bound");
    return 0;
  }
  for (index = 0u; index < count; ++index) {
    nexilb_buffer_t buffer;
    char *text;
    memset(&buffer, 0, sizeof(buffer));
    buffer.struct_size = (uint32_t)sizeof(buffer);
    buffer.struct_version = NEXILB_STRUCT_VERSION_1;
    error = NULL;
    if (!nexilb_consumer_call_ok(
            api, "catalog_id(size)",
            api->catalog_id(catalog, NEXILB_CATALOG_MODEL, index, &buffer,
                            &error),
            &error))
      return 0;
    if (buffer.required_bytes == 0u ||
        buffer.required_bytes > UINT64_C(1048576)) {
      nexilb_consumer_blocked("catalog ID size is outside the consumer bound");
      return 0;
    }
    text = (char *)malloc((size_t)buffer.required_bytes);
    if (text == NULL) {
      nexilb_consumer_blocked("cannot allocate the catalog ID buffer");
      return 0;
    }
    buffer.data = text;
    buffer.capacity_bytes = buffer.required_bytes;
    error = NULL;
    if (!nexilb_consumer_call_ok(
            api, "catalog_id(value)",
            api->catalog_id(catalog, NEXILB_CATALOG_MODEL, index, &buffer,
                            &error),
            &error)) {
      free(text);
      return 0;
    }
    if (text[buffer.required_bytes - 1u] != '\0') {
      free(text);
      nexilb_consumer_blocked("catalog ID is not NUL-terminated");
      return 0;
    }
    if (strcmp(text, model_id) == 0) {
      free(text);
      return 1;
    }
    free(text);
  }
  nexilb_consumer_blocked("the effective catalog does not expose the exact model ID");
  return 0;
}

static int nexilb_consumer_expect_state(const nexilb_api_v1 *api,
                                        nexilb_model_t model,
                                        nexilb_model_state_t first,
                                        nexilb_model_state_t second,
                                        const char *operation) {
  nexilb_model_state_info_t state;
  nexilb_error_t error = NULL;
  memset(&state, 0, sizeof(state));
  state.struct_size = (uint32_t)sizeof(state);
  state.struct_version = NEXILB_STRUCT_VERSION_1;
  if (!nexilb_consumer_call_ok(api, operation,
                               api->model_state(model, &state, &error),
                               &error))
    return 0;
  if (state.state != first && state.state != second) {
    nexilb_consumer_blocked("model lifecycle state is inconsistent");
    return 0;
  }
  return 1;
}

static int nexilb_consumer_validate_clear(
    const nexilb_api_v1 *api, nexilb_model_t model,
    nexilb_model_report_fn validate, const char *operation) {
  nexilb_validation_report_t report = NULL;
  nexilb_error_t error = NULL;
  uint64_t count = 0u;
  if (validate == NULL || api->validation_report_count == NULL ||
      api->validation_report_destroy == NULL) {
    nexilb_consumer_blocked("validation report slots are incomplete");
    return 0;
  }
  if (!nexilb_consumer_call_ok(
          api, operation, validate(model, &report, &error), &error))
    return 0;
  if (report == NULL ||
      api->validation_report_count(report, &count) != NEXILB_STATUS_OK) {
    nexilb_consumer_blocked("validation report cannot be inspected");
    if (report != NULL) api->validation_report_destroy(&report);
    return 0;
  }
  if (count != 0u) {
    nexilb_consumer_blocked("configuration validation returned diagnostics");
    api->validation_report_destroy(&report);
    return 0;
  }
  if (api->validation_report_destroy(&report) != NEXILB_STATUS_OK ||
      report != NULL) {
    nexilb_consumer_blocked("validation report destroy failed");
    return 0;
  }
  return 1;
}

static int nexilb_consumer_read_array(
    const nexilb_api_v1 *api, nexilb_snapshot_t snapshot,
    nexilb_snapshot_read_fn read, const char *attribute,
    nexilb_association_t expected_association, int allow_empty) {
  nexilb_array_view_t array;
  nexilb_string_view_t id;
  nexilb_error_t error = NULL;
  void *storage = NULL;
  uint64_t required;
  if (read == NULL) {
    nexilb_consumer_blocked("required snapshot read slot is NULL");
    return 0;
  }
  memset(&array, 0, sizeof(array));
  array.struct_size = (uint32_t)sizeof(array);
  array.struct_version = NEXILB_STRUCT_VERSION_1;
  id.data = attribute;
  id.byte_size = (uint64_t)strlen(attribute);
  if (!nexilb_consumer_call_ok(
          api, "snapshot array query", read(snapshot, id, &array, &error),
          &error))
    return 0;
  required = array.capacity_bytes;
  if (array.association != expected_association ||
      (!allow_empty && (array.element_count == 0u || required == 0u)) ||
      required > UINT64_C(1073741824)) {
    nexilb_consumer_blocked("snapshot array metadata is outside the case contract");
    return 0;
  }
  if (required == 0u) return allow_empty;
  storage = malloc((size_t)required);
  if (storage == NULL) {
    nexilb_consumer_blocked("cannot allocate snapshot array buffer");
    return 0;
  }
  array.data = storage;
  array.capacity_bytes = required;
  error = NULL;
  if (!nexilb_consumer_call_ok(
          api, "snapshot array copy", read(snapshot, id, &array, &error),
          &error)) {
    free(storage);
    return 0;
  }
  free(storage);
  return 1;
}

static int nexilb_run_case(const char *runtime_path,
                           const char *config_path,
                           const char *model_id,
                           const char *checkpoint_path) {
  nexilb_module_t module = NULL;
  nexilb_get_abi_version_proc get_abi = NULL;
  nexilb_get_api_proc get_api = NULL;
  nexilb_abi_version_t abi;
  nexilb_api_v1 api;
  nexilb_library_info_t library_info;
  nexilb_catalog_t catalog = NULL;
  nexilb_catalog_t effective_catalog = NULL;
  nexilb_context_t context = NULL;
  nexilb_model_t model = NULL;
  nexilb_error_t error = NULL;
  nexilb_context_options_t context_options;
  nexilb_step_result_t step;
  nexilb_string_view_t model_view;
  nexilb_string_view_t config_view;
  char *config_json = NULL;
  FILE *config_file = NULL;
  uint32_t device_count = 0u;
  int result = NEXILB_CONSUMER_BLOCKED;
  const uint64_t required_flags =
      NEXILB_API_FLAG_LIBRARY_CATALOG_AVAILABLE |
      NEXILB_API_FLAG_ERROR_OBJECTS_AVAILABLE |
      NEXILB_API_FLAG_DEVICE_ENUMERATION_AVAILABLE;

  if (!nexilb_consumer_relative_path(config_path)) {
    nexilb_consumer_blocked(
        "config path must be a normalized case-root-relative path using '/'");
    return result;
  }
  if (checkpoint_path != NULL &&
      !nexilb_consumer_relative_path(checkpoint_path)) {
    nexilb_consumer_blocked(
        "checkpoint path must be normalized and case-root-relative");
    return result;
  }
  config_file = fopen(config_path, "rb");
  if (config_file == NULL) {
    nexilb_consumer_blocked(
        "config is not visible from the current case-package working directory");
    return result;
  }
  fclose(config_file);

  module = nexilb_consumer_load(runtime_path);
  if (module == NULL) {
    nexilb_consumer_blocked("runtime dynamic library could not be loaded");
    goto cleanup;
  }
  {
    void *symbol = nexilb_consumer_symbol(module, "nexilb_get_abi_version");
    if (symbol != NULL && sizeof(symbol) == sizeof(get_abi))
      memcpy(&get_abi, &symbol, sizeof(get_abi));
    symbol = nexilb_consumer_symbol(module, "nexilb_get_api");
    if (symbol != NULL && sizeof(symbol) == sizeof(get_api))
      memcpy(&get_api, &symbol, sizeof(get_api));
  }
  if (get_abi == NULL || get_api == NULL) {
    nexilb_consumer_blocked("runtime lacks a required bootstrap symbol");
    goto cleanup;
  }

  memset(&abi, 0, sizeof(abi));
  if (get_abi(&abi) != NEXILB_STATUS_OK || abi.abi_major != NEXILB_ABI_MAJOR ||
      abi.api_min > NEXILB_API_V1 || abi.api_max < NEXILB_API_V1) {
    nexilb_consumer_blocked("runtime ABI/API interval is incompatible");
    goto cleanup;
  }
  memset(&api, 0, sizeof(api));
  if (get_api(NEXILB_API_V1, (uint64_t)sizeof(api), &api) !=
          NEXILB_STATUS_OK ||
      api.api_version != NEXILB_API_V1 ||
      api.struct_size < NEXILB_API_V1_MANDATORY_PREFIX_SIZE) {
    nexilb_consumer_blocked("runtime rejected the API v1 table negotiation");
    goto cleanup;
  }
  if (nexilb_case_config_path_slots_available(&api) != NEXILB_TRUE) {
    nexilb_consumer_blocked("a required configuration-file lifecycle slot is NULL");
    goto cleanup;
  }

  memset(&library_info, 0, sizeof(library_info));
  library_info.struct_size = (uint32_t)sizeof(library_info);
  library_info.struct_version = NEXILB_STRUCT_VERSION_1;
  if (!nexilb_consumer_call_ok(
          &api, "library_info", api.library_info(&library_info, &error),
          &error))
    goto cleanup;
  if (library_info.abi_major != NEXILB_ABI_MAJOR ||
      (library_info.api_flags & required_flags) != required_flags ||
      (api.api_flags & required_flags) != required_flags) {
    nexilb_consumer_blocked("runtime identity lacks required discovery flags");
    goto cleanup;
  }
  if (!nexilb_consumer_call_ok(
          &api, "library_catalog_create",
          api.library_catalog_create(&catalog, &error), &error))
    goto cleanup;
  if (!nexilb_consumer_catalog_has_model(&api, catalog, model_id)) goto cleanup;

  if (!nexilb_consumer_call_ok(
          &api, "device_count", api.device_count(&device_count, &error),
          &error))
    goto cleanup;
  if (device_count == 0u) {
    nexilb_consumer_blocked("runtime reports no usable device");
    goto cleanup;
  }
  memset(&context_options, 0, sizeof(context_options));
  context_options.struct_size = (uint32_t)sizeof(context_options);
  context_options.struct_version = NEXILB_STRUCT_VERSION_1;
  context_options.device_ordinal = 0;
  context_options.flags = NEXILB_CONTEXT_FLAGS_NONE;
  if (!nexilb_consumer_call_ok(
          &api, "context_create",
          api.context_create(&context_options, &context, &error), &error))
    goto cleanup;
  if (api.context_catalog_create != NULL) {
    if (!nexilb_consumer_call_ok(
            &api, "context_catalog_create",
            api.context_catalog_create(context, &effective_catalog, &error),
            &error))
      goto cleanup;
    if (!nexilb_consumer_catalog_has_model(&api, effective_catalog, model_id))
      goto cleanup;
  }

  model_view.data = model_id;
  model_view.byte_size = (uint64_t)strlen(model_id);
  if (!nexilb_consumer_call_ok(
          &api, "model_create",
          api.model_create(context, model_view, &model, &error), &error))
    goto cleanup;
  if (api.model_state != NULL &&
      !nexilb_consumer_expect_state(&api, model, NEXILB_MODEL_STATE_EMPTY,
                                    NEXILB_MODEL_STATE_EMPTY,
                                    "model_state(after create)"))
    goto cleanup;

  {
    size_t needed = strlen(config_path) + sizeof("{\"config_path\":\"\"}");
    config_json = (char *)malloc(needed);
    if (config_json == NULL) {
      nexilb_consumer_blocked("cannot allocate configuration JSON");
      goto cleanup;
    }
    if (snprintf(config_json, needed, "{\"config_path\":\"%s\"}",
                 config_path) < 0) {
      nexilb_consumer_blocked("cannot construct configuration JSON");
      goto cleanup;
    }
  }
  config_view.data = config_json;
  config_view.byte_size = (uint64_t)strlen(config_json);
  if (!nexilb_consumer_call_ok(
          &api, "model_set_config_json",
          api.model_set_config_json(model, config_view, &error), &error))
    goto cleanup;
  if (api.model_state != NULL &&
      !nexilb_consumer_expect_state(&api, model,
                                    NEXILB_MODEL_STATE_CONFIGURED,
                                    NEXILB_MODEL_STATE_VALIDATED,
                                    "model_state(after config)"))
    goto cleanup;
  /* Validation is an optional strengthening path.  The current local runtime
     leaves these slots NULL, so their absence must not block the documented
     config_path lifecycle.  A future implementation is used only as a
     complete group; a partial group fails closed instead of being guessed. */
  if (nexilb_case_validation_slots_available(&api) == NEXILB_TRUE) {
    if (!nexilb_consumer_validate_clear(
            &api, model, api.model_validate, "model_validate") ||
        !nexilb_consumer_validate_clear(
            &api, model, api.model_geometry_check, "model_geometry_check"))
      goto cleanup;
  } else if (api.model_validate != NULL || api.model_geometry_check != NULL ||
             api.validation_report_count != NULL ||
             api.validation_report_destroy != NULL) {
    nexilb_consumer_blocked("validation slot group is only partially populated");
    goto cleanup;
  } else {
    fprintf(stderr,
            "OPTIONAL-SKIPPED: validation slots are NULL; physical acceptance "
            "remains not_evaluated\n");
  }
  if (!nexilb_consumer_call_ok(
          &api, "model_initialize", api.model_initialize(model, &error),
          &error))
    goto cleanup;
  if (api.model_state != NULL &&
      !nexilb_consumer_expect_state(&api, model, NEXILB_MODEL_STATE_READY,
                                    NEXILB_MODEL_STATE_FINISHED,
                                    "model_state(after initialize)"))
    goto cleanup;

  memset(&step, 0, sizeof(step));
  step.struct_size = (uint32_t)sizeof(step);
  step.struct_version = NEXILB_STRUCT_VERSION_1;
  if (!nexilb_consumer_call_ok(
          &api, "model_step", api.model_step(model, &step, &error), &error))
    goto cleanup;
  if (api.model_state != NULL &&
      !nexilb_consumer_expect_state(&api, model, NEXILB_MODEL_STATE_READY,
                                    NEXILB_MODEL_STATE_FINISHED,
                                    "model_state(after step)"))
    goto cleanup;
  if (nexilb_case_snapshot_field_slots_available(&api) == NEXILB_TRUE) {
    nexilb_snapshot_t snapshot = NULL;
    nexilb_snapshot_info_t snapshot_info;
    error = NULL;
    if (!nexilb_consumer_call_ok(
            &api, "snapshot_create",
            api.snapshot_create(model, &snapshot, &error), &error))
      goto cleanup;
    memset(&snapshot_info, 0, sizeof(snapshot_info));
    snapshot_info.struct_size = (uint32_t)sizeof(snapshot_info);
    snapshot_info.struct_version = NEXILB_STRUCT_VERSION_1;
    if (!nexilb_consumer_call_ok(
            &api, "snapshot_info",
            api.snapshot_info(snapshot, &snapshot_info, &error), &error) ||
        snapshot_info.macro_step != step.macro_step ||
        snapshot_info.exact_tick != step.exact_tick ||
        !nexilb_consumer_read_array(
            &api, snapshot, api.field_read, "field.phase_fraction",
            NEXILB_ASSOCIATION_CELL, 0)) {
      api.snapshot_destroy(&snapshot, NULL);
      goto cleanup;
    }
    if (strcmp(model_id, "model.NPhaseImbDemContactAngle") == 0) {
      if (nexilb_case_snapshot_particle_slots_available(&api) == NEXILB_TRUE) {
        if (!nexilb_consumer_read_array(
                &api, snapshot, api.particle_read, "particle.id",
                NEXILB_ASSOCIATION_PARTICLE, 0)) {
          api.snapshot_destroy(&snapshot, NULL);
          goto cleanup;
        }
      } else {
        fprintf(stderr,
                "OPTIONAL-SKIPPED: particle snapshot slot is NULL\n");
      }
      if (nexilb_case_snapshot_contact_slots_available(&api) == NEXILB_TRUE) {
        if (!nexilb_consumer_read_array(
                &api, snapshot, api.contact_read, "wall_contact.active",
                NEXILB_ASSOCIATION_CONTACT, 1)) {
          api.snapshot_destroy(&snapshot, NULL);
          goto cleanup;
        }
      } else {
        fprintf(stderr,
                "OPTIONAL-SKIPPED: wall-contact snapshot slot is NULL\n");
      }
    }
    if (api.snapshot_destroy(&snapshot, NULL) != NEXILB_STATUS_OK ||
        snapshot != NULL) {
      nexilb_consumer_blocked("snapshot destroy failed");
      goto cleanup;
    }
  } else {
    fprintf(stderr,
            "OPTIONAL-SKIPPED: snapshot slots are NULL; no output or physical "
            "verification is claimed\n");
  }
  if (checkpoint_path != NULL) {
    nexilb_checkpoint_info_t checkpoint_info;
    nexilb_model_t restored = NULL;
    nexilb_model_state_info_t restored_state;
    nexilb_step_result_t restored_step;
    nexilb_string_view_t checkpoint_view;
    if (api.checkpoint_info == NULL || api.checkpoint_write == NULL ||
        api.checkpoint_read == NULL || api.model_reset == NULL) {
      nexilb_consumer_blocked("required checkpoint slot is NULL");
      goto cleanup;
    }
    checkpoint_view.data = checkpoint_path;
    checkpoint_view.byte_size = (uint64_t)strlen(checkpoint_path);
    error = NULL;
    if (!nexilb_consumer_call_ok(
            &api, "checkpoint_write",
            api.checkpoint_write(model, checkpoint_view, 0u, &error),
            &error))
      goto cleanup;
    memset(&checkpoint_info, 0, sizeof(checkpoint_info));
    checkpoint_info.struct_size = (uint32_t)sizeof(checkpoint_info);
    checkpoint_info.struct_version = NEXILB_STRUCT_VERSION_1;
    if (!nexilb_consumer_call_ok(
            &api, "checkpoint_info",
            api.checkpoint_info(checkpoint_view, &checkpoint_info, &error),
            &error) ||
        checkpoint_info.macro_step != step.macro_step ||
        checkpoint_info.exact_tick != step.exact_tick ||
        strcmp(checkpoint_info.model_id, model_id) != 0)
      goto cleanup;
    if (!nexilb_consumer_call_ok(
            &api, "model_reset(before restore)",
            api.model_reset(model, &error), &error))
      goto cleanup;
    if (!nexilb_consumer_call_ok(
            &api, "model_create(restored)",
            api.model_create(context, model_view, &restored, &error), &error))
      goto cleanup;
    if (!nexilb_consumer_call_ok(
            &api, "checkpoint_read",
            api.checkpoint_read(restored, checkpoint_view, 0u, &error),
            &error)) {
      api.model_destroy(&restored, NULL);
      goto cleanup;
    }
    memset(&restored_state, 0, sizeof(restored_state));
    restored_state.struct_size = (uint32_t)sizeof(restored_state);
    restored_state.struct_version = NEXILB_STRUCT_VERSION_1;
    if (!nexilb_consumer_call_ok(
            &api, "model_state(restored)",
            api.model_state(restored, &restored_state, &error), &error) ||
        restored_state.state != NEXILB_MODEL_STATE_READY ||
        restored_state.macro_step != step.macro_step ||
        restored_state.exact_tick != step.exact_tick) {
      api.model_destroy(&restored, NULL);
      goto cleanup;
    }
    memset(&restored_step, 0, sizeof(restored_step));
    restored_step.struct_size = (uint32_t)sizeof(restored_step);
    restored_step.struct_version = NEXILB_STRUCT_VERSION_1;
    if (!nexilb_consumer_call_ok(
            &api, "model_step(restored)",
            api.model_step(restored, &restored_step, &error), &error) ||
        restored_step.macro_step != step.macro_step + 1u ||
        restored_step.exact_tick != step.exact_tick * 2u) {
      api.model_destroy(&restored, NULL);
      goto cleanup;
    }
    if (api.model_destroy(&restored, NULL) != NEXILB_STATUS_OK ||
        restored != NULL) {
      nexilb_consumer_blocked("restored model destroy failed");
      goto cleanup;
    }
    printf("RESTART-COMPLETED: model=%s macro_step=%llu exact_tick=%llu\n",
           model_id, (unsigned long long)restored_step.macro_step,
           (unsigned long long)restored_step.exact_tick);
  }
  printf("RUN-COMPLETED: model=%s macro_step=%llu exact_tick=%llu\n", model_id,
         (unsigned long long)step.macro_step,
         (unsigned long long)step.exact_tick);
  result = 0;

cleanup:
  free(config_json);
  if (model != NULL && api.model_destroy != NULL) {
    nexilb_error_t cleanup_error = NULL;
    nexilb_status_t status = api.model_destroy(&model, &cleanup_error);
    if (!nexilb_consumer_call_ok(&api, "model_destroy", status,
                                 &cleanup_error))
      result = NEXILB_CONSUMER_BLOCKED;
  }
  if (effective_catalog != NULL && api.catalog_destroy != NULL) {
    nexilb_error_t cleanup_error = NULL;
    nexilb_status_t status =
        api.catalog_destroy(&effective_catalog, &cleanup_error);
    if (!nexilb_consumer_call_ok(&api, "effective catalog_destroy", status,
                                 &cleanup_error))
      result = NEXILB_CONSUMER_BLOCKED;
  }
  if (context != NULL && api.context_destroy != NULL) {
    nexilb_error_t cleanup_error = NULL;
    nexilb_status_t status = api.context_destroy(&context, &cleanup_error);
    if (!nexilb_consumer_call_ok(&api, "context_destroy", status,
                                 &cleanup_error))
      result = NEXILB_CONSUMER_BLOCKED;
  }
  if (catalog != NULL && api.catalog_destroy != NULL) {
    nexilb_error_t cleanup_error = NULL;
    nexilb_status_t status = api.catalog_destroy(&catalog, &cleanup_error);
    if (!nexilb_consumer_call_ok(&api, "catalog_destroy", status,
                                 &cleanup_error))
      result = NEXILB_CONSUMER_BLOCKED;
  }
  if (error != NULL && api.error_destroy != NULL) api.error_destroy(&error);
  nexilb_consumer_unload(module);
  return result;
}

static int nexilb_run_config_case(const char *runtime_path,
                                  const char *config_path,
                                  const char *model_id) {
  return nexilb_run_case(runtime_path, config_path, model_id, NULL);
}

static int nexilb_run_checkpoint_case(const char *runtime_path,
                                      const char *config_path,
                                      const char *model_id,
                                      const char *checkpoint_path) {
  return nexilb_run_case(runtime_path, config_path, model_id,
                         checkpoint_path);
}

#endif
