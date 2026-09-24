# ⚡ Quick Start - ESP32-S3 com DHT22 + MPU6050

**Comece em 15 minutos! Guia passo-a-passo para compilar, simular e testar o projeto**

---

## 📋 Índice

1. [Pré-Requisitos](#-pré-requisitos)
2. [Instalação Rápida](#-instalação-rápida-esp-idf)
3. [Clonar e Configurar Projeto](#-clonar-e-configurar-projeto)
4. [Compilar](#-compilar-e-verificar)
5. [Simular no Wokwi](#-simular-no-wokwi)
6. [Testar em Hardware Real](#-testar-em-hardware-real)
7. [Validação Rápida](#-validação-rápida)
8. [Troubleshooting Express](#-troubleshooting-express)

---

## ✅ Pré-Requisitos

### Hardware (para simulação)
- 💻 Computador com Windows, macOS ou Linux
- 🌐 Conexão com internet (para Wokwi)

### Hardware (para teste real - OPCIONAL)
- 🔧 ESP32-S3 DevKit-C 1
- 🌡️ Sensor DHT22
- 📍 Sensor MPU6050
- 🔌 Cabo USB-C
- 🪁 Jumpers/fios de conexão

### Software
- **VS Code** (free) → https://code.visualstudio.com/
- **ESP-IDF** (free) → v4.1+ (instruções abaixo)
- **Git** (free) → https://git-scm.com/

---

## 🚀 Instalação Rápida ESP-IDF

### Windows (PowerShell Admin)

```powershell
# Passo 1: Navegar para local de instalação
cd C:\esp

# Passo 2: Clonar ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Passo 3: Instalar versão LTS
git checkout release/v6.1
git submodule update --init --recursive

# Passo 4: Executar instalador
.\install.bat esp32s3

# Passo 5: Ativar ambiente
.\export.bat
```

**Tempo esperado:** 15-20 minutos

### macOS / Linux

```bash
# Passo 1: Clonar ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Passo 2: Instalar versão LTS
git checkout release/v6.1
git submodule update --init --recursive

# Passo 3: Instalar dependências
cd ~/esp/esp-idf
./install.sh esp32s3

# Passo 4: Ativar ambiente (adicionar ao ~/.bashrc ou ~/.zshrc)
source ~/esp/esp-idf/export.sh
```

**Verificar instalação:**
```bash
idf.py --version
# Esperado: ESP-IDF v6.1.x
```

---

## 📁 Clonar e Configurar Projeto

### Opção A: Seu Repositório Git

```bash
# Passo 1: Clonar
git clone <seu-repositorio-url> sensor-dht11-esp32
cd sensor-dht11-esp32

# Passo 2: Verificar estrutura
ls -la
# Esperado: CMakeLists.txt, main/, diagram.json, wokwi.toml

# Passo 3: Definir alvo
idf.py set-target esp32s3
# Aguardar: "Set target to: esp32s3"
```

### Opção B: Criar do Zero

```bash
# Passo 1: Criar estrutura
mkdir sensor-dht11-esp32
cd sensor-dht11-esp32
mkdir main

# Passo 2: Copiar arquivos
# (Copiar main.c para main/)
# (Copiar CMakeLists.txt para raiz e main/)
# (Copiar diagram.json, wokwi.toml, sdkconfig)

# Passo 3: Definir alvo
idf.py set-target esp32s3
```

### Verificar Estrutura

```bash
tree -L 2
# Esperado:
# .
# ├── CMakeLists.txt
# ├── main/
# │   ├── CMakeLists.txt
# │   └── main.c
# ├── diagram.json
# ├── wokwi.toml
# └── sdkconfig
```

---

## 🔨 Compilar e Verificar

### Compilação Básica

```bash
# Passo 1: Limpar (primeira vez, opcional)
idf.py clean

# Passo 2: Compilar
idf.py build

# Resultado esperado:
# ✓ [100%] Built target app
# Build complete.
```

**⏱️ Tempo esperado:**
- Primeira compilação: 30-60 segundos
- Compilações seguintes: 5-10 segundos

### Verificar Binário

```bash
# Verificar arquivo gerado
ls -lh build/sensor-dht11-esp32.elf
# Esperado: ~300-500 KB

# Verificar componentes compilados
ls -la build/esp-idf/
# Deve conter: driver/, freertos/, esp_rom/, etc.
```

### Erros Comuns - Solução Rápida

| Erro | Solução |
|------|---------|
| `cmake not found` | `pip install cmake` |
| `Set target to: esp32s3` missing | `idf.py set-target esp32s3` |
| `Undefined reference main` | Verificar `main/CMakeLists.txt` |
| `Build complete` mas .elf não existe | `idf.py clean && idf.py build` |

---

## 🎮 Simular no Wokwi

### Pré-Setup Wokwi (primeira vez)

```bash
# Passo 1: Instalar extensão VS Code
# Extensions (Ctrl+Shift+X) → procurar "Wokwi"
# Instalar "Wokwi Simulator for VS Code"

# Passo 2: Gerar token (gratuito)
# Acesse: https://wokwi.com/dashboard/ci
# Copiar token (40 caracteres)

# Passo 3: Configurar no VS Code
# Settings (Ctrl+,) → procurar "wokwi"
# Colar token em "Wokwi: Embed Token"
```

### Iniciar Simulação

```bash
# Via VS Code (recomendado)
# Pressionar: Ctrl+Shift+P
# Digitar: "Wokwi: Start Simulator"
# Aguardar: Janela Wokwi abrir (20-30s)

# Ou via CLI:
wokwi-cli --elf build/sensor-dht11-esp32.elf diagram.json
```

### Visualizar Monitor Serial

```
Na janela do Wokwi:
1. Procurar aba "Serial" (abaixo do circuito)
2. Clique em "Serial"
3. Monitor abre mostrando dados

Esperado a cada 2 segundos:
--- Leitura dos Sensores ---
DHT22   | Temperatura: 25.3 C | Humidade: 58.5 %
MPU6050 | AcelX: 1024 | AcelY: -256 | AcelZ: 16384
```

### Verificar Circuito Wokwi

```
DHT22 (verde no diagram):
✓ VCC → 3V3.1 (vermelho)
✓ SDA → GPIO 15 (verde)
✓ GND → GND.2 (preto)

MPU6050 (roxo no diagram):
✓ VCC → 3V3.2 (vermelho)
✓ GND → GND.1 (preto)
✓ SDA → GPIO 8 (azul)
✓ SCL → GPIO 9 (roxo)
```

---

## 🔌 Testar em Hardware Real

### Conexões Físicas

**DHT22:**
```
ESP32-S3  ←→  DHT22
3V3       ←→  VCC (vermelho)
GPIO 15   ←→  SDA (verde)
GND       ←→  GND (preto)
```

**MPU6050:**
```
ESP32-S3  ←→  MPU6050
3V3       ←→  VCC (vermelho)
GND       ←→  GND (preto)
GPIO 8    ←→  SDA (azul)
GPIO 9    ←→  SCL (roxo)
```

### Flashear e Monitorar

```bash
# Passo 1: Identificar porta serial
# Windows: COM3, COM4, ...
# Linux: /dev/ttyUSB0, /dev/ttyACM0
# macOS: /dev/cu.usbmodem14201

# Passo 2: Flashear
idf.py -p COM3 flash

# Passo 3: Monitorar saída
idf.py -p COM3 monitor

# Parar monitoramento: Ctrl+]
```

**Saída esperada:**
```
--- Leitura dos Sensores ---
DHT22   | Temperatura: 24.8 C | Humidade: 55.2 %
MPU6050 | AcelX: 512 | AcelY: -128 | AcelZ: 16420

--- Leitura dos Sensores ---
DHT22   | Temperatura: 24.9 C | Humidade: 55.1 %
MPU6050 | AcelX: 508 | AcelY: -136 | AcelZ: 16418
```

### Troubleshooting Hardware

| Problema | Solução |
|----------|---------|
| Porta serial não aparece | Instalar driver CH340 (Windows) |
| Timeout ao flashear | Manter ESP32-S3 em BOOT pressionando |
| Valores DHT22 -1 (erro) | Verificar GPIO 15 e alimentação |
| Valores MPU6050 zeros | Verificar I2C (GPIO 8/9) |
| Sem saída serial | Verificar baud rate (115200) |

---

## ✅ Validação Rápida

### Checklist Funcional

```
COMPILAÇÃO:
☐ Comando `idf.py build` completa sem erros
☐ Arquivo build/sensor-dht11-esp32.elf existe
☐ Tamanho > 200 KB e < 1 MB

SIMULAÇÃO WOKWI:
☐ Extensão Wokwi instalada e token configurado
☐ Janela Wokwi abre com circuito renderizado
☐ Monitor serial mostra dados a cada 2s
☐ Valores DHT22 entre 15-40 °C (realístico)
☐ Valores MPU6050 mostram aceleração 3D

HARDWARE (se testado):
☐ Sensores fisicamente conectados
☐ Cabo USB-C conectado
☐ LED de potência ESP32-S3 aceso
☐ Monitor serial mostra dados contínuos
☐ Nenhuma mensagem de erro

QUALIDADE:
☐ Código compila sem warnings
☐ Monitor não trava (uptime > 1 min)
☐ Dados estáveis (sem valores aleatórios)
```

### Teste de Estabilidade (5 minutos)

```bash
# Deixar rodando e verificar:
idf.py monitor

# Esperado:
✓ Dados atualizando regularmente
✓ Valores consistentes
✓ Sem mensagens de erro
✓ Sem travamentos
```

---

## 🐛 Troubleshooting Express

### Problema: "Build failed"

```bash
# Solução 1: Limpar cache
idf.py clean
idf.py build

# Solução 2: Verificar CMakeLists.txt
cat main/CMakeLists.txt
# Deve conter: idf_component_register(SRCS "main.c" ...)

# Solução 3: Reinstalar IDF
idf.py install
idf.py build
```

### Problema: "Wokwi não abre"

```bash
# Solução 1: Gerar token
# https://wokwi.com/dashboard/ci → copiar token

# Solução 2: Configurar extensão
# VS Code → Settings → "Wokwi" → "Embed Token"

# Solução 3: Verificar wokwi.toml
cat wokwi.toml
# Deve conter:
# elf = "build/sensor-dht11-esp32.elf"
```

### Problema: "Monitor serial vazio"

```bash
# Solução 1: Verificar baud rate
idf.py monitor --baud 115200

# Solução 2: Aguardar inicialização
# Sensor DHT22 lê a cada 2 segundos
# Aguarde 5 segundos antes de concluir

# Solução 3: Verificar USB/COM
# Windows: Device Manager → Portas COM
# Linux: ls /dev/ttyUSB*
# macOS: ls /dev/cu.usb*
```

### Problema: "Valores DHT22 sempre -1"

```bash
# Causa: Sensor não respondendo
# Soluções:

# 1. Verificar alimentação
# DHT22 VCC deve estar em 3V3

# 2. Verificar GPIO 15
# Verificar diagram.json:
grep "\"15\"" diagram.json

# 3. Teste isolado
# Compilar apenas teste DHT22 (ver DEBUGGING_CHECKLIST.md)
```

### Problema: "MPU6050 valores zerados"

```bash
# Causa: I2C não inicializado ou sensor desligado
# Soluções:

# 1. Verificar I2C init
# Em main.c: i2c_init() está sendo chamado?

# 2. Verificar GPIO 8/9
grep "I2C_MASTER_SDA_IO\|I2C_MASTER_SCL_IO" main.c
# Esperado: 8 e 9

# 3. Scan I2C
# Implementar i2c_scan() (ver DEBUGGING_CHECKLIST.md)
```

---

## 🎯 Próximos Passos

### Aprender Mais

1. **Documentação Completa**
   - Ler: `README.md`
   - Detalhes técnicos de cada componente

2. **Debugging Avançado**
   - Consultar: `DEBUGGING_CHECKLIST.md`
   - Testes isolados, performance, logs

3. **Código**
   - Estudar: `main/main.c`
   - Compreender protocolos DHT22 e I2C

### Personalizações Comuns

#### 1. Mudar Taxa de Amostragem

```c
// Em main.c, linha ~120:
vTaskDelay(pdMS_TO_TICKS(2000));  // Mudar 2000 para outro valor
// Exemplos: 1000 (1s), 5000 (5s)
```

#### 2. Adicionar Saída Formatada

```c
// Adicionar em main.c:
printf("Timestamp: %lld ms\n", esp_timer_get_time() / 1000);
```

#### 3. Integrar WiFi (futuro)

```c
// Adicionar dependência:
// idf_component_register(PRIV_REQUIRES wifi_provisioning)
```

#### 4. Salvar em SD Card (futuro)

```c
// Incluir: #include "esp_vfs_fat.h"
// Implementar write_to_sdcard()
```

---

## 📚 Referências Rápidas

| Recurso | Link |
|---------|------|
| ESP-IDF Docs | https://docs.espressif.com |
| DHT22 Datasheet | https://cdn-shop.adafruit.com/datasheets/DHT22.pdf |
| MPU6050 Docs | https://invensense.tdk.com/products/motion-tracking/6-axis/ |
| Wokwi Docs | https://docs.wokwi.com |
| FreeRTOS API | https://www.freertos.org/API-reference.html |

---

## ⏱️ Timeline Esperada

| Atividade | Tempo |
|-----------|-------|
| Instalar ESP-IDF | 15-20 min |
| Clonar projeto | 2 min |
| Compilar | 30-60 seg (primeira) |
| Configurar Wokwi | 5 min |
| Simular | 3 min |
| Ver dados | 1 min |
| **TOTAL** | **~30 min** |

---

## 🎉 Sucesso!

Se você viu dados no monitor serial:

```
✓ DHT22   | Temperatura: XX.X C | Humidade: XX.X %
✓ MPU6050 | AcelX: XXXX | AcelY: XXXX | AcelZ: XXXX
```

**Parabéns!** Seu projeto está funcionando! 🚀

---

## 📞 Suporte

Se algo der errado:

1. **Verificar**: `DEBUGGING_CHECKLIST.md`
2. **Ler**: Seção "Troubleshooting Express" acima
3. **Pesquisar**: [ESP-IDF Issues](https://github.com/espressif/esp-idf/issues)
4. **Consultar**: [Fórum ESP32](https://www.esp32.com/)

---

**Última atualização:** 24 de setembro de 2026  
**Versão:** 1.0  
**Tempo de leitura:** ~10 minutos
