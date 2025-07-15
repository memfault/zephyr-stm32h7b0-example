//! @file

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app_version.h"
#include "memfault/components.h"

LOG_MODULE_REGISTER(mflt_app, LOG_LEVEL_DBG);

#include <stdint.h>
#include <zephyr/drivers/watchdog.h>
#define CONFIG_WINDOW_WATCHDOG 1

#if defined(CONFIG_WINDOW_WATCHDOG)
static const struct device *window_watchdog_dev;
static int window_watchdog_id;
#endif

static void window_watchdog_callback(const struct device *dev, int channel_id) {
  ARG_UNUSED(dev);
  ARG_UNUSED(channel_id);
  // If we can catch this in the ISR, then try to disable the watchdogs and reset more gracefully
#if defined(CONFIG_WINDOW_WATCHDOG)
  // watchdog_helper_panic();
#endif

#if defined(CONFIG_MEMFAULT)
  MEMFAULT_TASK_WATCHDOG();
#endif
}

void window_watchdog_start(void) {
#if defined(CONFIG_WINDOW_WATCHDOG)

  window_watchdog_dev = DEVICE_DT_GET(DT_ALIAS(watchdog0));
  struct wdt_timeout_cfg wdt_config = {
    /* Reset SoC when watchdog timer expires. */
    .flags = WDT_FLAG_RESET_SOC | WDT_OPT_PAUSE_HALTED_BY_DBG,

    /* Expire watchdog after max window */
    .window.min = 3,  // CONFIG_WINDOW_WATCHDOG_MIN_WINDOW,
    .window.max = 7,  // CONFIG_WINDOW_WATCHDOG_MAX_WINDOW,

    .callback = window_watchdog_callback,
  };
  window_watchdog_id = wdt_install_timeout(window_watchdog_dev, &wdt_config);
  if (window_watchdog_id >= 0) {
    wdt_setup(window_watchdog_dev, WDT_FLAG_RESET_SOC | WDT_OPT_PAUSE_HALTED_BY_DBG);
  }

#endif
}

// #include <stm32h7b0xxq.h>
#include <stm32h7b3xxq.h>

#define WWDG1_COUNTER_VALUE (WWDG1->CR & WWDG_CR_T)

void window_watchdog_force_feed(void) {
#if defined(CONFIG_WINDOW_WATCHDOG)
  // TODO- should we check NCDB.trigger_wwd here? I assume that's used for
  // testing the wwd, in which case we want to force feed regardless, since
  // this function is used for a last gasp state saving only.
  // if (NCDB.trigger_wwd) {
  //     return;
  // }

  // Check if the watchdog is enabled
  if (!(WWDG1->CR & WWDG_CR_WDGA)) {
    return;
  }

  // Check if the window has already expired (counter <= 0x3F)
  // If counter is 0x3F or below, the watchdog has already triggered a reset condition
  if (WWDG1_COUNTER_VALUE <= 0x3F) {
    // Window has expired, do nothing
    return;
  }

  // Read the window value from WWDG_CFR register (bits 6:0)
  uint32_t window = WWDG1->CFR & WWDG_CFR_W;

  // busy loop using k_busy_wait until the window is open
  while (WWDG1_COUNTER_VALUE > window) {
    k_busy_wait(1);  // Wait until the watchdog is ready
  }

  // Window is open, safe to feed the watchdog
  wdt_feed(window_watchdog_dev, window_watchdog_id);
#endif
}

bool memfault_platform_coredump_save_begin(void) {
  // This function is called before saving a coredump, we can use it to
  // force feed the watchdog to prevent a reset during the save process.
  window_watchdog_force_feed();

  // Return true to indicate that we are ready to save the coredump
  return true;
}

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
