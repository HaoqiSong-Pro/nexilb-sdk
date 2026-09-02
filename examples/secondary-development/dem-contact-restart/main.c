#include "nexilb_runtime_consumer.h"

int main(int argc, char **argv) {
  if (argc != 3 && argc != 4) {
    nexilb_consumer_blocked(
        "usage: dem-contact-consumer <runtime-library> <relative-config> "
        "[relative-checkpoint-directory]");
    return NEXILB_CONSUMER_BLOCKED;
  }
  if (argc == 3)
    return nexilb_run_config_case(
        argv[1], argv[2], "model.NPhaseImbDemContactAngle");
  return nexilb_run_checkpoint_case(
      argv[1], argv[2], "model.NPhaseImbDemContactAngle", argv[3]);
}
