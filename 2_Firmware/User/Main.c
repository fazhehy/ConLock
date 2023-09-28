//
// Created by fazhehy on 2023/9/23.
//

#include "common_inc.h"

void beep_on()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
}

void beep_off()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
}

void beep_toggle(uint16_t ms)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(ms);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(ms);
}

void motor_on(uint8_t dir)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    if (dir){
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
    }
    else{
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    }

}

void motor_off()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}

#ifdef MATCH_FINGER
uint32_t get_battery_value()
{
    uint32_t battery_value = 0;
    HAL_ADC_Start(&hadc1);     //启动ADC转换
    HAL_ADC_PollForConversion(&hadc1, 50);
    if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC)) {
        battery_value = HAL_ADC_GetValue(&hadc1);   //获取AD值
        battery_value = (uint32_t) (((float) (battery_value - 3028) / 410) * 100);
    }
    return battery_value;
}

void battery_display(uint8_t percent)
{
    if (percent >= 99)
        percent = 99;
    if (percent <= 10)
    {
        for (int i = 0; i < 10; ++i) {
            beep_toggle(50);
        }
    }
    oled_show_num(84+25, 1, percent, 2);
    oled_show_char(84+26+2*6, 1, '%');
    oled_draw_round_rect(88, 0, 21, 10, 3);
    uint8_t w = 21*percent/100;
    if (w >= 6)
        oled_draw_fill_round_rect(88, 0, w, 10, 3);
}

void scene_display()
{
    battery_display(get_battery_value());
    oled_show_string(0, 0, "121 Dorm");
    oled_show_bmp(64-38, 32+3, name, 76, 24);
    oled_show_bmp(64-16, 6, fingerprint, 32, 32);
    oled_show_string(0, 56, "Designed");
    oled_show_string(8*6+3, 56, "by");
    oled_show_string(10*6+3*2, 56, "fazhehy");
    oled_show_string(17*6+3*3, 56, "Li");
}

void Main()
{
    HAL_ADCEx_Calibration_Start(&hadc1);
    oled_init();
    fpm383c_on(); 
    fpm383c_init();
    scene_display();
    oled_update_screen();



//    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);//强制使能WKUP(PA0)引脚
//    SET_BIT(PWR->CR, PWR_CR_CWUF_Msk);//写1清除该位 唤醒位
//    SET_BIT(PWR->CR, PWR_CR_CSBF_Msk);//写1清除该位 待机位

//    motor_on(0);

//    while (1)
//    {
////        motor_on(1);
////        HAL_Delay(500);
////        motor_on(0);
////        HAL_Delay(500);
//    }
    for (;;){
//        fpm383c_controlLED(2);
//        HAL_Delay(500)
        if (fpm383c_finger_is_exist(100) == 0){
            HAL_Delay(200);
            if (fpm383c_match_finger(100) == 0){
                fpm383c_controlLED(6);
                oled_clear_buffer();
                oled_show_bmp(64-18, 0, thumb, 39, 64);
                oled_update_screen();

                motor_on(0);
                beep_toggle(500);
                HAL_Delay(3800);
                motor_off();
                HAL_Delay(500);
                motor_on(1);
                HAL_Delay(2600);
                beep_toggle(500);
                motor_off();
                oled_clear_buffer();
                scene_display();
                oled_update_screen();
            }
            else{

                fpm383c_controlLED(2);
                oled_clear_buffer();
                oled_show_bmp(64-23, 0, middle_finger, 46, 64);
                oled_update_screen();
                beep_toggle(100);
                HAL_Delay(100);
                oled_clear_buffer();
                scene_display();
                oled_update_screen();
            }
        }
    }

//    fpm383c_sleep();
//    SET_BIT(PWR->CR, PWR_CR_CWUF_Msk);//写1清除该位 唤醒位 如果不清楚此位 系统将保持唤醒状态
//    HAL_Delay(50);
//    HAL_PWR_EnterSTANDBYMode();//进入待机模式

}
#endif

#ifdef ADD_FINGER
void display_progress(uint8_t progress)
{
    if (progress >= 100){
        oled_clear_buffer();
        oled_show_bmp(32, 0, ok_picture, 64, 64);
        oled_update_screen();
        oled_clear_buffer();
        fpm383c_controlLED(6);
        beep_toggle(200);
        HAL_Delay(800);
    }
    else{
        oled_show_num(64-12, 32-8-9+5, progress, 3);
        oled_show_char(64+3*6-11, 32-8-9+5, '%');
        oled_draw_round_rect(64-40, 32-7+5, 80, 15, 5);
        if (80*progress/100 > 10)
            oled_draw_fill_round_rect(64-40, 32-7+5, 80*progress/100, 15, 5);
        oled_update_screen();
    }
}

void display_error(char * str)
{
    char temp[22];
    if (strlen(str)<=21)
        oled_show_string(0, 0, str);
    else{
        strncpy(temp, str, 21);
        oled_show_string(0, 0, temp);
        oled_show_string(0, 9, str+21);
    }
    oled_update_screen();
}

void display_finger_num()
{
    uint8_t finger_num = fpm383c_get_finger_num(1000);
    if (finger_num == 0xff)
        display_error("get timeout!!!");
    else if (finger_num == 1)
        display_error("get system error!!!");
    oled_show_num(12*6, 56, finger_num >> 4, 2);
    oled_show_string(0, 56, "finger num:");
    oled_update_screen();
}

void Main()
{
    oled_init();
    fpm383c_on();
    fpm383c_init();
//    servo_init();
    uint8_t state =  0;

    for (;;){
        HAL_Delay(200);
        display_finger_num();
        state = fpm383c_add_finger(1000, display_progress, display_error);
        display_progress(0);
        if (state == 0xff)
            display_error("add timeout!!!");
        else if (state == 1)
            display_error("add system error!!!");
        display_finger_num();
        oled_clear_buffer();
    }
}
#endif

#ifdef DELETE_FINGER
void display_error(char * str)
{
    char temp[22];
    if (strlen(str)<=21)
        oled_show_string(0, 0, str);
    else{
        strncpy(temp, str, 21);
        oled_show_string(0, 0, temp);
        oled_show_string(0, 9, str+21);
    }
    oled_update_screen();
}

void display_finger_num()
{
    uint8_t finger_num = fpm383c_get_finger_num(200);
    if (finger_num == 0xff)
        display_error("get timeout!!!");
    else if (finger_num == 1)
        display_error("get system error!!!");
    oled_show_num(12*6, 56, finger_num >> 4, 2);
    oled_show_string(0, 56, "finger num:");
    oled_update_screen();
}

void Main()
{
    oled_init();
    fpm383c_on();
    fpm383c_init();
    for (;;){
//        motor_off();
//        servo_loosen();
//        HAL_Delay(2000);
//        servo_shrink();
        fpm383c_empty();
        display_finger_num();
//        HAL_Delay(2000);
    }
}
#endif

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
    if (huart->Instance == fpm383c_uart.Instance){
        fpm383c_callback();
    }
}
