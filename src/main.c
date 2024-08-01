//! @file

#include <string.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app_version.h"
#include "memfault/components.h"
#include "memfault/ports/zephyr/http.h"

LOG_MODULE_REGISTER(mflt_app, LOG_LEVEL_DBG);

const char *memfault_zephyr_get_device_id(void) {
  uint8_t dev_id[16] = {0};
  static char dev_id_str[sizeof(dev_id) * 2 + 1];

  // Check if the device id has already been initialized
  if (dev_id_str[0]) {
    return dev_id_str;
  }

  // Obtain the device id
  ssize_t length = hwinfo_get_device_id(dev_id, sizeof(dev_id));

  // Render the obtained serial number in hexadecimal representation
  for (size_t i = 0; i < length; i++) {
    (void)snprintf(&dev_id_str[i * 2], sizeof(dev_id_str), "%02x", dev_id[i]);
  }

  return dev_id_str;
}

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {
  *info = (sMemfaultDeviceInfo){
    .device_serial = memfault_zephyr_get_device_id(),
    .software_type = "app",
    .software_version = APP_VERSION_STRING "+" ZEPHYR_MEMFAULT_EXAMPLE_GIT_SHA1,
    .hardware_version = CONFIG_BOARD,
  };
}

int main(void) {
  LOG_INF("Memfault Demo App! Board %s\n", CONFIG_BOARD);

  printk("\n" MEMFAULT_BANNER_COLORIZED);

  memfault_device_info_dump();

  return 0;
}
