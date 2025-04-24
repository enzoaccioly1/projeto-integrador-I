# 📃 | Arduino IDE (configuração)

## 0. Instalar a Arduino IDE
https://www.arduino.cc/en/software/

## 1. Adicionar a 'Board' do ESP32
Cole ```https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json``` em File > Preferences > Additional boards manager URLs

## 2. Instalar a 'Board' do ESP32
Pesquise por "**esp32**" e instale em Tools > Board > Board Manager

## 3. Selecionar a 'Board'
Tools > Board > esp32 > "ESP32 Dev Module"

## 4. Selecionar o 'Partition Scheme'
Tools > Partition Scheme > RainMaker 4MB No OTA

## 5. Selecionar a 'Port'
Selecione a 'port' por onde o ESP32 está conectado em Tools > Port 

## 6. Possíveis erros
Certifique-se de que o Windows ou seu sistema operacional reconhece a porta conectada do ESP32 (mais informações em https://www.reddit.com/r/esp32/comments/11pmedy/issue_uploading_to_esp32/)
