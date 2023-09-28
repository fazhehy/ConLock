//
// Created by fazhehy on 2023/9/23.
//

#ifndef COMMON_INC_H
#define COMMON_INC_H

#define MATCH_FINGER
//#define ADD_FINGER
//#define DELETE_FINGER

#include <string.h>
#include "stm32f1xx_hal.h"
#include "adc.h"

#include "oled.h"
#include "fpm383c.h"

void Main();
void beep_toggle(uint16_t ms);

#endif //COMMON_INC_H
