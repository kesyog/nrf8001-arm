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

#define INPUT         0x00
#define OUTPUT        0x01
#define INPUT_PULLUP  0x02

void delay(uint16_t delay);
bool digitalRead(uint8_t pin);
void digitalWrite(eGpioPin pin, bool value);
void pinMode(eGpioPin pin, uint8_t mode);
bool transmit_SPI_byte(uint8_t txbuf, uint8_t *rxbuf);

#endif