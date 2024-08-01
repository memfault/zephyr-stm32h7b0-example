# Memfault Zephyr + STM32H7B3I_DK Example

This sample app showcases Memfault functionality on the `stm32h7b3i_dk` board.

## Getting Started

The Memfault Zephyr integration guide is an excellent reference, and documents
how the Memfault SDK was added to the base example app:

https://docs.memfault.com/docs/mcu/zephyr-guide

To try out this example app:

1. Set up the Zephyr prerequisites: https://docs.zephyrproject.org/latest/develop/getting_started/index.html
2. Create a zephyr workspace and set it up with this project:

   ```bash
   mkdir zephyr-workspace
   cd zephyr-workspace
   west init -m https://github.com/memfault/stm32h7b3i_dk
   west update
   ```

3. Build the example app:

   ```bash
   west build stm32h7b3i_dk --sysbuild
   ```

   `--sysbuild` will build the MCUboot image as well, which will also get
   flashed in the following step. To run sysbuild by default, you can configure
   your workspace with `west config --local build.sysbuild True`.

4. Flash the example app:

   ```bash
    west flash
   ```

5. Open a serial console and interact with the sample app shell. For example
   using PySerial:

   ```bash
   # select the correct serial port for your system
   pyserial-miniterm --raw /dev/serial/by-id/TODO 115200

   uart:~$ mflt get_device_info
   [00:09:21.911,000] <inf> mflt: S/N: DEMOSERIAL
   [00:09:21.911,000] <inf> mflt: SW type: zephyr-app
   [00:09:21.911,000] <inf> mflt: SW version: 0.0.1727a85c
   [00:09:21.911,000] <inf> mflt: HW version: esp32c3_devkitm
   ```
