#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h" 
#include "esp_timer.h"
#include "esp_rom_sys.h"

#define DHT_PIN             15
#define I2C_MASTER_SDA_IO   8
#define I2C_MASTER_SCL_IO   9
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_FREQ_HZ  100000
#define MPU6050_ADDR        0x68

// ============================================================
// INICIALIZAÇÃO I2C 
// ============================================================
void i2c_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    // Acorda o MPU6050
    uint8_t cmd[2] = {0x6B, 0x00}; 
    i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_ADDR, cmd, sizeof(cmd), pdMS_TO_TICKS(100));
}

// ============================================================
// LER MPU6050
// ============================================================
void ler_mpu6050(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t reg = 0x3B; 
    uint8_t data[6];

    i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDR, &reg, 1, data, 6, pdMS_TO_TICKS(100));

    *ax = (data[0] << 8) | data[1];
    *ay = (data[2] << 8) | data[3];
    *az = (data[4] << 8) | data[5];
}

// ============================================================
// LER DHT22
// ============================================================
int ler_dht22(float *temperatura, float *umidade) {
    uint8_t data[5] = {0};

    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT_PIN, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);

    int timeout = 0;
    while (gpio_get_level(DHT_PIN) == 1) { if (++timeout > 100) return -1; esp_rom_delay_us(1); }
    timeout = 0;
    while (gpio_get_level(DHT_PIN) == 0) { if (++timeout > 100) return -1; esp_rom_delay_us(1); }
    timeout = 0;
    while (gpio_get_level(DHT_PIN) == 1) { if (++timeout > 100) return -1; esp_rom_delay_us(1); }

    for (int i = 0; i < 40; i++) {
        timeout = 0;
        while (gpio_get_level(DHT_PIN) == 0) { if (++timeout > 100) return -1; esp_rom_delay_us(1); }
        int64_t inicio = esp_timer_get_time();
        timeout = 0;
        while (gpio_get_level(DHT_PIN) == 1) { if (++timeout > 100) return -1; esp_rom_delay_us(1); }
        if ((esp_timer_get_time() - inicio) > 40) data[i / 8] |= (1 << (7 - (i % 8)));
    }

    if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4]) return -1;

    *umidade = ((data[0] << 8) | data[1]) / 10.0f;
    *temperatura = (((data[2] & 0x7F) << 8) | data[3]) / 10.0f;
    if (data[2] & 0x80) *temperatura *= -1;

    return 0;
}

// ============================================================
// PROGRAMA PRINCIPAL
// ============================================================
void app_main(void) {
    i2c_init(); // Inicia o I2C para o MPU6050

    float temp = 0.0f, hum = 0.0f;
    int16_t ax, ay, az;

    while (1) {
        printf("\n--- Leitura dos Sensores ---\n");

        if (ler_dht22(&temp, &hum) == 0) {
            printf("DHT22   | Temperatura: %.1f C | Humidade: %.1f %%\n", temp, hum);
        } else {
            printf("DHT22   | Erro de leitura!\n");
        }

        ler_mpu6050(&ax, &ay, &az);
        printf("MPU6050 | AcelX: %d | AcelY: %d | AcelZ: %d\n", ax, ay, az);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}