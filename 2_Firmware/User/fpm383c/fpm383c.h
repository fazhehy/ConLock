//
// Created by fazhehy on 2023/8/27.
//

#ifndef FPM383C_H
#define FPM383C_H

#include <stdbool.h>
#include "usart.h"

#define fpm383c_uart    huart2
#define fpm383c_enable_port GPIOA
#define fpm383c_enable_pin GPIO_PIN_4

void fpm383c_init();
void fpm383c_on();
void fpm383c_off();
void fpm383c_callback();
uint8_t fpm383c_finger_is_exist(uint16_t timeout);
uint8_t fpm383c_match_finger(uint16_t timeout);
void fpm383c_controlLED(uint8_t color);
void fpm383c_sleep();
uint8_t fpm383c_add_finger(uint16_t timeout, void (*show_progress)(uint8_t progress), void (*show_error)(char * str));
uint8_t fpm383c_get_finger_num(uint16_t timeout);
void fpm383c_empty();

#endif //FPM383C_H
