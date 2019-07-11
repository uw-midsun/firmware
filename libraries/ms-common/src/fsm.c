#include "fsm.h"
#include "critical_section.h"

void fsm_init(Fsm *fsm, const char *name, FsmState *default_state, void *context) {
  fsm->name = name;
  fsm->context = context;
  fsm->current_state = default_state;
}

bool fsm_process_event(Fsm *fsm, const Event *e) {
  bool disabled = critical_section_start();
  bool transitioned = false;

  fsm->current_state->table(fsm, e, &transitioned);

  critical_section_end(disabled);
  return transitioned;
}

bool fsm_guard_true(Fsm *fsm, const Event *e, void *context) {
  return true;
}
