# nRF8001-sys

A Rust sys crate containing a WIP ARM port of Nordic Semiconductor's [nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino).
All low-level IO and delay routines are left as external dependencies so that they are not tied to
any one vendor's peripheral library.

# Usage

TODO: expand

* Initialize GPIO
* Implement functions in io\_support.h

# References

* [Nordic's nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino). Porting
instructions are provided in their documentation.
* [shraken/nrf8001-stm32f4](https://github.com/shraken/nrf8001-stm32f4): another port of the nRF8001
SDK to the STM32F4 that uses the STM32 peripheral library extensively. This was an invaluable
resource and served as a starting point.
* [Making a \*-sys crate](https://kornel.ski/rust-sys-crate)
