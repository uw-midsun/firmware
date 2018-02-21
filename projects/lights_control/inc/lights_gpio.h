#pragma once

typedef enum { LIGHTS_BOARD_FRONT, LIGHTS_BOARD_REAR } BoardType;

StatusCode lights_gpio_init(BoardType boardtype);
StatusCode lights_gpio_set(Event e);
StatusCode get_board_type(BoardType* type);
