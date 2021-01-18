# nRF8001-sys

Rust bindings for a port of Nordic Semiconductor's [nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino).
All low-level IO and delay routines are left as external dependencies so that they're not tied to
any particular architecture.

## Status

The SDK functions but the IO interface could use some rethinking and cleanup.
I generally erred on the side of leaving the SDK as intact as possible for the sake of getting it
working with minimal risk.

The SDK is configured to run without the use of interrupts.

## Usage

* Initialize GPIO and SPI yourself
* Provide implementations for functions in [io\_support.h](libraries/BLE/io_support.h).
* Follow Nordic's documentation to use the SDK. Nordic provides many examples in C in the [examples](libraries/BLE/examples)
folder. The ACI library is well-documented [here](https://www.nordicsemi.com/-/media/DocLib/Other/Product_Spec/nRF8001PSv13.pdf).

See [nrf8001-sandbox](https://github.com/kesyog/nrf8001-sandbox) for a Rust example.

## Why?

Just mashing old dev boards I had laying around together and seeing what happens. The nRF8001 is
used on the [Adafruit Bluefruit LE](https://www.adafruit.com/product/1697), though it is not
recommended for new designs by Nordic.
 
The Rust-C interface is ugly in that it requires a lot of unsafe calls. This can probably be cleaned
up with more thinking and better abstractions so that applications can be built on top of it in safe
Rust.

## References

* [Nordic's nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino). Porting
instructions are provided in their documentation.
* [nRF8001 product specification](https://www.nordicsemi.com/-/media/DocLib/Other/Product_Spec/nRF8001PSv13.pdf)
* [shraken/nrf8001-stm32f4](https://github.com/shraken/nrf8001-stm32f4): another port of the nRF8001
SDK to the STM32F4 that uses the STM32 peripheral library extensively. This served as a useful
reference and starting point.
* [Making a \*-sys crate](https://kornel.ski/rust-sys-crate)
