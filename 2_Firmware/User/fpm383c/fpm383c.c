//
// Created by fazhehy on 2023/8/27.
//

#include "fpm383c.h"

union {
    uint8_t frame[24];
    struct {
        uint64_t head;
        uint16_t length;
        uint8_t  check_sum;
        uint8_t verify_code1;
        uint8_t verify_code2;
        uint8_t verify_code3;
        uint8_t verify_code4;
        uint8_t command1;
        uint8_t command2;
        uint8_t data1;
        uint8_t data2;
        uint8_t data3;
        uint8_t data4;
        uint8_t data5;
        uint8_t data6;
        uint8_t data7;
    };
}sendData;

union {
    uint8_t frame[28];
    struct {
        uint64_t head;
        uint16_t length;
        uint8_t  check_sum;
        uint8_t verify_code1;
        uint8_t verify_code2;
        uint8_t verify_code3;
        uint8_t verify_code4;
        uint8_t command1;
        uint8_t command2;
        uint8_t error_code1;
        uint8_t error_code2;
        uint8_t error_code3;
        uint8_t error_code4;
        uint8_t data1;
        uint8_t data2;
        uint8_t data3;
        uint8_t data4;
        uint8_t data5;
        uint8_t data6;
        uint8_t data7;
    };
}receiveData;

#define VERIFY_CODE1    0x00
#define VERIFY_CODE2    0x00
#define VERIFY_CODE3    0x00
#define VERIFY_CODE4    0x00

static uint8_t get_check_sum(const uint8_t *data, uint8_t length)
{
    uint32_t i, sum = 0;
    for (i = 0; i < length; ++i) {
        sum += data[i];
    }

    return (uint8_t)(~((uint8_t)sum&0xff)+1);
}

static uint8_t data;

void fpm383c_init()
{
    sendData.head = 0x8aa86bb62ee21ff1;
    sendData.verify_code1 = VERIFY_CODE1;
    sendData.verify_code2 = VERIFY_CODE2;
    sendData.verify_code3 = VERIFY_CODE3;
    sendData.verify_code4 = VERIFY_CODE4;

    HAL_UART_Receive_IT(&fpm383c_uart, &data, 1);
}

void fpm383c_on()
{
    HAL_GPIO_WritePin(fpm383c_enable_port, fpm383c_enable_pin, GPIO_PIN_RESET);
    HAL_Delay(200);
}

void fpm383c_off()
{
    HAL_GPIO_WritePin(fpm383c_enable_port, fpm383c_enable_pin, GPIO_PIN_SET);
}

enum State{
    error, doing, success
};

static enum State state = error;

static void send_command(uint16_t length)
{
    uint16_t temp;
    state = error;
    length += 7;
    temp = (length&0x00ff)<<8;
    temp |= (length&0xff00)>>8;

    sendData.length = temp;
    sendData.check_sum = get_check_sum(sendData.frame, 10);
    sendData.frame[10 + length] = get_check_sum(sendData.frame + 11, length - 1);

    HAL_UART_Transmit(&fpm383c_uart, sendData.frame, 11 + length, 0xffff);
}

bool receiveFlag = false;

static bool receive_is_over()
{
    if (receiveFlag){
        receiveFlag = false;
        return true;
    }

    return false;
}

void fpm383c_callback()
{
    static uint8_t temp;
    static uint8_t index = 0;

    if (state == error)
        index = 0;

    receiveData.frame[index] = data;
    if (index == 10 + receiveData.length){
        if (data != get_check_sum(receiveData.frame + 11, receiveData.length - 1))
            state = error;
        else
            state = success;
    }
    else{
        switch (index ++) {
            case 0:
                if (data != 0xf1)
                    state = error;
                else
                    state = doing;
                break;
            case 1:
                if (data != 0x1f)
                    state = error;
                break;
            case 2:
                if (data != 0xe2)
                    state = error;
                break;
            case 3:
                if (data != 0x2e)
                    state = error;
                break;
            case 4:
                if (data != 0xb6)
                    state = error;
                break;
            case 5:
                if (data != 0x6b)
                    state = error;
                break;
            case 6:
                if (data != 0xa8)
                    state = error;
                break;
            case 7:
                if (data != 0x8a)
                    state = error;
                break;
            case 8:
                temp = data;
                break;
            case 9:
                receiveData.length = (temp<<8)|data;
                break;
            case 10:
                if (data != get_check_sum(receiveData.frame, 10))
                    state = error;
                break;
            case 11:
                if (data != VERIFY_CODE1)
                    state = error;
                break;
            case 12:
                if (data != VERIFY_CODE2)
                    state = error;
                break;
            case 13:
                if (data != VERIFY_CODE3)
                    state = error;
                break;
            case 14:
                if (data != VERIFY_CODE4)
                    state = error;
                break;
            default:
                break;
        }
    }

    if (state == success){
//        HAL_UART_Transmit(&huart1, (uint8_t *)&state, 1, 10);
        receiveFlag = true;
        state = error;
        index = 0;
    }

    HAL_UART_Receive_IT(&fpm383c_uart, &data, 1);
}

uint8_t fpm383c_match_finger(uint16_t timeout)
{
    int32_t timeout_ = timeout;

    MATCH_FINGER:
    sendData.command1 = 0x01;
    sendData.command2 = 0x21;
    send_command(0);
    while (!receive_is_over() && (--timeout_))
        HAL_Delay(1);
    if (timeout_ <= 0)
    {
//        oled_show_char(0, 0, 'e');
        return 0xff;
    }
    timeout_ = timeout;

    CHECK_MATCH:
    sendData.command1 = 0x01;
    sendData.command2 = 0x22;
    send_command(0);
    while (!receive_is_over() && (--timeout_))
        HAL_Delay(1);
    if (timeout_ <= 0)
    {
//        oled_show_char(0, 0, 'e');
        return 0xff;
    }

//    oled_show_num(0, 0, receiveData.error_code4, 2);
    if (receiveData.error_code4 == 0x04 || receiveData.error_code4 == 0x05){
        HAL_Delay(200);
        goto CHECK_MATCH;
    }
    else if (receiveData.error_code4 == 0x0e || receiveData.error_code4 == 0x10 || receiveData.error_code4 == 0x09){
        //again
        goto MATCH_FINGER;
    }
    else if (receiveData.error_code4 == 0x00){
        if (receiveData.data2 == 0x01)
            return 0;
    }
    return 1;
}

void fpm383c_controlLED(uint8_t color)
{
    sendData.command1 = 0x02;
    sendData.command2 = 0x0f;
    sendData.data1 = 0x04;
    sendData.data2 = color;
    sendData.data3 = 0x01;
    sendData.data4 = 0x00;
    sendData.data5 = 0x14;
    send_command(5);
}

uint8_t fpm383c_finger_is_exist(uint16_t timeout)
{
    int32_t timeout_ = timeout;
    sendData.command1 = 0x01;
    sendData.command2 = 0x35;
    send_command(0);
    while (!receive_is_over() && (--timeout_))
        HAL_Delay(1);
    if (timeout_ <= 0)
        return 0xff;
    if (receiveData.data1 == 0x01)
        return 0;

    return 1;
}

void fpm383c_sleep()
{
    uint16_t timeout = 1000;
    int32_t timeout_ = timeout;
    do{
        receiveData.data1 = 0xff;
        sendData.command1 = 0x01;
        sendData.command2 = 0x35;
        send_command(0);
        fpm383c_controlLED(2);
        timeout_ = timeout;
        while (!receive_is_over() && (--timeout_))
            HAL_Delay(1);
        if (timeout_ <= 0)
            continue;
        if (receiveData.error_code4 != 0x00)
            continue;
    } while (receiveData.data1 != 0x00);

    sendData.command1 = 0x02;
    sendData.command2 = 0x0c;
    sendData.data1 = 0x00;
    send_command(1);
}

uint8_t fpm383c_add_finger(uint16_t timeout, void (*show_progress)(uint8_t progress), void (*show_error)(char * str))
{
    uint8_t reg_idx = 0x01;
    uint8_t id_h, id_l;
    int32_t timeout_ = timeout;

    show_progress(0);

    //发送指纹注册命令
    ADD_FINGER:
    sendData.command1 = 0x01;
    sendData.command2 = 0x11;
    sendData.data1 = reg_idx;
    send_command(1);
    while (!receive_is_over() && (--timeout_))
        HAL_Delay(1);
    if (timeout_ <= 0)
        return 0xff;
    timeout_ = timeout;
    if (receiveData.error_code4 != 0x00)
        return 0x01;
//    else if (receiveData.error_code4 == 0x00){
//        show_error("ok                          ");
//    }

    //查询注册结果
    CHECK_RESULT:
    sendData.command1 = 0x01;
    sendData.command2 = 0x12;
    send_command(0);
    while (!receive_is_over() && (--timeout_))
        HAL_Delay(1);
    if (timeout_ <= 0 )
        return 0xff;
    timeout_ = timeout;
    if (receiveData.error_code4 == 0x04){
        HAL_Delay(200);
        goto CHECK_RESULT;
    }
    else if (receiveData.error_code4 == 0x08){
        show_error("no finger                 ");
        goto ADD_FINGER;
    }
    else if (receiveData.error_code4 == 0x09){
        show_error("please again              ");
        goto ADD_FINGER;
    }
    else if (receiveData.error_code4 == 0x0f){
        show_error("same! please again        ");
        goto ADD_FINGER;
    }
    else if (receiveData.error_code4 == 0x0e)
    {
        show_error("poor quality! please again");
        goto ADD_FINGER;
    }
    else if (receiveData.error_code4 == 0x10)
    {
        show_error("small area! please again  ");
        goto ADD_FINGER;
    }
    else if (receiveData.error_code4 == 0x11)
    {
        show_error("move much! please again   ");
        goto ADD_FINGER;
    }
    else if (receiveData.error_code4 == 0x0e)
    {
        show_error("move little! please again   ");
        goto ADD_FINGER;
    }
//    else if (receiveData.error_code4 == 0x00){
//        show_error("ok                          ");
//    }
    else if (receiveData.error_code4 != 0x00){
        return 1;
    }
    id_h = receiveData.data1;
    id_l = receiveData.data2;
    show_progress(receiveData.data3);
    if (receiveData.data3 >= 100){
        goto SAVE_FINGER;
    }

    //查询手指在位状态
    CHECK_FINGER:
    sendData.command1 = 0x01;
    sendData.command2 = 0x35;
    send_command(0);
    while (!receive_is_over() && (--timeout_))
        HAL_Delay(1);
    if (timeout_ <= 0 )
        return false;
    timeout_ = timeout;
    if (receiveData.error_code4 != 0x00 || receiveData.data1 == 0x01){
        show_error("please again                   ");
        HAL_Delay(200);
//        OLED_ShowString(1, 1, "Again2");
        goto CHECK_FINGER;
    }
//    else if (receiveData.error_code4 == 0x00){
//        show_error("ok                          ");
//    }
    else{
        reg_idx ++;
        goto ADD_FINGER;
    }

    SAVE_FINGER:
    sendData.command1 = 0x01;
    sendData.command2 = 0x13;
    sendData.data1 = id_h;
    sendData.data2 = id_l;
    send_command(2);

    //查询保存的指纹
    CHECK_SAVE_FINGER:
    sendData.command1 = 0x01;
    sendData.command2 = 0x14;
    send_command(0);
    while (!receive_is_over() && (--timeout_))
        HAL_Delay(1);
    if (timeout_ <= 0 )
        return 0xff;
    timeout_ = timeout;
    if (receiveData.error_code4 == 0x00)
        return 0;
    else if (receiveData.error_code4 == 0x04){
        HAL_Delay(200);
        goto CHECK_SAVE_FINGER;
    }
    else if (receiveData.error_code4 == 0x0f)
    {
        show_error("template repeat        ");
        return 2;
    }
//    else if (receiveData.error_code4 == 0x00){
//        show_error("ok                          ");
//    }
    else{
        return 1;
    }
    return 1;
}

uint8_t fpm383c_get_finger_num(uint16_t timeout)
{
    int32_t timeout_ = timeout;

    sendData.command1 = 0x02;
    sendData.command2 = 0x03;
    send_command(0);
    while (!receive_is_over() && (--timeout_))
        HAL_Delay(1);
    if (timeout_ <= 0 )
        return 0xff;

    if (receiveData.error_code4 == 0x00)
        return (receiveData.data2<<4);

    return 1;
}

void fpm383c_empty()
{
    sendData.command1 = 0x01;
    sendData.command2 = 0x36;
    sendData.data1 = 0x01;
    sendData.data2 = 0x00;
    sendData.data3 = 0x01;
    send_command(3);
}
