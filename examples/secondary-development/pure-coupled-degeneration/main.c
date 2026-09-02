#include "nexilb_runtime_consumer.h"

int main(int argc, char **argv) {
  int first;
  if (argc != 4) {
    nexilb_consumer_blocked(
        "usage: degeneration-consumer <runtime-library> <pure-config> "
        "<coupled-config>");
    return NEXILB_CONSUMER_BLOCKED;
  }
  first = nexilb_run_config_case(argv[1], argv[2],
                                 "model.NPhaseContactAngle");
  if (first != 0) return first;
  return nexilb_run_config_case(argv[1], argv[3],
                                "model.NPhaseImbDemContactAngle");
}
