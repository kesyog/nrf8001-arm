#ifndef IO_SUPPORT_H
#define IO_SUPPORT_H

#include <stdint.h>
#include <stdbool.h>

#define LOW           false
#define HIGH          true

typedef enum {
  kpin_mode_input,
  kpin_mode_output,
  kpin_mode_input_pullup,
} eGpioPinMode;

void delay(uint16_t delay);
// pin numbers match what is passed into aci_pins_t
bool digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, bool value);
// Doesn't do anything. Probably should delete.
void pinMode(uint8_t pin, eGpioPinMode mode);
bool transmit_SPI_byte(uint8_t txbuf, uint8_t *rxbuf);

#endif