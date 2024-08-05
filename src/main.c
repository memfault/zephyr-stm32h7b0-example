//! @file

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app_version.h"
#include "memfault/components.h"

LOG_MODULE_REGISTER(mflt_app, LOG_LEVEL_DBG);

int main(void) {
  LOG_INF("Memfault Demo App! Board %s\n", CONFIG_BOARD);

  printk("\n" MEMFAULT_BANNER_COLORIZED);
  printk("\nApplication git describe: " STRINGIFY(APP_BUILD_VERSION) "\n");

  // 250ms delay seems to make the logs in the *info_dump() commands work
  // correctly
  k_sleep(K_MSEC(250));

  memfault_device_info_dump();
  memfault_build_info_dump();

  return 0;
}
