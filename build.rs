use std::env;
use std::path::{Path, PathBuf};

const BLE_LIBRARY_PATH: &str = "libraries/BLE";

fn main() {
    cc::Build::new()
        .include("src")
        .include(Path::new(BLE_LIBRARY_PATH))
        .file("src/ble_assert.c")
        .file("libraries/BLE/acilib.c")
        .file("libraries/BLE/aci_queue.c")
        .file("libraries/BLE/aci_setup.c")
        .file("libraries/BLE/hal_aci_tl.c")
        .file("libraries/BLE/lib_aci.c")
        .opt_level_str("s")
        .no_default_flags(true)
        .flag("-mcpu=cortex-m4")
        .flag("-mthumb")
        .flag("-mlittle-endian")
        .flag("-mfpu=fpv4-sp-d16")
        .flag("-mfloat-abi=hard")
        .flag("-mthumb-interwork")
        .flag("-std=c99")
        .flag("-fPIE")
        .pic(true)
        .warnings(false)
        .extra_warnings(false)
        // TODO: Don't hard-code the compiler so that tests work
        .compiler(Path::new("arm-none-eabi-gcc"))
        .archiver(Path::new("arm-none-eabi-ar"))
        .compile("libnrf8001.a");

    // Tell cargo to invalidate the built crate whenever the wrapper changes
    println!("cargo:rerun-if-changed=bindings.h");

    // https://rust-lang.github.io/rust-bindgen/introduction.html
    let bindings = bindgen::Builder::default()
        .header("bindings.h")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Don't use std types
        .use_core()
        .ctypes_prefix("cty")
        .clang_arg("-D__arm__")
        // Add include paths
        // maybe: single source of truth for include paths between cc and bindgen
        .clang_arg("-Isrc")
        .clang_arg(["-I", BLE_LIBRARY_PATH].join(""))
        // Whitelist functions
        .whitelist_function("do_aci_setup")
        .whitelist_function("lib_aci.*")
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
