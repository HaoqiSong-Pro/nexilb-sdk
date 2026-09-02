#include "nexilb/nexilb.h"

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HMODULE nexilb_native_module_t;
static nexilb_native_module_t open_module(const char *path) {
  return LoadLibraryExA(path, NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                                    LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
}
static void *find_symbol(nexilb_native_module_t module, const char *name) {
  return (void *)(uintptr_t)GetProcAddress(module, name);
}
static void close_module(nexilb_native_module_t module) { (void)FreeLibrary(module); }
#else
#include <dlfcn.h>
typedef void *nexilb_native_module_t;
static nexilb_native_module_t open_module(const char *path) {
  return dlopen(path, RTLD_NOW | RTLD_LOCAL);
}
static void *find_symbol(nexilb_native_module_t module, const char *name) {
  return dlsym(module, name);
}
static void close_module(nexilb_native_module_t module) { (void)dlclose(module); }
#endif

typedef nexilb_status_t(NEXILB_CALL *get_abi_fn)(nexilb_abi_version_t *);
typedef nexilb_status_t(NEXILB_CALL *get_api_fn)(uint32_t, uint64_t,
                                                 nexilb_api_v1 *);

/* Consumer-side loader contract. It is compiled but not run without an
 * approved runtime asset. The module remains loaded until every handle created
 * through its table has been destroyed. */
int nexilb_consumer_probe_runtime(const char *path) {
  nexilb_native_module_t module;
  get_abi_fn get_abi;
  get_api_fn get_api;
  nexilb_abi_version_t version = {0};
  nexilb_api_v1 api = {0};
  if (path == NULL || path[0] == '\0') {
    return 2;
  }
  module = open_module(path);
  if (module == NULL) {
    return 3;
  }
  get_abi = (get_abi_fn)find_symbol(module, "nexilb_get_abi_version");
  get_api = (get_api_fn)find_symbol(module, "nexilb_get_api");
  if (get_abi == NULL || get_api == NULL ||
      get_abi(&version) != NEXILB_STATUS_OK ||
      version.abi_major != NEXILB_ABI_MAJOR ||
      version.api_min > NEXILB_API_V1 || version.api_max < NEXILB_API_V1 ||
      get_api(NEXILB_API_V1, sizeof(api), &api) != NEXILB_STATUS_OK ||
      api.struct_size < NEXILB_API_V1_MANDATORY_PREFIX_SIZE ||
      api.status_name == NULL || api.library_info == NULL) {
    close_module(module);
    return 4;
  }
  close_module(module);
  return 0;
}

