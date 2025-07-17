//! @file

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app_version.h"
#include "memfault/components.h"
#include "sw_watchdog.h"

// ARM Cortex-M registers for accessing NVIC and vector table
#define SCB_VTOR_ADDR (0xE000ED08U)
#define NVIC_IPR_BASE (0xE000E400U)
#define NVIC_ISER_BASE (0xE000E100U)

LOG_MODULE_REGISTER(mflt_app, LOG_LEVEL_DBG);

// Set up a timer that expires in 1 second.
static void prv_stuck_timer_callback(struct k_timer *dummy) {
  while (1) {
    k_busy_wait(1000 * USEC_PER_MSEC);
  }
}

K_TIMER_DEFINE(s_stuck_timer, prv_stuck_timer_callback, NULL);

static void prv_stuck_timer(void) {
  static uint32_t s_software_watchdog_timeout_ms = 1000;

  k_timer_start(&s_stuck_timer, K_MSEC(s_software_watchdog_timeout_ms),
                K_MSEC(s_software_watchdog_timeout_ms));
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

  // if coredump is present, dump it
  size_t total_size = 0;
  if (memfault_coredump_has_valid_coredump(&total_size)) {
    LOG_INF("Coredump of size %zu, dumping...", total_size);
    memfault_data_export_dump_chunks();
  }

  sw_watchdog_init();
  prv_stuck_timer();

  // Hack: force systick to a lower priority
  NVIC_SetPriority(SysTick_IRQn, _IRQ_PRIO_OFFSET + 1);

  return 0;
}
