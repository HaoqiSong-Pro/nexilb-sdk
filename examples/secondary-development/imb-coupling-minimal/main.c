#include "nexilb_runtime_consumer.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    nexilb_consumer_blocked(
        "usage: imb-coupling-consumer <runtime-library> <relative-config>");
    return NEXILB_CONSUMER_BLOCKED;
  }
  return nexilb_run_config_case(argv[1], argv[2],
                                "model.NPhaseImbDemContactAngle");
}
