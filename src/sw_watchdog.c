//! @file

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app_version.h"
#include "memfault/components.h"

LOG_MODULE_REGISTER(swwdg, LOG_LEVEL_DBG);

#include <zephyr/drivers/counter.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "memfault/components.h"

#define TIMER_DEV DT_INST(0, st_stm32_counter)
#define WATCHDOG_TIMEOUT_MS 10000

static const struct device *s_sw_watchdog_timer;
struct counter_top_cfg s_sw_watchdog_top_cfg;

static void prv_sw_watchdog_callback(const struct device *dev, void *user_data) {
  ARG_UNUSED(dev);
  ARG_UNUSED(user_data);

  LOG_ERR("Software watchdog expired!");
  MEMFAULT_SOFTWARE_WATCHDOG();
}

void sw_watchdog_feed(void) {
  // To reset the watchdog, we simply reconfigure the top value.
  int err = counter_set_top_value(s_sw_watchdog_timer, &s_sw_watchdog_top_cfg);
  if (err) {
    LOG_ERR("Failed to feed watchdog: %d", err);
  }
}

int sw_watchdog_init(void) {
  int err;
  s_sw_watchdog_timer = DEVICE_DT_GET(TIMER_DEV);
  if (!device_is_ready(s_sw_watchdog_timer)) {
    LOG_ERR("Timer device not ready");
    return -ENODEV;
  }

  s_sw_watchdog_top_cfg = (struct counter_top_cfg){
    .callback = prv_sw_watchdog_callback,
    .user_data = NULL,
    .ticks = counter_us_to_ticks(s_sw_watchdog_timer, WATCHDOG_TIMEOUT_MS * 1000),
    .flags = 0,
  };

  err = counter_start(s_sw_watchdog_timer);
  if (err) {
    LOG_ERR("Failed to start counter: %d", err);
    return err;
  }

  err = counter_set_top_value(s_sw_watchdog_timer, &s_sw_watchdog_top_cfg);
  if (err) {
    LOG_ERR("Failed to set top value: %d", err);
    return err;
  }

  return 0;
}
