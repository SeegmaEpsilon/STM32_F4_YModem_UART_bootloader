# STM32 Modular Bootloader

**Русская версия ниже**

## Overview

This repository contains a **modular and extensible bootloader for STM32F4 microcontrollers** with support for encrypted firmware updates and pluggable transport layers.  
The project is designed as a foundation for secure in-field firmware upgrades, focusing on clean architecture, portability, and ease of extension.

The bootloader is written in **C using STM32 HAL**, and currently supports **UART + YMODEM** as a transport, with the architecture prepared for future transports (e.g. ZMODEM, USB, CAN, Ethernet).

## Key Features

- Modular bootloader architecture (transport / crypto / flash separated)
- Firmware update via UART using YMODEM
- AES-based firmware decryption on-the-fly
- No plaintext firmware stored in Flash
- Clean jump to user application
- Flash erase/write abstraction
- Configurable memory layout
- HAL-based, STM32Cube-compatible

## Supported MCUs

- STM32F4 family  
  Tested on:
  - STM32F401
  - STM32F405
  - STM32F407

## Project Structure

```
.
├── crypto.c / crypto.h        # High-level crypto interface (CTR stream)
├── aes.c / aes.h              # AES implementation
├── key_template.h             # Encryption key template
├── flash.c / flash.h          # Flash erase/write abstraction
├── ymodem.c / ymodem.h        # YMODEM protocol implementation
├── uart_transport.c / .h      # UART transport layer
├── cpu.c / cpu.h              # CPU-specific helpers (jump, reset, etc.)
├── interface.h                # Bootloader public interfaces
├── defines.h                  # Memory map and configuration
└── README.md
```

## Memory Map (default)

| Region       | Address        |
|--------------|----------------|
| Bootloader   | 0x08000000     |
| Application  | 0x08010000     |

The application start address can be changed in `defines.h`.

## Security Model

- Firmware is encrypted **off-device**
- Bootloader performs **AES-CTR decryption during reception**
- Decrypted data is written directly to Flash
- No decrypted image is stored in RAM or Flash
- Encryption key is compiled into the bootloader (see `key_template.h`)

> ⚠️ Key protection relies on MCU readout protection (RDP).  
> For higher security, integration with HW crypto or secure elements is recommended.

## Firmware Update Flow

1. MCU boots into bootloader
2. Bootloader waits for command over transport
3. Host sends encrypted firmware via YMODEM
4. Bootloader:
   - receives data blocks
   - decrypts on-the-fly
   - writes to Flash
5. Integrity checks pass
6. Bootloader jumps to user application

## UART Settings (default)

- Interface: USART
- Baudrate: 115200
- Format: 8-N-1
- Flow control: none

## How to Use

1. Build and flash the bootloader to the MCU
2. Prepare encrypted firmware image on PC
3. Connect UART to host PC
4. Open terminal (e.g. Tera Term)
5. Start YMODEM transfer
6. Bootloader writes and decrypts firmware automatically
7. MCU jumps to application

## Extending the Bootloader

The architecture is intentionally modular.

You can:
- Replace YMODEM with another protocol
- Add new transport layers (USB, CAN, Ethernet)
- Swap crypto backend
- Add authentication / signature verification
- Add rollback protection

Key extension points:
- `uart_transport.*`
- `crypto.*`
- `interface.h`

## Build Environment

- STM32CubeIDE / STM32CubeMX
- ARM GCC
- C99

## Related Projects

- UART YMODEM updater (host-side):
  https://github.com/SeegmaEpsilon/UART_YModem_Updater

## License

MIT License

## Disclaimer

This project is intended for educational and research purposes.  
It does **not** claim to be a complete secure boot solution.

# Модульный загрузчик для STM32

## Обзор

Данный репозиторий содержит **модульный и расширяемый загрузчик для микроконтроллеров STM32F4** с поддержкой зашифрованного обновления прошивки и подключаемых транспортных уровней.  
Проект задуман как основа для безопасного обновления прошивки «в поле» (in-field update) с упором на чистую архитектуру, переносимость и удобство расширения.

Загрузчик написан на языке **C с использованием STM32 HAL**. В текущей реализации поддерживается транспорт **UART + YMODEM**, при этом архитектура заранее подготовлена для добавления других протоколов и интерфейсов (например, ZMODEM, USB, CAN, Ethernet).

## Основные возможности

- Модульная архитектура загрузчика (разделение transport / crypto / flash)
- Обновление прошивки по UART с использованием YMODEM
- Дешифрование прошивки на лету с использованием AES
- Отсутствие хранения незашифрованной прошивки во Flash
- Корректный переход к пользовательскому приложению
- Абстракция операций стирания и записи Flash-памяти
- Настраиваемая карта памяти
- Совместимость со STM32Cube и HAL

## Поддерживаемые микроконтроллеры

- Семейство STM32F4  
  Протестировано на:
  - STM32F401
  - STM32F405
  - STM32F407

## Структура проекта

```
.
├── crypto.c / crypto.h        # Высокоуровневый криптографический интерфейс (CTR-поток)
├── aes.c / aes.h              # Реализация AES
├── key_template.h             # Шаблон ключа шифрования
├── flash.c / flash.h          # Абстракция работы с Flash-памятью
├── ymodem.c / ymodem.h        # Реализация протокола YMODEM
├── uart_transport.c / .h      # UART-транспорт
├── cpu.c / cpu.h              # CPU-зависимые функции (jump, reset и т.п.)
├── interface.h                # Публичные интерфейсы загрузчика
├── defines.h                  # Карта памяти и конфигурация
└── README.md
```

## Карта памяти (по умолчанию)

| Область        | Адрес          |
|----------------|----------------|
| Загрузчик      | 0x08000000     |
| Приложение     | 0x08010000     |

Адрес начала пользовательского приложения может быть изменён в файле `defines.h`.

## Модель безопасности

- Прошивка шифруется **вне устройства**
- Загрузчик выполняет **AES-CTR-дешифрование во время приёма данных**
- Расшифрованные данные сразу записываются во Flash
- Незашифрованный образ прошивки не хранится ни в RAM, ни во Flash
- Ключ шифрования вшит в загрузчик (см. `key_template.h`)

> ⚠️ Защита ключа основана на механизме Readout Protection (RDP) микроконтроллера.  
> Для более высокого уровня безопасности рекомендуется интеграция аппаратной криптографии или защищённых элементов.

## Процесс обновления прошивки

1. Микроконтроллер запускается в режиме загрузчика
2. Загрузчик ожидает команду по выбранному транспортному интерфейсу
3. Хост передаёт зашифрованную прошивку по YMODEM
4. Загрузчик:
   - принимает блоки данных
   - выполняет дешифрование на лету
   - записывает данные во Flash
5. Выполняются проверки целостности
6. Загрузчик передаёт управление пользовательскому приложению

## Настройки UART (по умолчанию)

- Интерфейс: USART
- Скорость: 115200 бод
- Формат: 8-N-1
- Управление потоком: отсутствует

## Использование

1. Соберите и прошейте загрузчик в микроконтроллер
2. Подготовьте зашифрованный файл прошивки на ПК
3. Подключите UART к компьютеру
4. Откройте терминальную программу (например, Tera Term)
5. Запустите передачу файла по YMODEM
6. Загрузчик автоматически выполнит дешифрование и запись прошивки
7. Микроконтроллер перейдёт к пользовательскому приложению

## Расширение загрузчика

Архитектура проекта изначально является модульной.

Возможные направления расширения:
- Замена YMODEM на другой протокол
- Добавление новых транспортов (USB, CAN, Ethernet)
- Замена или расширение криптографического backend
- Добавление аутентификации и проверки цифровой подписи
- Реализация защиты от отката версии прошивки

Ключевые точки расширения:
- `uart_transport.*`
- `crypto.*`
- `interface.h`

## Среда сборки

- STM32CubeIDE / STM32CubeMX
- ARM GCC
- C99

## Связанные проекты

- Хост-утилита для загрузки по UART YMODEM:  
  https://github.com/SeegmaEpsilon/UART_YModem_Updater

## Лицензия

MIT License

## Отказ от ответственности

Проект предназначен для учебных и исследовательских целей.  
Он **не является** полноценной реализацией защищённой загрузки промышленного уровня.
