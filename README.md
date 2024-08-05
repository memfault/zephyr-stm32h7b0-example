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
   west init -m https://github.com/memfault/zephyr-stm32h7b0-example
   west update
   ```

3. Build the example app:

   ```bash
   west build --sysbuild --board stm32h7b3i_dk --pristine=always zephyr-stm32h7b0-example
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
   pyserial-miniterm --raw /dev/serial/by-id/usb-STMicroelectronics_STLINK-V3_002000353331510933323639-if02 115200

   uart:~$ mflt get_device_info
   [00:00:03.449,000] <inf> mflt: S/N: 373535343430510d00410045
   [00:00:03.455,000] <inf> mflt: SW type: app
   [00:00:03.460,000] <inf> mflt: SW version: 0.0.1+1e097ff
   [00:00:03.465,000] <inf> mflt: HW version: stm32h7b3i_dk
   ```

## Note about MCUboot + Sysbuild

This example application uses [Zephyr's
Sysbuild](https://docs.zephyrproject.org/latest/build/sysbuild/index.html)
feature to build both the bootloader and application together.

To enable MCUboot, this guide was followed:

https://docs.mcuboot.com/readme-zephyr

The following steps were taken:

1. add a `sysbuild.conf` file with `SB_CONFIG_BOOTLOADER_MCUBOOT=y` to enable MCUboot
2. a DT overlay file, `boards/stm32h7b3i_dk.overlay`, to specify a custom partition layout:

   - boot_partition: MCUboot goes here
   - storage_partition: unused, could be used for a filesystem or NVS
   - slot0_partition: primary slot for the application
   - slot1_partition: secondary slot for the application
   - scratch_partition: used by MCUboot for swap operations

3. add the `sysbuild/mcuboot` directory with the following files:

   1. `prj.conf`: generic options for the MCUboot child image
   2. `sysbuild/mcuboot/boards/stm32h7b3i_dk.overlay`: MCUboot-specific overlay,
      used to select the correct partition for MCUboot. This overlay includes
      the above overlay, so the partition layout is defined in one place.

      ```devicetree
      zephyr,code-partition = &boot_partition;
      ```
