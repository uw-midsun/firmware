#include "power_fsm.h"
#include "direction_fsm.h"
#include "headlights_fsm.h"
#include "hazards_fsm.h"

typedef StatusCode (*ConsoleControlsFsmInitFn)(Fsm *fsm, EventArbiterStorage *storage);

typedef enum {
  CONSOLE_CONTROLS_FSM_POWER = 0,
  CONSOLE_CONTROLS_FSM_DIRECTION,
  CONSOLE_CONTROLS_FSM_HEADLIGHTS,
  CONSOLE_CONTROLS_FSM_HAZARDS,
  NUM_CONSOLE_CONTROLS_FSMS,
} ConsoleControlsFsm;

int main() {

}
