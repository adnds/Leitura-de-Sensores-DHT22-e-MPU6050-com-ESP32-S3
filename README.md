# Leitura de Sensores DHT22 e MPU6050 com ESP32-S3

**Projeto de aquisição de dados ambientais e inerciais em tempo real usando ESP32-S3, DHT22 e MPU6050**

---

## 📋 Visão Geral

Este projeto implementa um sistema embarcado completo para captura simultânea de:
- **Temperatura e Umidade** (sensor DHT22)
- **Aceleração em 3 eixos** (sensor MPU6050)

A solução é desenvolvida em **C** usando **ESP-IDF** e pode ser simulada via **Wokwi** ou executada em hardware real.

---

## 🎯 Características

✅ Leitura contínua de sensores DHT22 (temperatura/umidade)  
✅ Leitura de aceleração 3-eixos (MPU6050 via I2C)  
✅ Protocolo de comunicação personalizado para DHT22  
✅ Integração I2C master para MPU6050  
✅ Inicialização automática do MPU6050  
✅ Simulação validada em Wokwi  
✅ Compatível com ESP-IDF v6.1+  

---

## 🛠️ Requisitos

### Hardware (para execução real)
- **Microcontrolador**: ESP32-S3 DevKit-C 1
- **Sensor de Temperatura/Umidade**: DHT22
- **Sensor Inercial**: MPU6050
- **Cabos de conexão** e **alimentação 5V**

### Software
- **ESP-IDF**: v4.1.0 ou superior (v6.1 recomendado)
- **VS Code** com extensão ESP-IDF
- **Wokwi** (para simulação) – extensão ou plataforma web
- **Git**

### Dependências C
Incluídas automaticamente via ESP-IDF:
- `freertos` – multitarefa em tempo real
- `driver/gpio` – controle de pinos GPIO
- `driver/i2c` – comunicação I2C
- `esp_timer` – temporização de alta precisão

---

## 📦 Estrutura do Projeto

```
sensor-dht11-esp32/
├── CMakeLists.txt                 # Configuração raiz (cmake)
├── main/
│   ├── CMakeLists.txt             # Configuração componente main
│   └── main.c                     # Código principal C
├── diagram.json                   # Circuito Wokwi
├── wokwi.toml                     # Config Wokwi/simulação
├── sdkconfig                      # Configurações SDK do ESP-IDF
├── QUICKSTART.md                  # Guia rápido (5 passos)
├── devcontainer.json              # Ambiente Docker isolado
├── .vscode/
│   ├── c_cpp_properties.json       # Intelisense C/C++
│   ├── launch.json                # Debug configuration
│   └── settings.json              # VS Code settings
├── .gitignore                     # Exclusões Git
├── dependencies.lock              # Lock das dependências IDF
└── README.md                      # Esta documentação
```

---

## 🚀 Instalação e Configuração

### 1. Clonar o Repositório

```bash
git clone <seu-repositorio> sensor-dht11-esp32
cd sensor-dht11-esp32
```

### 2. Configurar o Alvo (ESP32-S3)

```bash
idf.py set-target esp32s3
```

### 3. Instalar Dependências (se necessário)

```bash
idf.py install
```

### 4. Compilar o Projeto

**Via CLI:**
```bash
idf.py build
```

**Via VS Code:**
- Pressionar `Ctrl+Alt+B`
- Ou: Cmd Palette (`Ctrl+Shift+P`) → "ESP-IDF: Build Project"
- Aguardar mensagem: `Build complete`

### 5. Executar

**Simulação Wokwi:**
- Cmd Palette (`Ctrl+Shift+P`) → "Wokwi: Start Simulator"
- Monitor serial abrirá automaticamente

**Hardware Real:**
```bash
idf.py -p COM3 flash monitor
```
(Substituir `COM3` pela porta serial do ESP32)

---

## 📊 Conexões de Hardware

### DHT22
| Pino DHT22 | Pino ESP32-S3 | Cor |
|:----------:|:-------------:|:---:|
| VCC        | 3V3.1         | 🔴 |
| SDA        | GPIO 15       | 🟢 |
| GND        | GND.2         | ⚫ |

### MPU6050 (I2C Master)
| Pino MPU6050 | Pino ESP32-S3 | Cor     |
|:------------:|:-------------:|:-------:|
| VCC          | 3V3.2         | 🔴 |
| GND          | GND.1         | ⚫ |
| SDA          | GPIO 8        | 🔵 |
| SCL          | GPIO 9        | 🟣 |

---

## 💻 Uso

### Compilar e Executar

```bash
# Limpar build anterior (se necessário)
idf.py clean

# Compilar
idf.py build

# Flashear e monitorar (hardware real)
idf.py -p /dev/ttyUSB0 flash monitor

# Interromper monitor: Ctrl+]
```

### Saída Esperada no Monitor Serial

```
--- Leitura dos Sensores ---
DHT22   | Temperatura: 25.3 C | Humidade: 58.5 %
MPU6050 | AcelX: 1024 | AcelY: -256 | AcelZ: 16384

--- Leitura dos Sensores ---
DHT22   | Temperatura: 25.4 C | Humidade: 58.6 %
MPU6050 | AcelX: 1032 | AcelY: -248 | AcelZ: 16390
```

**Frequência:** Dados atualizados a cada 2 segundos

---

## 🔧 Detalhes Técnicos

### Protocolo DHT22

O código implementa o protocolo proprietário do DHT22:

1. **Envio de comando** (GPIO em nível baixo por 20 ms)
2. **Resposta do sensor** (pulsos de duração variável)
3. **Decodificação de bits** (40 bits: 2 bytes umidade + 2 bytes temperatura + 1 checksum)
4. **Validação** (CRC-8 automático)

**Pins usados:**
```c
#define DHT_PIN 15  // GPIO para comunicação DHT22
```

**Fórmula de conversão:**
- Temperatura: `raw_value / 10.0` °C
- Umidade: `raw_value / 10.0` %RH

### Comunicação I2C (MPU6050)

**Configuração:**
- Modo: Master
- Velocidade: 100 kHz
- Endereço MPU6050: `0x68`

**Registros lidos:**
```c
#define MPU6050_ACCEL_X_REG 0x3B  // 2 bytes big-endian
#define MPU6050_ACCEL_Y_REG 0x3D
#define MPU6050_ACCEL_Z_REG 0x3F
```

**Inicialização:**
```c
uint8_t cmd[2] = {0x6B, 0x00};  // Power Management - acordar sensor
i2c_master_write_to_device(...);
```

---

## 🐛 Troubleshooting

| Problema | Causa | Solução |
|----------|-------|---------|
| `"Cannot find build"` | Arquivo build corrompido | `idf.py clean` + `idf.py build` |
| **Wokwi não inicia** | Extensão não instalada | Instalar "Wokwi Simulator for VS Code" |
| **Monitor serial vazio** | Sensor lê a cada 2s | Aguarde 2–3 segundos |
| **Erro de compilação: "undefined reference"** | CMakeLists.txt incorreto | Verificar paths e `PRIV_REQUIRES` |
| **DHT22 retorna erro (-1)** | Timeout na leitura | Verificar conexão do pino 15 |
| **MPU6050 não responde** | I2C não inicializado | Verificar pins SDA (8) e SCL (9) |
| **Valores aleatórios MPU6050** | Sensor não acordado | Confirmar comando de wake-up em `i2c_init()` |
| **Serial mostra lixo** | Baud rate incompatível | Verificar `idf.py menuconfig` (padrão: 115200) |

---

## 📈 Extensões Possíveis

### 1. **Armazenamento em SD Card**
Adicionar suporte a escrita de dados em cartão microSD:
```c
// Incluir: esp_vfs_fat_sdmmc_mount()
// Usar: fopen("sdcard/sensor_log.csv", "a")
```

### 2. **Transmissão WiFi**
Enviar dados para servidor MQTT ou HTTP:
```c
// Incluir: esp_wifi.h, mqtt_client.h
// Conectar a rede WiFi + publicar tópicos MQTT
```

### 3. **Salva em SPIFFS**
Armazenar histórico na memória flash do ESP32:
```c
// Incluir: esp_spiffs.h
// Implementar buffer circular de últimas 1000 leituras
```

### 4. **Interface Web com Captive Portal**
Servir página HTML com dados em tempo real:
```c
// Incluir: esp_http_server.h
// Endpoint GET /api/sensors retorna JSON
```

### 5. **Processamento de Sinais (FFT)**
Analisar frequências de vibração do MPU6050:
```c
// Incluir: esp_dsp.h
// Computar transformada de Fourier nos 3 eixos
```

---

## 📄 Arquivos de Configuração

### `sdkconfig`
Configurações do ESP-IDF (geradas por `idf.py menuconfig`):
- Baud rate: 115200
- Otimizações: `-O2`
- Componentes necessários: driver GPIO, I2C, timer

### `wokwi.toml`
Define entrada/saída para simulador:
```toml
[wokwi]
version = 1
elf = "build/sensor-dht11-esp32.elf"
firmware = "build/flasher_args.json"
```

### `devcontainer.json`
Ambiente Docker para desenvolvimento isolado:
- Imagem: `espressif/idf:release-v6.1`
- Volume montado na pasta do projeto

---

## 📝 Gitignore

Arquivos automaticamente gerados (não devem ser versionados):
```
build/
sdkconfig.old
.DS_Store
.vscode/.history/
```

---

## 🔬 Validação e Testes

### Checklist de Funcionalidade

- [ ] Compilação sem erros
- [ ] Simulação Wokwi iniciando
- [ ] DHT22 retornando valores válidos (15–40 °C)
- [ ] MPU6050 retornando acelerações (±16g)
- [ ] Taxa de atualização: 1 leitura a cada 2 segundos
- [ ] Checksum DHT22 validado corretamente
- [ ] I2C respondendo com ACK do MPU6050

### Testes em Hardware Real

```bash
# Compilar com otimizações
idf.py build

# Flashear
idf.py -p /dev/ttyUSB0 flash

# Monitorar saída (115200 baud)
idf.py -p /dev/ttyUSB0 monitor
```

---

## 📚 Referências

- [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/en/stable/)
- [DHT22 Datasheet](https://cdn-shop.adafruit.com/datasheets/DHT22.pdf)
- [MPU-6050 Register Map](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf)
- [Wokwi Simulator](https://wokwi.com/)
- [FreeRTOS Docs](https://www.freertos.org/Documentation/161522_FreeRTOS_Quick_Start_Guide.pdf)

---

## 📧 Suporte e Contribuições

Para dúvidas, bugs ou sugestões:
1. Verificar [Troubleshooting](#-troubleshooting)
2. Consultar guia [QUICKSTART.md](QUICKSTART.md)
3. Abrir issue no repositório

---

## 📜 Licença

Este projeto é fornecido como referência educacional e de pesquisa.

---

**Última atualização:** 23 de setembro de 2026  
**Versão:** 1.0.0  
**Autor:** Adilson Nascimento
# Leitura-de-Sensores-DHT22-e-MPU6050-com-ESP32-S3
