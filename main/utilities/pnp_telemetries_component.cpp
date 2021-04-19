// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Standard C header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "m5go.h"
// PnP routines
#include "pnp_protocol.h"
#include "pnp_telemetries_component.h"

// Core IoT SDK utilities
#include "azure_c_shared_utility/xlogging.h"

#define LGFX_M5STACK
#include <LovyanGFX.hpp>

extern LGFX lcd;


void PnP_TelemetriesComponent_SendTelemetry(const char *MessageBuffer, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = NULL;
    IOTHUB_CLIENT_RESULT iothubResult;

    if (MessageBuffer == NULL)
    {
        LogError("snprintf of current temperature telemetry failed");
    }
    else if ((messageHandle = PnP_CreateTelemetryMessageHandle(NULL, MessageBuffer)) == NULL)
    {
        LogError("Unable to create telemetry message");
    }
    else if ((iothubResult = IoTHubDeviceClient_LL_SendEventAsync(deviceClientLL, messageHandle, NULL, NULL)) != IOTHUB_CLIENT_OK)
    {
        LogError("Unable to send telemetry message, error=%d", iothubResult);
    }

    IoTHubMessage_Destroy(messageHandle);
}

uint8_t PnP_SendTelemetry(IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    static int sendMun;
    char StringBuffer[32];
    int ret = -1;

    time_t now;
    time(&now);
    char strftime_buf[64];
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    timeinfo.tm_hour = (timeinfo.tm_hour + 8) % 24;
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);

    lcd.clear(0x00);
    lcd.setCursor(0, 0, lgfx::fontdata[4]);
    lcd.printf("/** %s **/\r\n", strftime_buf);
    lcd.setCursor(0, 50, lgfx::fontdata[2]);

    double temp, humidity;
    SHT30_get(&temp, &humidity);
    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"Humidity\":%.02f}", humidity);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);

    double temperature, pressure;
    bmp280_get_temperature_and_pressure(&temperature, &pressure);
    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"Temperature\":%.02f}", temperature);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);
    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"Pressure\":%.02f}", pressure);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);
    lcd.printf("Temperature : %.02f Celsius\r\nHumidity : %.02f %% \r\nPressure : %.02f Pa \r\n", temperature, humidity, pressure);

    float ax, ay, az;
    MPU6886_GetAccelData(&ax, &ay, &az);
    lcd.printf("Accel : (%.02f ,%.02f ,%.02f)\r\n", ax, ay, az);
    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"AccelX\":%.02f}", ax);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);

    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"AccelY\":%.02f}", ay);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);

    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"AccelZ\":%.02f}", az);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);

    float gx, gy, gz;
    MPU6886_GetGyroData(&gx, &gy, &gz);
    lcd.printf("Gyro : (%.02f ,%.02f ,%.02f)    \r\n", gx, gy, gz);
    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"GyroX\":%.02f}", gx);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);
    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"GyroY\":%.02f}", gy);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);
    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"GyroZ\":%.02f}", gz);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);

    uint8_t angle = m5go_Get_Angle();
    ret = snprintf(StringBuffer, sizeof(StringBuffer), "{\"angle\":%d}", angle);
    if (ret < 0)
    {
        return 1;
    }
    PnP_TelemetriesComponent_SendTelemetry(StringBuffer, deviceClientLL);
    lcd.printf("Angle : %d / 100\r\n", angle);

    if (m5go_Get_Motion())
    {
        PnP_TelemetriesComponent_SendTelemetry("{\"pir\":\"true}\"", deviceClientLL);
        lcd.printf("PIR : true\r\n");
    }
    else
    {
        PnP_TelemetriesComponent_SendTelemetry("{\"pir\":\"false\"}", deviceClientLL);
        lcd.printf("PIR : False\r\n");
    }

    lcd.printf("Telemetry number of sends : %d\r\n", ++sendMun);

    return 0;
}
