# 🔍 Checklist de Debugging - ESP32-S3 com DHT22 + MPU6050

**Guia sistemático para identificar e resolver problemas no projeto**

---

## 📋 Índice Rápido

1. [Pré-Compilação](#-pré-compilação)
2. [Compilação](#-compilação)
3. [Configuração ESP-IDF](#-configuração-esp-idf)
4. [Ambiente Wokwi](#-ambiente-wokwi)
5. [Hardware/Simulação](#-hardwaresimulação)
6. [DHT22 Isolado](#-dht22-isolado)
7. [MPU6050 Isolado](#-mpu6050-isolado)
8. [Integração Completa](#-integração-completa)
9. [Testes de Performance](#-testes-de-performance)
10. [Diagnóstico Avançado](#-diagnóstico-avançado)

---

## ✅ PRÉ-COMPILAÇÃO

### Estrutura de Arquivos

```
☐ Existe arquivo: main/main.c
☐ Existe arquivo: main/CMakeLists.txt (componente)
☐ Existe arquivo: CMakeLists.txt (raiz)
☐ Existe arquivo: diagram.json (Wokwi)
☐ Existe arquivo: wokwi.toml
☐ Existe arquivo: sdkconfig
```

**Se algum faltar:**
```bash
# Verificar estrutura
ls -la main/
ls -la .

# Copiar arquivos de backup se necessário
cp CMakeLists.txt.bak CMakeLists.txt
```

### Verificações de Caminho

```
☐ Nenhum espaço em branco no caminho do projeto
☐ Nenhum caractere especial (~, !, @, #, $, %)
☐ Caminho absoluto máximo < 260 caracteres (Windows)
```

**Verificar caminho atual:**
```bash
pwd
# Saída esperada: /home/usuario/sensor-dht11-esp32
# NÃO esperado: /home/usuario/My Projects/sensor-dht11-esp32
```

### Permissões de Arquivo

```bash
# Linux/macOS - verificar permissões
☐ chmod +x main/main.c
☐ chmod +x CMakeLists.txt
☐ chmod 644 sdkconfig
```

### Variáveis de Ambiente

```bash
# Verificar IDF_PATH
☐ echo $IDF_PATH
# Deve retornar: /path/to/esp-idf

# Se vazio:
☐ source ~/esp-idf/export.sh
# Ou no Windows CMD:
☐ %IDF_PATH%\export.bat
```

---

## 🔨 COMPILAÇÃO

### Limpeza Inicial

```bash
# Passo 1: Remover build anterior
☐ idf.py clean
# Aguardar conclusão (sem erros)

# Passo 2: Remover cache CMake (se persiste erro)
☐ rm -rf build/
☐ rm -rf .pytest_cache/
```

### Definir Alvo Correto

```bash
# Passo 1: Verificar alvo atual
☐ idf.py --version
# Saída esperada: ESP-IDF v6.1 ou v5.1+

# Passo 2: Definir alvo ESP32-S3
☐ idf.py set-target esp32s3
# Aguardar: "Set target to: esp32s3"

# Passo 3: Verificar arquivo gerado
☐ ls -la sdkconfig
# Deve existir e ter ~10 KB
```

### Compilação Básica

```bash
# Passo 1: Build
☐ idf.py build
# Aguardar: "Build complete"
# Tempo esperado: 30-120 segundos (primeiro build)

# Passo 2: Verificar binário
☐ ls -la build/
# Deve conter: sensor-dht11-esp32.elf
☐ ls -la build/esp-idf/
# Deve conter subdiretorios dos componentes
```

### Erros Comuns de Compilação

#### Erro: "cmake not found"
```bash
☐ pip install cmake
☐ cmake --version
# Esperado: cmake version 3.5+
```

#### Erro: "xtensa-esp32s3-elf-gcc: command not found"
```bash
☐ source ~/esp-idf/export.sh
# Ou verificar IDF_PATH
☐ echo $IDF_PATH
```

#### Erro: "undefined reference to `main`"
```bash
☐ Verificar main/CMakeLists.txt contém:
   idf_component_register(SRCS "main.c" ...)
☐ Compilar novamente: idf.py build
```

#### Erro: "error: field has incomplete type"
```bash
☐ Verificar includes em main.c:
   #include "driver/i2c.h"
   #include "driver/gpio.h"
   #include "freertos/FreeRTOS.h"
☐ Compilar: idf.py build
```

---

## ⚙️ CONFIGURAÇÃO ESP-IDF

### Menu de Configuração

```bash
# Abrir configurador interativo
☐ idf.py menuconfig

# Navegar até:
Component config → Common ESP-related → UART Baudrate
☐ Verificar: 115200 baud (padrão)

# Monitorar saída:
Component config → esp_rom → Logging
☐ Verificar: Default log level = INFO
```

**Configurações críticas:**
```
☐ Serial baudrate: 115200
☐ ESP-IDF version: >= 4.1.0
☐ Target: esp32s3
☐ Optimization: -O2 (default)
```

### Validar sdkconfig

```bash
# Verificar se arquivo foi salvo
☐ cat sdkconfig | grep BAUDRATE
# Esperado: CONFIG_ESP_CONSOLE_UART_BAUDRATE=115200

# Verificar alvo
☐ cat sdkconfig | grep IDF_TARGET
# Esperado: CONFIG_IDF_TARGET="esp32s3"
```

---

## 🎮 AMBIENTE WOKWI

### Verificar Instalação

```bash
# Passo 1: Extensão no VS Code
☐ Abrir: Extensions (Ctrl+Shift+X)
☐ Procurar: "Wokwi Simulator"
☐ Status: Deve estar instalada

# Passo 2: Gerar token Wokwi
☐ Acessar: https://wokwi.com/dashboard/ci
☐ Copiar token (40 caracteres)
☐ VS Code → Settings → Wokwi → API Token
☐ Colar token
```

### Configurar wokwi.toml

```bash
# Verificar conteúdo
☐ cat wokwi.toml

# Deve conter:
[wokwi]
version = 1
elf = "build/sensor-dht11-esp32.elf"
firmware = "build/flasher_args.json"

# Se diferente:
☐ Editar e atualizar caminhos
```

### Diagrama JSON

```bash
# Verificar diagram.json
☐ cat diagram.json | head -20

# Esperado: seção "parts" com:
☐ board-esp32-s3-devkitc-1
☐ wokwi-dht22 (pino 15)
☐ wokwi-mpu6050 (pinos I2C 8/9)

# Esperado: seção "connections" com:
☐ dht1:VCC → esp:3V3
☐ dht1:SDA → esp:15
☐ dht1:GND → esp:GND
☐ imu1:SDA → esp:8
☐ imu1:SCL → esp:9
```

### Iniciar Simulação

```bash
# Via VS Code
☐ Cmd Palette: Ctrl+Shift+P
☐ Digitar: "Wokwi: Start Simulator"
☐ Aguardar janela abrir (20-30 segundos)

# Verificações:
☐ Circu Wokwi renderizado
☐ Monitor serial visível (abaixo do circuito)
☐ Nenhum erro vermelho em "Logs"
```

---

## 🔌 HARDWARE/SIMULAÇÃO

### Verificações do Circuito Wokwi

```
DHT22 (wokwi-dht22):
☐ Pinagem visual no Wokwi:
  ├─ VCC (vermelho) → 3V3.1
  ├─ SDA (verde) → GPIO 15
  └─ GND (preto) → GND.2

MPU6050 (wokwi-mpu6050):
☐ Pinagem visual no Wokwi:
  ├─ VCC (vermelho) → 3V3.2
  ├─ GND (preto) → GND.1
  ├─ SDA (azul) → GPIO 8
  └─ SCL (roxo) → GPIO 9
```

**Se pinos incorretos:**
```
1. Clicar em componente no Wokwi
2. Verificar "Connections" panel
3. Arrastar pino correto
4. Salvar diagram.json
5. Reiniciar simulação
```

### Verificação de Alimentação

```
☐ ESP32-S3 tem indicador LED de potência (aceso)
☐ DHT22 exibe temperatura inicial (~20°C)
☐ MPU6050 aceita conexões I2C
```

---

## 🌡️ DHT22 ISOLADO

### Teste de Comunicação Básica

```c
// Teste minimalista no main.c
void app_main(void) {
    float temp = 0.0f, hum = 0.0f;
    
    while (1) {
        int ret = ler_dht22(&temp, &hum);
        
        if (ret == 0) {
            printf("✓ DHT22 OK: %.1f°C, %.1f%%\n", temp, hum);
        } else {
            printf("✗ DHT22 ERRO: %d\n", ret);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
```

### Verificações no Monitor Serial

```
☐ Mensagens aparecem a cada 2 segundos
☐ Valores de temperatura: 15–40 °C (realistico)
☐ Valores de umidade: 20–90 % (realistico)
☐ Checksum válido (sem erros -1)
```

**Se valores errados:**

| Sintoma | Causa | Solução |
|---------|-------|--------|
| Sempre -1 (erro) | Timeout na leitura | Verificar GPIO 15 em diagram.json |
| Valores ~0 °C | Sensor desligado | Verificar VCC em diagram.json |
| Valores aleatórios | Ruído de sinal | Adicionar capacitor 100nF entre VCC-GND |
| Sem saída | Task não criada | Verificar FreeRTOS |

### Debug Passo-a-Passo

```c
// Adicionar logs detalhados
void ler_dht22(float *temperatura, float *umidade) {
    printf("[DHT22] Iniciando leitura...\n");
    
    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    printf("[DHT22] Pull-down iniciado\n");
    gpio_set_level(DHT_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    
    printf("[DHT22] Aguardando resposta...\n");
    gpio_set_level(DHT_PIN, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);
    
    // ... resto do código com printf()
}
```

---

## 🎯 MPU6050 ISOLADO

### Teste de Comunicação I2C

```c
// Teste minimalista
void app_main(void) {
    i2c_init();
    
    int16_t ax, ay, az;
    
    while (1) {
        ler_mpu6050(&ax, &ay, &az);
        printf("MPU6050: AX=%d, AY=%d, AZ=%d\n", ax, ay, az);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
```

### Verificações no Monitor Serial

```
☐ Valores de aceleração entre -32768 e +32767
☐ AZ próximo de 16384 (gravidade em Z)
☐ Valores estáveis (desvio < 100 unidades)
☐ Nenhuma mensagem de erro I2C
```

**Se valores errados:**

| Sintoma | Causa | Solução |
|---------|-------|--------|
| Sempre 0 | Sensor não acordado | Verificar i2c_init() |
| Aleatórios | I2C não inicializado | Executar i2c_init() antes |
| AZ ~0 | Sensor horizontalizado | Inclinar ESP32 |
| Erro timeout I2C | Conexão GPIO 8/9 | Verificar diagram.json |

### Verificar Endereço I2C

```c
// Adicionar scan I2C
void scan_i2c(void) {
    printf("Scaneando I2C...\n");
    for (int addr = 0x01; addr < 0x7F; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            printf("Dispositivo encontrado no endereço: 0x%02X\n", addr);
        }
    }
}
```

---

## 🔗 INTEGRAÇÃO COMPLETA

### Teste Simultâneo dos Sensores

```
☐ Wokwi iniciada com ambos sensores
☐ Monitor serial mostrando DHT22 E MPU6050
☐ Dados atualizando a cada 2 segundos
☐ Nenhuma mensagem de erro
```

### Saída Esperada

```
--- Leitura dos Sensores ---
DHT22   | Temperatura: 25.3 C | Humidade: 58.5 %
MPU6050 | AcelX: 1024 | AcelY: -256 | AcelZ: 16384

--- Leitura dos Sensores ---
DHT22   | Temperatura: 25.4 C | Humidade: 58.6 %
MPU6050 | AcelX: 1032 | AcelY: -248 | AcelZ: 16390
```

### Sincronização de Tarefas

```
☐ Frequência estável (2 segundos entre leituras)
☐ Nenhum travamento (watchdog não acionado)
☐ Memória estável (heap não cresce indefinidamente)
```

**Monitorar heap:**
```c
printf("Heap livre: %lu bytes\n", esp_get_free_heap_size());
```

---

## 📊 TESTES DE PERFORMANCE

### Latência de Leitura

```bash
# Adicionar timestamps
printf("[%lld ms] DHT22 lido\n", esp_timer_get_time() / 1000);

# Esperado:
# Leitura DHT22: ~5-10 ms
# Leitura MPU6050: ~2-5 ms
```

### Taxa de Amostragem

```c
// Medir intervalo entre leituras
static int64_t last_read = 0;
int64_t now = esp_timer_get_time();
printf("Intervalo: %lld ms\n", (now - last_read) / 1000);
last_read = now;

// Esperado: ~2000 ms (2 segundos)
```

### Consumo de Stack

```c
// Verificar uso de stack
TaskHandle_t xHandle = xTaskGetCurrentTaskHandle();
UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(xHandle);
printf("Stack disponível: %u bytes\n", uxHighWaterMark * sizeof(StackType_t));

// Esperado: > 1024 bytes
```

---

## 🔬 DIAGNÓSTICO AVANÇADO

### Habilitar Logs Detalhados

```bash
# Via menuconfig
idf.py menuconfig
# Component config → Log output
# Logging level → DEBUG

# Ou compile com:
idf.py build -DLOG_LOCAL_LEVEL=ESP_LOG_DEBUG
```

### Logs Customizados

```c
#include "esp_log.h"

static const char* TAG = "SENSORS";

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando...");
    ESP_LOGD(TAG, "Debug: heap_size=%lu", esp_get_free_heap_size());
    
    // ...
    
    ESP_LOGW(TAG, "Aviso: sensor lento");
    ESP_LOGE(TAG, "Erro: timeout I2C");
}
```

### Monitorar com idf.py monitor

```bash
# Conexão serial com mais opções
idf.py -p /dev/ttyUSB0 monitor --baudrate 115200

# Filtrar por tag:
idf.py monitor --elf=build/sensor-dht11-esp32.elf
```

### Análise de Core Dump

Se houver crash:

```bash
# Gerar arquivos de análise
idf.py build

# Flashear
idf.py -p /dev/ttyUSB0 flash

# Monitorar com decode
idf.py -p /dev/ttyUSB0 monitor --decode-coredump=esp_coredump
```

### Verificar Limites de Stack/Heap

```c
// Stack
#define STACK_SIZE 4096  // Ajustar se necessário

// Heap
void check_heap(void) {
    size_t free_heap = esp_get_free_heap_size();
    size_t min_free_heap = esp_get_minimum_free_heap_size();
    
    printf("Heap livre agora: %zu bytes\n", free_heap);
    printf("Heap mínimo durante execução: %zu bytes\n", min_free_heap);
    
    if (free_heap < 2048) {
        printf("⚠️ AVISO: Heap baixo!\n");
    }
}
```

---

## 🚨 CHECKLIST FINAL PRÉ-PRODUÇÃO

Antes de considerar o projeto pronto:

```
COMPILAÇÃO:
☐ Build completo sem warnings
☐ ELF gerado: build/sensor-dht11-esp32.elf
☐ Size otimizado (< 500 KB)

FUNCIONALIDADE:
☐ DHT22 retorna valores válidos
☐ MPU6050 retorna aceleração 3D
☐ Taxa de atualização: 2 segundos estável
☐ Nenhum travamento (uptime > 5 min)

QUALIDADE:
☐ Memória estável (heap não cresce)
☐ Stack adequado (> 1024 bytes livres)
☐ Logs claros e informativos
☐ Tratamento de erros presente

HARDWARE (se aplicável):
☐ Testado em ESP32-S3 físico
☐ Alimentação estável (5V)
☐ Pinos conectados corretamente
☐ Sensores respondendo

DOCUMENTAÇÃO:
☐ README.md completo
☐ QUICKSTART.md funcional
☐ Comentários no código
☐ Pinagem documentada
```

---

## 📞 Quando Buscar Ajuda

Se após esta checklist o problema persiste:

1. **Verificar logs completos:**
   ```bash
   idf.py monitor > debug.log 2>&1
   # Enviar debug.log para análise
   ```

2. **Descrever problema com:**
   - Mensagem de erro exata
   - Versão ESP-IDF (`idf.py --version`)
   - Sistema operacional
   - Passos para reproduzir

3. **Recursos oficiais:**
   - [ESP-IDF Docs](https://docs.espressif.com)
   - [Fórum Espressif](https://www.esp32.com/viewforum.php?f=21)
   - [Wokwi Docs](https://docs.wokwi.com)

---

**Última atualização:** 24 de setembro de 2026  
**Versão:** 1.0  
**Compatível com:** ESP-IDF v4.1+, ESP32-S3
