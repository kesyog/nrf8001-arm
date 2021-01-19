# nRF8001-sys

A fork of Nordic Semiconductor's [nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino)
ported for ARM with optional Rust bindings generated via [bindgen](https://github.com/rust-lang/rust-bindgen).

All low-level IO and delay routines are left as external dependencies so that they're not directly
tied to any particular architecture.

## Status

The SDK works as expected though the IO interface could use some minor cleanup. It's configured to
run without the use of interrupts.

I generally erred on the side of leaving the SDK and associated API's as intact as possible for the
sake of getting it working and maintaining backwards compatibility, at least for now.

Although there's nothing ARM-specific about the codebase, it is currently configured to only run on
ARM. Fixing this should be pretty easy:

* Don't hard-code the ARM gcc compiler and archiver in the build.rs script.
* Rip out the old AVR-specific code that's currently #define'd out when the `__arm__` flag is
available and then get rid of that build flag.

## Usage

* Initialize GPIO and SPI yourself
* Provide implementations for functions in [io\_support.h](libraries/BLE/io_support.h).
* Follow Nordic's documentation to use the SDK. The [reference manual](https://www.nordicsemi.com/-/media/DocLib/Other/Product_Spec/nRF8001PSv13.pdf)
is generally great, and Nordic provides many examples in C in the [examples](libraries/BLE/examples)
folder. 

See [nrf8001-sandbox](https://github.com/kesyog/nrf8001-sandbox) for some example usage in Rust.

## Why?

* Mashing old dev boards I had laying around together. The nRF8001 is used on the [Adafruit Bluefruit LE](https://www.adafruit.com/product/1697).
* Figuring out how to use bindgen to generate C bindings for Rust, and seeing what I can prototype
and build in Rust on top of this.
* Learning a little bit about BLE along the way.

A few downers I learned along the way:

* The nRF8001 is not recommended for new designs by Nordic.
* Defining BLE characteristics beyond the examples in the Nordic SDK requires using the Windows-only
[nRFgo Studio](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRFgo-Studio) to
generate opaque setup messages.
* Building a Rust-C interface is a bit ugly in that it requires a lot of unsafe calls. There's no
real way around this other than to build a wrapper that hides away all the unsafe calls.

## References

* [Nordic's nRF8001 Arduino SDK](https://github.com/NordicSemiconductor/ble-sdk-arduino). Porting
instructions are provided in their documentation.
* [nRF8001 product specification](https://www.nordicsemi.com/-/media/DocLib/Other/Product_Spec/nRF8001PSv13.pdf)
* [shraken/nrf8001-stm32f4](https://github.com/shraken/nrf8001-stm32f4): another port of the nRF8001
SDK to the STM32F4 that uses the STM32 peripheral library extensively. This served as a useful
reference and starting point.
* [Making a \*-sys crate](https://kornel.ski/rust-sys-crate)
