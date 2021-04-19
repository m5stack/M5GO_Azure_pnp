#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/spi_common.h"
#include "esp_idf_version.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_err.h"

#include "sk6812.h"
#include "m5go.h"
#include "driver/gpio.h"
#include "i2c_device.h"

pixel_settings_t px;

void m5go_Sk6812_Init() {
    px.pixel_count = 10;
    px.brightness = 20;
    sprintf(px.color_order, "GRBW");
    px.nbits = 24;
    px.timings.t0h = (350);
    px.timings.t0l = (800);
    px.timings.t1h = (600);
    px.timings.t1l = (700);
    px.timings.reset = 80000;
    px.pixels = (uint8_t *)malloc((px.nbits / 8) * px.pixel_count);
    neopixel_init(GPIO_NUM_15, RMT_CHANNEL_0);
    np_clear(&px);
}

void m5go_Sk6812_SetColor(uint16_t pos, uint32_t color) {
    np_set_pixel_color(&px, pos, color << 8);
}

void m5go_Sk6812_SetSideColor(uint8_t side, uint32_t color) {
    if (side == SK6812_SIDE_LEFT) {
        for (uint8_t i = 5; i < 10; i++) {
            np_set_pixel_color(&px, i, color << 8);
        }
    } else {
        for (uint8_t i = 0; i < 5; i++) {
            np_set_pixel_color(&px, i, color << 8);
        }
    }
}

void m5go_Sk6812_SetBrightness(uint8_t brightness) {
    px.brightness = brightness;
}

void m5go_Sk6812_Show() {
    np_show(&px, RMT_CHANNEL_0);
}

void m5go_Sk6812_Clear() {
    np_clear(&px);
}



void m5go_Angle_Init(void){
    //gpio36
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
}


uint8_t m5go_Get_Angle(void){
    uint8_t val = adc1_get_raw(ADC1_CHANNEL_0)/40.95;
    
    printf("adc : %d\r\n",val);
    return val;
}

void m5go_Motion_Init(void){
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_NUM_17;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE ;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}


uint8_t m5go_Get_Motion(void){
    return gpio_get_level(GPIO_NUM_17);
}

void m5go_Mpu6886_Init(void){
    MPU6886_Init();
}

