#pragma once

#include "generic_can.h"
#include "status.h"

StatusCode permissions_init(GenericCan *can);

void permissions_request(void);

void permissions_cease_request(void);
