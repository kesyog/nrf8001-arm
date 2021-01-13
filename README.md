# nRF8001-sys

Rust bindings for a WIP ARM port of Nordic Semiconductor's [nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino).
All low-level IO and delay routines are left as external dependencies so that they're not tied to
any particular architecture.

## Status

The SDK functions but the IO interface could use some rethinking and cleanup.
I generally erred on the side of leaving the SDK as intact as possible for the sake of getting it
working with minimal risk.

## Usage

* Initialize GPIO and SPI yourself
* Provide implementations for functions in [io\_support.h](libraries/BLE/io_support.h).
* Follow Nordic's documentation to use the SDK. Nordic provides many examples in C in the [examples](libraries/BLE/examples)
folder. Note that everything is driven by polling. TBD if it's better if this part in C vs. Rust.

See [nrf8001-sandbox](https://github.com/kesyog/nrf8001-sandbox) for a Rust example.

## Why?

Just mashing old dev boards I had laying around together and seeing what happens. The nRF8001 is
used on the [Adafruit Bluefruit LE](https://www.adafruit.com/product/1697).

There are at least a few reasons why this might not make much sense:

* The nRF8001 chip is not recommended for new designs by Nordic.
* The Rust-C interface was surprisingly ugly in that it requires a lot of unsafe calls. This could
probably be cleaned up with more thinking and better abstractions.

## References

* [Nordic's nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino). Porting
instructions are provided in their documentation.
* [shraken/nrf8001-stm32f4](https://github.com/shraken/nrf8001-stm32f4): another port of the nRF8001
SDK to the STM32F4 that uses the STM32 peripheral library extensively. This served as a useful
reference and starting point.
* [Making a \*-sys crate](https://kornel.ski/rust-sys-crate)
