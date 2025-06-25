# STM32 F4 UART Bootloader with YModem Protocol

# (ENGLISH):

This project provides a simple and reliable bootloader implementation for STM32F4 microcontrollers using UART communication and the YModem protocol. It enables firmware updates via a serial terminal application such as Tera Term.
See also: [UART_YModem_Updater](https://github.com/SeegmaEpsilon/UART_YModem_Updater).


## Features

- UART-based firmware download using YModem
- Flash memory erase and write support
- Automatic jump to user application on timeout
- Simple text menu interface over UART
- Written using STM32 HAL library

## Requirements

- STM32F4 MCU (tested with STM32F407 and STM32F405)
- Tera Term, HyperTerminal or similar terminal with YModem support
- A `.bin` firmware file
- UART1 connected to a host PC

## Memory Map

- Bootloader: placed at 0x08000000
- Application: placed at 0x08010000 (see `defines.h`)

## How to Use

1. **Connect UART1 to your PC** at 115200 baud (default). Configure 8-N-1, no flow control.

2. **Reset or power on the device.** The bootloader will wait for input:

   ```
   =========================
   =     UART BOOTLOADER   =
   =     VERSION: 1.0.1    =
   =========================
   Press '1' to download image to the Internal Flash...
   ```

3. **Press `1`** in the terminal. The device will start YModem file transfer mode.

4. **Send firmware via YModem.**
   - In Tera Term: go to `File -> Transfer -> YMODEM -> Send`, select your `.bin` file.

5. **Wait until the upload completes.**
   - After upload, the device will automatically jump to the user application.

6. **To erase flash manually**, reset the device and send `#` instead of `1`.

## Notes

- The bootloader waits ~2 seconds (`timeout_us = 2000000`) for input before jumping to the main application.
- If no valid application is found (`0xFFFFFFFF` at 0x08010000), bootloader stays active indefinitely.

# (РУССКИЙ):

Этот проект представляет собой простую и надёжную реализацию загрузчика (bootloader) для микроконтроллеров STM32F4, использующую UART и протокол YModem. Он позволяет обновлять прошивку через терминальную программу, такую как Tera Term.
См. также: [UART_YModem_Updater](https://github.com/SeegmaEpsilon/UART_YModem_Updater).

## Возможности

- Загрузка прошивки через UART с использованием YModem
- Поддержка стирания и записи во Flash-память
- Автоматический переход к пользовательскому приложению по таймауту
- Простой текстовый интерфейс по UART
- Используется STM32 HAL-библиотека

## Требования

- МК STM32F4 (протестировано на STM32F407 и STM32F405)
- Tera Term, HyperTerminal или другой терминал с поддержкой YModem
- Файл прошивки `.bin`
- UART1, подключённый к ПК

## Карта памяти

- Загрузчик: размещён по адресу 0x08000000
- Приложение: размещено по адресу 0x08010000 (см. `defines.h`)

## Инструкция по использованию

1. **Подключите UART1 к ПК** со скоростью 115200 бод (по умолчанию). Конфигурация: 8 бит, без чётности, 1 стоп-бит, без управления потоком.

2. **Сбросьте или включите устройство.** Загрузчик ожидает ввод:

   ```
   =========================
   =     UART BOOTLOADER   =
   =     VERSION: 1.0.1    =
   =========================
   Press '1' to download image to the Internal Flash...
   ```

3. **Нажмите `1`** в терминале. Устройство войдёт в режим приёма по YModem.

4. **Отправьте прошивку по YModem.**
   - В Tera Term: выберите `File -> Transfer -> YMODEM -> Send`, укажите ваш `.bin` файл.

5. **Дождитесь завершения передачи.**
   - После загрузки устройство автоматически перейдёт к пользовательскому приложению.

6. **Чтобы вручную стереть Flash**, при сбросе устройства отправьте `#` вместо `1`.

## Примечания

- Загрузчик ожидает ввод около 2 секунд (`timeout_us = 2000000`), после чего переходит к основному приложению.
- Если по адресу 0x08010000 находится `0xFFFFFFFF`, загрузчик не находит приложение и остаётся активным.