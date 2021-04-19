#include "freertos/FreeRTOS.h"
#include "i2c_device.h"
#include "env.h"

#define dig_T1 bmp280->T1
#define dig_T2 bmp280->T2
#define dig_T3 bmp280->T3
#define dig_P1 bmp280->P1
#define dig_P2 bmp280->P2
#define dig_P3 bmp280->P3
#define dig_P4 bmp280->P4
#define dig_P5 bmp280->P5
#define dig_P6 bmp280->P6
#define dig_P7 bmp280->P7
#define dig_P8 bmp280->P8
#define dig_P9 bmp280->P9

struct bmp280 *bmp280;

static double bmp280_compensate_temperature_double(int32_t adc_T);
static double bmp280_compensate_pressure_double(int32_t adc_P);

static I2CDevice_t Bmp280_I2cHandle;

void BMP280_I2C_Init(void)
{
    Bmp280_I2cHandle = i2c_malloc_device(I2C_NUM_1, 21, 22, 400000, 0x76);
}

static uint8_t bmp280_read_register(uint8_t reg_addr)
{
    uint8_t buff;
    i2c_read_bytes(Bmp280_I2cHandle, reg_addr, &buff, 1);
    return buff;
}

static void bmp280_write_register(uint8_t reg_addr, uint8_t reg_data)
{
    i2c_write_bytes(Bmp280_I2cHandle, reg_addr, &reg_data, 1);
}

double bmp280_get_temperature(void)
{
    uint8_t lsb, msb, xlsb;
    int32_t adc_T;
    double temperature;

    xlsb = bmp280_read_register(BMP280_TEMPERATURE_XLSB_REG);
    lsb = bmp280_read_register(BMP280_TEMPERATURE_LSB_REG);
    msb = bmp280_read_register(BMP280_TEMPERATURE_MSB_REG);

    adc_T = (msb << 12) | (lsb << 4) | (xlsb >> 4);
    temperature = bmp280_compensate_temperature_double(adc_T);

    return temperature;
}

double bmp280_get_pressure(void)
{
    uint8_t lsb, msb, xlsb;
    int32_t adc_P;
    double pressure;

    xlsb = bmp280_read_register(BMP280_PRESSURE_XLSB_REG);
    lsb = bmp280_read_register(BMP280_PRESSURE_LSB_REG);
    msb = bmp280_read_register(BMP280_PRESSURE_MSB_REG);

    adc_P = (msb << 12) | (lsb << 4) | (xlsb >> 4);
    pressure = bmp280_compensate_pressure_double(adc_P);

    return pressure;
}

uint8_t bmp280_init(void)
{
    uint8_t bmp280_id;
    uint8_t lsb, msb;
    uint8_t ctrlmeas_reg, config_reg;

    BMP280_I2C_Init();
    bmp280_id = bmp280_read_register(BMP280_CHIPID_REG);
    if (bmp280_id == 0x58)
    {
        bmp280 = malloc(sizeof(struct bmp280));

        bmp280->mode = BMP280_SLEEP_MODE;
        bmp280->t_sb = BMP280_T_SB1;
        bmp280->p_oversampling = BMP280_P_MODE_1;
        bmp280->t_oversampling = BMP280_T_MODE_1;
        bmp280->filter_coefficient = BMP280_FILTER_MODE_1;
    }
    else
    {
        printf("Read BMP280 id error!\r\n");
        return 1;
    }

    /*readthetemperaturecalibrationparameters*/
    lsb = bmp280_read_register(BMP280_DIG_T1_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_T1_MSB_REG);
    dig_T1 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_T2_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_T2_MSB_REG);
    dig_T2 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_T3_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_T3_MSB_REG);
    dig_T3 = msb << 8 | lsb;

    /*readthepressurecalibrationparameters*/
    lsb = bmp280_read_register(BMP280_DIG_P1_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P1_MSB_REG);
    dig_P1 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_P2_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P2_MSB_REG);
    dig_P2 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_P3_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P3_MSB_REG);
    dig_P3 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_P4_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P4_MSB_REG);
    dig_P4 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_P5_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P5_MSB_REG);
    dig_P5 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_P6_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P6_MSB_REG);
    dig_P6 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_P7_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P7_MSB_REG);
    dig_P7 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_P8_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P8_MSB_REG);
    dig_P8 = msb << 8 | lsb;
    lsb = bmp280_read_register(BMP280_DIG_P9_LSB_REG);
    msb = bmp280_read_register(BMP280_DIG_P9_MSB_REG);
    dig_P9 = msb << 8 | lsb;

    bmp280_reset();

    ctrlmeas_reg = bmp280->t_oversampling << 5 | bmp280->p_oversampling << 2 | bmp280->mode;
    config_reg = bmp280->t_sb << 5 | bmp280->filter_coefficient << 2;

    bmp280_write_register(BMP280_CTRLMEAS_REG, ctrlmeas_reg);
    bmp280_write_register(BMP280_CONFIG_REG, config_reg);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    return 0;
}

void bmp280_reset(void)
{
    bmp280_write_register(BMP280_RESET_REG, BMP280_RESET_VALUE);
}

void bmp280_set_standby_time(BMP280_T_SB t_standby)
{
    uint8_t config_reg;

    bmp280->t_sb = t_standby;
    config_reg = bmp280->t_sb << 5 | bmp280->filter_coefficient << 2;

    bmp280_write_register(BMP280_CONFIG_REG, config_reg);
}

void bmp280_set_work_mode(BMP280_WORK_MODE mode)
{
    uint8_t ctrlmeas_reg;

    bmp280->mode = mode;
    ctrlmeas_reg = bmp280->t_oversampling << 5 | bmp280->p_oversampling << 2 | bmp280->mode;

    bmp280_write_register(BMP280_CTRLMEAS_REG, ctrlmeas_reg);
}

void bmp280_set_temperature_oversampling_mode(BMP280_T_OVERSAMPLING t_osl)
{
    uint8_t ctrlmeas_reg;

    bmp280->t_oversampling = t_osl;
    ctrlmeas_reg = bmp280->t_oversampling << 5 | bmp280->p_oversampling << 2 | bmp280->mode;

    bmp280_write_register(BMP280_CTRLMEAS_REG, ctrlmeas_reg);
}

void bmp280_set_pressure_oversampling_mode(BMP280_P_OVERSAMPLING p_osl)
{
    uint8_t ctrlmeas_reg;

    bmp280->t_oversampling = p_osl;
    ctrlmeas_reg = bmp280->t_oversampling << 5 | bmp280->p_oversampling << 2 | bmp280->mode;

    bmp280_write_register(BMP280_CTRLMEAS_REG, ctrlmeas_reg);
}

void bmp280_set_filter_mode(BMP280_FILTER_COEFFICIENT f_coefficient)
{
    uint8_t config_reg;

    bmp280->filter_coefficient = f_coefficient;
    config_reg = bmp280->t_sb << 5 | bmp280->filter_coefficient << 2;

    bmp280_write_register(BMP280_CONFIG_REG, config_reg);
}

static double bmp280_compensate_temperature_double(int32_t adc_T)
{
    double var1, var2, temperature;

    var1 = (((double)adc_T) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
    var2 = ((((double)adc_T) / 131072.0 - ((double)dig_T1) / 8192.0) * (((double)adc_T) / 131072.0 - ((double)dig_T1) / 8192.0)) * ((double)dig_T3);
    bmp280->t_fine = (int32_t)(var1 + var2);
    temperature = (var1 + var2) / 5120.0;

    return temperature;
}

static double bmp280_compensate_pressure_double(int32_t adc_P)
{
    double var1, var2, pressure;

    var1 = ((double)bmp280->t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);

    if (var1 == 0.0)
    {
        return 0;
    }

    pressure = 1048576.0 - (double)adc_P;
    pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * pressure * pressure / 2147483648.0;
    var2 = pressure * ((double)dig_P8) / 32768.0;
    pressure = pressure + (var1 + var2 + ((double)dig_P7)) / 16.0;

    return pressure;
}

void bmp280_get_temperature_and_pressure(double *temperature, double *pressure)
{
    *temperature = bmp280_get_temperature();
    *pressure = bmp280_get_pressure();
}

static I2CDevice_t SHT30_I2cHandle;

void SHT30_Init(void)
{
    SHT30_I2cHandle = i2c_malloc_device(I2C_NUM_1, 21, 22, 400000, 0x44);
    i2c_write_byte(SHT30_I2cHandle, 0x21, 0x26);
}

void SHT30_get(double *temp, double *humidity)
{
    uint8_t buff[6];
    i2c_write_byte(SHT30_I2cHandle, 0xe0, 0x00);
    i2c_read(SHT30_I2cHandle, buff, 6);
    *temp = ((((buff[0] * 256.0) + buff[1]) * 175) / 65535.0) - 45;
    *humidity = ((((buff[3] * 256.0) + buff[4]) * 100) / 65535.0);
}