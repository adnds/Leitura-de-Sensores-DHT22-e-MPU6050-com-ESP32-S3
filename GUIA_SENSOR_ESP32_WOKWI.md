# 📚 Guia Completo - Sensores com ESP32-S3 e Wokwi

**Documentação técnica aprofundada sobre DHT22, MPU6050, protocolos e simulação**

---

## 📋 Índice

1. [Introdução](#-introdução)
2. [Conceitos Fundamentais](#-conceitos-fundamentais)
3. [Sensor DHT22 - Protocolo 1-Wire](#-sensor-dht22---protocolo-1-wire)
4. [Sensor MPU6050 - Comunicação I2C](#-sensor-mpu6050---comunicação-i2c)
5. [Integração no Código](#-integração-no-código)
6. [Calibração e Validação](#-calibração-e-validação)
7. [Simulação Wokwi](#-simulação-wokwi)
8. [Otimizações e Performance](#-otimizações-e-performance)
9. [Extensões Avançadas](#-extensões-avançadas)
10. [Referência de Pinagem](#-referência-de-pinagem)

---

## 🎯 Introdução

Este guia aprofunda o conhecimento sobre:

- **Como funcionam** sensores DHT22 e MPU6050
- **Protocolos de comunicação** 1-Wire e I2C
- **Integração com FreeRTOS** para multitarefa
- **Simulação realista** em Wokwi
- **Calibração e validação** de dados
- **Otimizações** de código e consumo

### Público-Alvo

✅ Desenvolvedores com experiência em C/ESP-IDF  
✅ Hobbyistas de eletrônica embarcada  
✅ Estudantes de engenharia/computação  
✅ Profissionais de IoT  

### Pré-requisitos

- Conhecimento básico de C
- Familiaridade com VS Code
- ESP-IDF v4.1+ instalado
- Wokwi configurado

---

## 📖 Conceitos Fundamentais

### Microcontrolador ESP32-S3

```
┌──────────────────────────────────────┐
│      ESP32-S3 DevKit-C 1             │
├──────────────────────────────────────┤
│ • Dual-core CPU (240 MHz)            │
│ • WiFi 802.11 b/g/n                  │
│ • Bluetooth 5.0 LE                   │
│ • 34 GPIO pinos                      │
│ • ADC, SPI, I2C, UART integrados     │
│ • 8 MB Flash + 2 MB PSRAM            │
│ • FreeRTOS incluído                  │
└──────────────────────────────────────┘
```

### Comunicação Serial

**UART (Serial):**
- Taxa padrão: **115200 baud**
- Uso: Log de dados, debug
- Pinos: TX (GPIO 43), RX (GPIO 44)

**I2C (Inter-Integrated Circuit):**
- Protocolo: Master-Slave
- Velocidade: 100 kHz (padrão), até 400 kHz
- Pinos: SDA (dados), SCL (clock)
- Múltiplos dispositivos em um barramento

**GPIO (General Purpose I/O):**
- Entrada/saída digital
- 3.3V lógico (NÃO 5V!)
- Suporta PWM, interrupções

### Temporização com FreeRTOS

```c
// Atraso em milissegundos
vTaskDelay(pdMS_TO_TICKS(1000));  // 1 segundo

// Atraso em microssegundos (precisão)
esp_rom_delay_us(50);  // 50 microssegundos

// Timer de alta resolução
int64_t timestamp = esp_timer_get_time();  // microsegundos desde boot
```

---

## 🌡️ Sensor DHT22 - Protocolo 1-Wire

### Especificações Técnicas

```
┌─────────────────────────────────────────┐
│          DHT22 (AM2302)                 │
├─────────────────────────────────────────┤
│ Temperatura: -40 a 125 °C               │
│ Umidade: 0 a 100% RH                    │
│ Precisão T: ±0.5 °C                     │
│ Precisão H: ±2% RH                      │
│ Taxa de amostragem: max 1/2s            │
│ Tensão: 3.3-5.5V (esp32: 3.3V)         │
│ Consumo: ~0.5-2.5 mA                    │
│ Protocolo: 1-Wire proprietário          │
└─────────────────────────────────────────┘
```

### Pinagem DHT22

```
   VCC ┌─┐ GND
       │ │
     1 │ │ 3
       │ │
   SDA └─┘ NC
       
VCC:  Alimentação (3.3V)
SDA:  Dados (single wire)
GND:  Terra
NC:   Não conectado
```

### Protocolo 1-Wire (Timing Crítico)

```
Master → Sensor: Pull-down 20ms → Release → Aguarda resposta
Sensor → Master: ACK (80us LOW, 80us HIGH) → Envia 40 bits

Cada bit de dados:
├─ LOW 50us → HIGH 26us = BIT "0"
└─ LOW 50us → HIGH 70us = BIT "1"

Ordem: MSB first (bit 39 para bit 0)
       Bytes: [Umidade_INT][Umidade_DEC][Temp_INT][Temp_DEC][Checksum]
```

### Implementação em C

```c
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#define DHT_PIN 15

int ler_dht22(float *temperatura, float *umidade) {
    uint8_t data[5] = {0};
    
    // PASSO 1: Enviar sinal de início (pull-down 20ms)
    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT_PIN, 1);
    
    // PASSO 2: Sensor responde (80us LOW, 80us HIGH)
    esp_rom_delay_us(40);
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);
    
    // Aguardar resposta do sensor
    int timeout = 0;
    while (gpio_get_level(DHT_PIN) == 1) {
        if (++timeout > 100) return -1;  // Timeout
        esp_rom_delay_us(1);
    }
    
    // PASSO 3: Ler 40 bits
    for (int i = 0; i < 40; i++) {
        // Aguardar bit LOW terminar
        timeout = 0;
        while (gpio_get_level(DHT_PIN) == 0) {
            if (++timeout > 100) return -1;
            esp_rom_delay_us(1);
        }
        
        // Medir duração do bit HIGH
        int64_t inicio = esp_timer_get_time();
        timeout = 0;
        while (gpio_get_level(DHT_PIN) == 1) {
            if (++timeout > 100) return -1;
            esp_rom_delay_us(1);
        }
        
        // Se HIGH > 40us → BIT "1", senão BIT "0"
        if ((esp_timer_get_time() - inicio) > 40) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    
    // PASSO 4: Validar checksum
    if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4]) {
        return -1;  // Checksum inválido
    }
    
    // PASSO 5: Decodificar dados
    *umidade = ((data[0] << 8) | data[1]) / 10.0f;
    *temperatura = (((data[2] & 0x7F) << 8) | data[3]) / 10.0f;
    
    // Bit 7 do byte 2 = sinal de temperatura negativa
    if (data[2] & 0x80) {
        *temperatura *= -1;
    }
    
    return 0;  // Sucesso
}
```

### Exemplo de Leitura

```c
void task_ler_dht22(void *pvParameters) {
    float temp = 0.0f, hum = 0.0f;
    
    while (1) {
        int ret = ler_dht22(&temp, &hum);
        
        if (ret == 0) {
            printf("✓ DHT22: %.1f°C, %.1f%% RH\n", temp, hum);
        } else {
            printf("✗ DHT22 Erro: %d\n", ret);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));  // Respeitar limite 1/2s
    }
}
```

### Troubleshooting DHT22

| Sintoma | Causa | Solução |
|---------|-------|---------|
| Sempre retorna -1 | Timeout na leitura | Verificar GPIO, alimentação |
| Valores ~0°C, 0% RH | Sensor desligado | Verificar VCC = 3.3V |
| Valores erráticos | Ruído eletromagnético | Adicionar capacitor 100nF VCC-GND |
| Leitura lenta | Rate limiting | Aguardar 2s entre leituras |
| Checksum inválido | Dados corrompidos | Repetir leitura, verificar cabos |

---

## 🎯 Sensor MPU6050 - Comunicação I2C

### Especificações Técnicas

```
┌──────────────────────────────────────────┐
│    MPU-6050 (6-Axis IMU)                 │
├──────────────────────────────────────────┤
│ Aceleração: ±16g (±156 m/s²)             │
│ Giroscópio: ±2000°/s                     │
│ Temperatura interna: -40 a 85°C          │
│ Resolução: 16-bit ADC                    │
│ Endereço I2C: 0x68 (padrão)              │
│ Tensão: 2.375-3.465V (esp32: 3.3V)      │
│ Consumo: ~3.9 mA                         │
│ Interface: I2C (até 400 kHz)             │
└──────────────────────────────────────────┘
```

### Pinagem MPU6050

```
   VCC ┌────┐ GND
       │ 1  │
   SDA │ 2  │ 3 SCL
       │ 4  │
   INT └────┘ AD0
   
VCC:  Alimentação (3.3V)
GND:  Terra
SDA:  Dados I2C (GPIO 8 no ESP32)
SCL:  Clock I2C (GPIO 9 no ESP32)
INT:  Interrupção (opcional)
AD0:  Seleção endereço (GND=0x68, VCC=0x69)
```

### Protocolo I2C

**Master Write:**
```
START → [ADDR + W] → ACK → [REGISTER] → ACK → [DATA] → ACK → STOP
```

**Master Read:**
```
START → [ADDR + W] → ACK → [REGISTER] → ACK → 
RESTART → [ADDR + R] → ACK → [DATA] → NACK → STOP
```

### Registros Importantes

```
0x6B: Power Management 1
      Bit 7=1: Reset
      Bit 6=1: Sleep
      Bit 0: Clock selection (0=internal)
      
0x3B-0x3D: Aceleração X, Y, Z (2 bytes cada)
           Formato: big-endian
           Exemplo: 0x3B=MSB_X, 0x3C=LSB_X
           
0x43-0x45: Giroscópio X, Y, Z (2 bytes cada)
           
0x41: Aceleração Full Scale
      Bits 4-3: 0=±2g, 1=±4g, 2=±8g, 3=±16g
```

### Implementação em C

```c
#include "driver/i2c.h"

#define I2C_MASTER_SDA_IO   8
#define I2C_MASTER_SCL_IO   9
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_FREQ_HZ  100000
#define MPU6050_ADDR        0x68

// Inicializar I2C e acordar MPU6050
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
    
    // Acordar MPU6050 (limpar bit Sleep)
    uint8_t cmd[2] = {0x6B, 0x00};  // Reg 0x6B = 0x00
    i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_ADDR, 
                               cmd, sizeof(cmd), pdMS_TO_TICKS(100));
    
    printf("[I2C] MPU6050 inicializado\n");
}

// Ler aceleração 3-eixos
void ler_mpu6050(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t reg = 0x3B;  // Endereço de Aceleração X (MSB)
    uint8_t data[6];     // 6 bytes = 3 valores de 16-bit
    
    // I2C Read: registro 0x3B + 6 bytes
    i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDR,
                                 &reg, 1, data, 6,
                                 pdMS_TO_TICKS(100));
    
    // Converter big-endian para int16_t
    *ax = (int16_t)((data[0] << 8) | data[1]);
    *ay = (int16_t)((data[2] << 8) | data[3]);
    *az = (int16_t)((data[4] << 8) | data[5]);
}

// Ler temperatura (bônus)
int16_t ler_mpu6050_temp(void) {
    uint8_t reg = 0x41;  // Temperatura
    uint8_t data[2];
    
    i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDR,
                                 &reg, 1, data, 2,
                                 pdMS_TO_TICKS(100));
    
    int16_t raw = (int16_t)((data[0] << 8) | data[1]);
    return raw;  // °C = raw/340 + 36.53
}
```

### Conversão de Valores Brutos

```c
// Aceleração (com escala ±16g)
float ax_m_s2 = ax_raw * (9.81 / 2048.0);  // m/s²

// Temperatura (valores brutos)
float temp_celsius = (temp_raw / 340.0) + 36.53;

// Exemplo:
// ax_raw = 16384 → ~9.81 m/s² (gravidade em Z)
```

### Exemplo de Leitura

```c
void task_ler_mpu6050(void *pvParameters) {
    i2c_init();
    
    int16_t ax, ay, az;
    
    while (1) {
        ler_mpu6050(&ax, &ay, &az);
        
        printf("MPU6050: AX=%6d | AY=%6d | AZ=%6d\n", ax, ay, az);
        
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1x por segundo
    }
}
```

### Troubleshooting MPU6050

| Sintoma | Causa | Solução |
|---------|-------|---------|
| Valores todos zeros | Sensor não acordado | Chamar i2c_init() antes |
| Timeout I2C | GPIO 8/9 não conectado | Verificar diagram.json |
| Valores erráticos | I2C CLK lenta | Aumentar freq para 400kHz |
| Device not found | Endereço incorreto | Implementar I2C scan |
| AZ ~16384 (esperado) | Sensor horizontal OK | Inclinar para testar |

---

## 🔗 Integração no Código

### Estrutura Modular

```c
// main.c - versão modular

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_timer.h"

// Forward declarations
void i2c_init(void);
int ler_dht22(float *temperatura, float *umidade);
void ler_mpu6050(int16_t *ax, int16_t *ay, int16_t *az);

// Tasks
void task_dht22(void *pvParameters) {
    float temp = 0.0f, hum = 0.0f;
    
    while (1) {
        if (ler_dht22(&temp, &hum) == 0) {
            printf("DHT22: %.1f°C, %.1f%%\n", temp, hum);
        } else {
            printf("DHT22: Erro\n");
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void task_mpu6050(void *pvParameters) {
    int16_t ax, ay, az;
    
    while (1) {
        ler_mpu6050(&ax, &ay, &az);
        printf("MPU6050: X=%6d Y=%6d Z=%6d\n", ax, ay, az);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    printf("=== Sistema de Sensores ===\n");
    
    // Inicializar I2C
    i2c_init();
    
    // Criar tasks
    xTaskCreate(task_dht22, "DHT22", 2048, NULL, 5, NULL);
    xTaskCreate(task_mpu6050, "MPU6050", 2048, NULL, 5, NULL);
    
    // FreeRTOS inicia automaticamente
}
```

### Sincronização com Mutex

```c
#include "freertos/semphr.h"

static SemaphoreHandle_t mutex_sensors = NULL;

void app_main(void) {
    mutex_sensors = xSemaphoreCreateMutex();
    
    // Tasks usando mutex para dados compartilhados
    xTaskCreate(task_dht22, "DHT22", 2048, NULL, 5, NULL);
}

// Usar mutex em dados compartilhados
float temperatura_global = 0.0f;

void atualizar_temperatura(float new_temp) {
    if (xSemaphoreTake(mutex_sensors, pdMS_TO_TICKS(100))) {
        temperatura_global = new_temp;
        xSemaphoreGive(mutex_sensors);
    }
}
```

---

## 📊 Calibração e Validação

### Validação de Dados DHT22

```c
bool validar_dht22(float temp, float hum) {
    // Faixas esperadas
    if (temp < -40.0f || temp > 125.0f) {
        ESP_LOGW("DHT22", "Temperatura fora de range: %.1f°C", temp);
        return false;
    }
    
    if (hum < 0.0f || hum > 100.0f) {
        ESP_LOGW("DHT22", "Umidade fora de range: %.1f%%", hum);
        return false;
    }
    
    // Ambientes realísticos: 15-40°C, 20-95% RH
    if (temp < 10.0f || temp > 50.0f) {
        ESP_LOGW("DHT22", "Temperatura suspeita: %.1f°C", temp);
    }
    
    return true;
}
```

### Calibração MPU6050

```c
// Offset para Zero-G em Z
int16_t offset_az = 0;

void calibrar_mpu6050(void) {
    printf("[MPU6050] Calibrando... Mantenha o sensor parado!\n");
    
    int32_t az_sum = 0;
    const int samples = 100;
    
    for (int i = 0; i < samples; i++) {
        int16_t ax, ay, az;
        ler_mpu6050(&ax, &ay, &az);
        az_sum += az;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    offset_az = az_sum / samples - 16384;  // 16384 = 1g
    printf("[MPU6050] Offset calibrado: %d\n", offset_az);
}

void ler_mpu6050_calibrado(int16_t *ax, int16_t *ay, int16_t *az) {
    ler_mpu6050(ax, ay, az);
    *az -= offset_az;  // Aplicar offset
}
```

### Validação MPU6050

```c
bool validar_mpu6050(int16_t ax, int16_t ay, int16_t az) {
    // Faixa: ±32768 (16-bit signed)
    // Escala: ±16g = ±156 m/s²
    
    // Gravidade é sempre ~9.81 m/s², distribuída em 3 eixos
    float g = sqrt((ax*ax + ay*ay + az*az) / pow(2048.0, 2)) * 9.81;
    
    if (g < 8.0f || g > 11.0f) {
        ESP_LOGW("MPU6050", "Gravidade anômala: %.2f m/s²", g);
        return false;
    }
    
    return true;
}
```

---

## 🎮 Simulação Wokwi

### Configurar Wokwi

**diagram.json:**
```json
{
  "version": 1,
  "author": "Seu Nome",
  "parts": [
    {
      "type": "board-esp32-s3-devkitc-1",
      "id": "esp",
      "top": 0,
      "left": 150
    },
    {
      "type": "wokwi-dht22",
      "id": "dht1",
      "top": 77.1,
      "left": 378.6,
      "attrs": { "temperature": "25" }
    },
    {
      "type": "wokwi-mpu6050",
      "id": "imu1",
      "top": 282.22,
      "left": -26.48
    }
  ],
  "connections": [
    [ "esp:TX", "$serialMonitor:RX", "", [] ],
    [ "esp:RX", "$serialMonitor:TX", "", [] ],
    [ "dht1:VCC", "esp:3V3.1", "red", [] ],
    [ "dht1:SDA", "esp:15", "green", [] ],
    [ "dht1:GND", "esp:GND.2", "black", [] ],
    [ "imu1:VCC", "esp:3V3.2", "red", [] ],
    [ "imu1:GND", "esp:GND.1", "black", [] ],
    [ "imu1:SDA", "esp:8", "blue", [] ],
    [ "imu1:SCL", "esp:9", "violet", [] ]
  ]
}
```

**wokwi.toml:**
```toml
[wokwi]
version = 1
elf = "build/sensor-dht11-esp32.elf"
firmware = "build/flasher_args.json"
```

### Simular Condições Ambientais

```javascript
// No simulador Wokwi (JavaScript):
// Variar temperatura do DHT22
chip.setAttr("dht1", "temperature", "20");  // Mudar para 20°C
chip.setAttr("dht1", "humidity", "60");     // Umidade 60%

// Simular movimento no MPU6050
chip.setAttr("imu1", "pitch", "45");  // Inclinar 45°
```

---

## ⚡ Otimizações e Performance

### Reduzir Consumo de Energia

```c
// Modo Light Sleep: WiFi desligado, cores dormindo
esp_light_sleep_start();

// Modo Deep Sleep (requer RTC): CPU/RAM dormindo
esp_sleep_enable_timer_wakeup(10 * 1000000);  // 10 segundos
esp_deep_sleep_start();
```

### Otimizar Taxa de Amostragem

```c
// Frequência diferentes para cada sensor
void app_main(void) {
    xTaskCreate(task_dht22, "DHT22", 2048, NULL, 5, NULL);   // 2s
    xTaskCreate(task_mpu6050, "MPU6050", 1024, NULL, 6, NULL); // 1s
    // MPU6050 tem prioridade maior (6 > 5)
}
```

### Uso de Stack e Heap

```c
// Monitorar memória
void task_monitor_memoria(void *pvParameters) {
    while (1) {
        size_t free_heap = esp_get_free_heap_size();
        size_t min_free = esp_get_minimum_free_heap_size();
        
        printf("Heap: %u bytes, Min: %u bytes\n", free_heap, min_free);
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
```

### Buffer Circular para Dados

```c
#define BUFFER_SIZE 100

typedef struct {
    float temp;
    float hum;
    int64_t timestamp;
} dht22_sample_t;

static dht22_sample_t buffer[BUFFER_SIZE];
static int buffer_index = 0;

void salvar_amostra(float temp, float hum) {
    buffer[buffer_index].temp = temp;
    buffer[buffer_index].hum = hum;
    buffer[buffer_index].timestamp = esp_timer_get_time();
    
    buffer_index = (buffer_index + 1) % BUFFER_SIZE;  // Circular
}
```

---

## 🚀 Extensões Avançadas

### 1. Envio WiFi com MQTT

```c
#include "mqtt_client.h"

void event_handler(void *arg, esp_event_base_t event_base,
                   int32_t event_id, void *event_data) {
    if (event_id == MQTT_EVENT_CONNECTED) {
        printf("MQTT conectado!\n");
    }
}

void iniciar_mqtt(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.mosquitto.org",
    };
    
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, 
                                   event_handler, NULL);
    esp_mqtt_client_start(client);
}
```

### 2. Armazenamento SD Card

```c
#include "esp_vfs_fat.h"

void escrever_sd(float temp, float hum) {
    FILE *f = fopen("/sdcard/dados.csv", "a");
    if (f != NULL) {
        fprintf(f, "%.1f,%.1f,%lld\n", temp, hum, esp_timer_get_time());
        fclose(f);
    }
}
```

### 3. Web Server REST

```c
#include "esp_http_server.h"

esp_err_t sensor_get_handler(httpd_req_t *req) {
    char buf[100];
    snprintf(buf, sizeof(buf), 
             "{\"temp\":%.1f,\"humidity\":%.1f}", 
             temperatura_global, umidade_global);
    
    httpd_resp_send(req, buf, strlen(buf));
    return ESP_OK;
}
```

### 4. Análise FFT (Frequência)

```c
#include "esp_dsp.h"

void analisar_vibracao(int16_t *samples, int n) {
    float *input = (float *)malloc(n * sizeof(float));
    for (int i = 0; i < n; i++) {
        input[i] = samples[i] / 2048.0f;  // Normalizar
    }
    
    // dsps_fft2r_fc32(input, n);  // FFT
    // Analisar frequências...
    
    free(input);
}
```

---

## 🔌 Referência de Pinagem

### ESP32-S3 DevKit-C 1

```
┌─────────────────────────────────────┐
│  USB-C (Programação)                │
├─────────────────────────────────────┤
│ GND │ IO46 │ IO45 │ IO44(RX) │      │  Port
│ │GND│ 3V3  │  IO43(TX)  │      │
│ │   │      │      │      │         │
│ ┌───┴──────┴──────┴──────┴────┐    │
│ │  ESP32-S3 (Dual-Core 240MHz)│    │
│ │  34 GPIO Pins, WiFi, BLE    │    │
│ └─────────────────────────────┘    │
│                                     │
│ I2C MASTER:     GPIO 8 (SDA)       │
│                 GPIO 9 (SCL)       │
│                                     │
│ DHT22:          GPIO 15            │
│                                     │
│ GND (2x):       GND.1, GND.2       │
│ 3V3 (2x):       3V3.1, 3V3.2       │
│                                     │
│ Serial Monitor: RX (GPIO 44)       │
│                 TX (GPIO 43)       │
└─────────────────────────────────────┘
```

### Tabela de Pinos I2C

| Função | ESP32 GPIO | Descrição |
|--------|-----------|-----------|
| SDA | 8 | Serial Data (dados I2C) |
| SCL | 9 | Serial Clock (clock I2C) |
| GND | GND | Terra (referência) |
| 3V3 | 3V3 | Alimentação 3.3V |

### Tabela de Pinos GPIO Livres

```
Usáveis para expansão:
GPIO 1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 16, 17
GPIO 18, 19, 20, 21, 22, 23, 25, 26, 27, 28, 29, 30, 31, 32

Entrada apenas (sem pull-up):
GPIO 34, 35, 36, 37, 38, 39

Pinos especiais (evitar):
GPIO 43, 44 (Serial RX/TX)
GPIO 0 (Boot/Reset)
GPIO 46 (Boot)
```

---

## 📚 Referências e Recursos

### Datasheets Oficiais

- [DHT22 Datasheet](https://cdn-shop.adafruit.com/datasheets/DHT22.pdf)
- [MPU-6050 Register Map](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf)
- [ESP32-S3 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)

### Documentação Online

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [Wokwi Documentation](https://docs.wokwi.com/)

### Comunidades

- [ESP32 Forum](https://www.esp32.com/viewforum.php?f=2)
- [Arduino ESP32 GitHub](https://github.com/espressif/arduino-esp32)
- [Wokwi Discussions](https://github.com/wokwi/wokwi-features/discussions)

---

## ✅ Checklist Final

Antes de publicar seu projeto:

```
DOCUMENTAÇÃO:
☐ Código comentado (inline e funções)
☐ Protocolos documentados
☐ Pinagem clara
☐ Calibração descrita

VALIDAÇÃO:
☐ Compilação sem warnings
☐ Testes em Wokwi OK
☐ Testes em hardware OK (se disponível)
☐ Uptime > 10 minutos
☐ Dados realísticos

SEGURANÇA:
☐ Sem overflow de buffer
☐ Tratamento de erros presente
☐ Timeouts implementados
☐ Verificação de checksum

PERFORMANCE:
☐ Taxa de amostragem adequada
☐ Uso de memória OK (heap > 2KB)
☐ Nenhum travamento
☐ Logs informativos
```

---

**Última atualização:** 24 de setembro de 2026  
**Versão:** 1.0  
**Compatível com:** ESP-IDF v4.1+, Wokwi, ESP32-S3
