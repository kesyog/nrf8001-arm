#ifndef IO_SUPPORT_H
#define IO_SUPPORT_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  kgpio_mosi,
  kgpio_miso,
  kgpio_sclk,
  kgpio_reset,
  kgpio_rdyn,
  kgpio_reqn,
} eGpioPin;

#define LOW           false
#define HIGH          true

typedef enum {
  kpin_mode_input,
  kpin_mode_output,
  kpin_mode_input_pullup,
} eGpioPinMode;

void delay(uint16_t delay);
bool digitalRead(uint8_t pin);
void digitalWrite(eGpioPin pin, bool value);
void pinMode(eGpioPin pin, eGpioPinMode mode);
bool transmit_SPI_byte(uint8_t txbuf, uint8_t *rxbuf);

#endif