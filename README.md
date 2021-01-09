# nRF8001 SDK for ARM

A WIP ARM port of Nordic Semiconductor's [nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino).
The immediate goal is to build a standalone static library that isn't tied to any particular
microcontroller vendor's peripheral library.

# References

* [Nordic's nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino). Porting
instructions are provided in their documentation.
* [shraken/nrf8001-stm32f4](https://github.com/shraken/nrf8001-stm32f4): another port of the nRF8001
SDK to the STM32F4 that uses the STM32 peripheral library extensively. This was an invaluable
resource and served as a starting point.
