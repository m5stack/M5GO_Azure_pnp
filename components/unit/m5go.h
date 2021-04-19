#pragma once

#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/spi_common.h"
#include "esp_idf_version.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_err.h"
#define SK6812_SIDE_LEFT 0
#define SK6812_SIDE_RIGHT 1
#include "mpu6886.h"
#include "env.h"
void m5go_Sk6812_Init();
void m5go_Sk6812_SetColor(uint16_t pos, uint32_t color);
void m5go_Sk6812_SetSideColor(uint8_t side, uint32_t color);
void m5go_Sk6812_SetBrightness(uint8_t brightness);
void m5go_Sk6812_Show();
void m5go_Sk6812_Clear();

void m5go_Angle_Init(void);
uint8_t m5go_Get_Angle(void);

void m5go_Motion_Init(void);
uint8_t m5go_Get_Motion(void);
