#include "nexilb/nexilb.h"

_Static_assert(sizeof(void *) == 8u, "NexiLB ABI v1 is 64-bit only");
_Static_assert(sizeof(nexilb_status_t) == 4u, "status width changed");
_Static_assert(sizeof(nexilb_bool_t) == 4u, "boolean width changed");
_Static_assert(sizeof(nexilb_string_view_t) == 16u,
               "string-view layout changed");
_Static_assert(offsetof(nexilb_buffer_t, struct_size) == 0u,
               "struct_size must be first");
_Static_assert(offsetof(nexilb_buffer_t, struct_version) == 4u,
               "struct_version must be second");
_Static_assert(offsetof(nexilb_api_v1, status_name) == 24u,
               "API prefix layout changed");
_Static_assert(NEXILB_API_V1_MANDATORY_PREFIX_SIZE ==
                   offsetof(nexilb_api_v1, library_catalog_create),
               "mandatory prefix definition changed");
_Static_assert(NEXILB_API_V1_MANDATORY_PREFIX_SIZE < sizeof(nexilb_api_v1),
               "mandatory prefix must permit API growth");

int main(void) {
  if (NEXILB_STATUS_OK != 0 || NEXILB_STATUS_INVALID_ARGUMENT >= 0 ||
      NEXILB_STATUS_NOT_IMPLEMENTED >= 0 || NEXILB_MODEL_STATE_EMPTY != 0u ||
      NEXILB_CATALOG_CONTACT_LAW == NEXILB_CATALOG_PARTICLE_SHAPE ||
      NEXILB_API_FLAG_ERROR_OBJECTS_AVAILABLE ==
          NEXILB_API_FLAG_DEVICE_ENUMERATION_AVAILABLE ||
      NEXILB_ERROR_SUBSYSTEM_API == NEXILB_ERROR_SUBSYSTEM_CATALOG ||
      NEXILB_ERROR_SUBSYSTEM_CATALOG == NEXILB_ERROR_SUBSYSTEM_DEVICE) {
    return 1;
  }
  return 0;
}
