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

// dump out the NVIC IPR registers, 0-60, 8 bits at a time
static void prv_dump_nvic_ipr(void) {
  printk("\n=== NVIC IPR Registers ===\n");
  for (int i = 0; i < 60; i++) {
    volatile uint32_t *ipr_reg = (volatile uint32_t *)(NVIC_IPR_BASE + i * 4);
    // uint8_t priority = (*ipr_reg >> ((i % 4) * 8 + 4)) & 0xFF;
    printk("IPR[%02d]: %04X\n", i, *ipr_reg);
  }
}

static void prv_print_nvic_vector_table(void) {
  // Get the Vector Table Offset Register (VTOR)
  uint32_t *vtor = (uint32_t *)(*((volatile uint32_t *)SCB_VTOR_ADDR));

  printk("\n=== NVIC Vector Table Information ===\n");
  printk("VTOR Address: 0x%08X\n", (uint32_t)vtor);
  printk("Format: IRQ#, Priority, Handler Address\n\n");

  // Print system exceptions (negative IRQ numbers)
  printk("System Exceptions:\n");
  printk("  Reset:     N/A, N/A,       0x%08X\n", vtor[1]);  // Reset handler (SP is at vtor[0])
  printk("  NMI:       -14, -1,       0x%08X\n", vtor[2]);
  printk("  HardFault: -13, -2,       0x%08X\n", vtor[3]);
  printk("  MemManage: -12, 0x%02X,     0x%08X\n", NVIC_GetPriority(-12), vtor[4]);
  printk("  BusFault:  -11, 0x%02X,     0x%08X\n", NVIC_GetPriority(-11), vtor[5]);
  printk("  UsageFault:-10, 0x%02X,     0x%08X\n", NVIC_GetPriority(-10), vtor[6]);
  printk("  SVCall:    -5,  0x%02X,     0x%08X\n", NVIC_GetPriority(-5), vtor[11]);
  printk("  PendSV:    -2,  0x%02X,     0x%08X\n", NVIC_GetPriority(-2), vtor[14]);
  printk("  SysTick:   -1,  0x%02X,     0x%08X\n", NVIC_GetPriority(-1), vtor[15]);

  printk("\nExternal Interrupts:\n");

  // Check external interrupts (IRQ 0 and up)
  for (int irq = 0; irq < CONFIG_NUM_IRQS; irq++) {
    uint32_t handler_addr = vtor[16 + irq];  // External IRQs start at offset 16

    // Only print if the handler is populated (not default)
    if (handler_addr != 0 && handler_addr != 0xFFFFFFFF) {
      // Get priority from NVIC_IPR registers
      // Each IPR register holds 4 priorities (8 bits each, upper 4 bits used)
      uint32_t ipr_index = irq / 4;
      uint32_t ipr_offset = irq % 4;
      volatile uint32_t *ipr_reg = (volatile uint32_t *)(NVIC_IPR_BASE + ipr_index * 4);
      uint8_t priority = ((*ipr_reg) >> (8 * ipr_offset + 4)) & 0xF;

      // Check if interrupt is enabled
      uint32_t iser_index = irq / 32;
      uint32_t iser_bit = irq % 32;
      volatile uint32_t *iser_reg = (volatile uint32_t *)(NVIC_ISER_BASE + iser_index * 4);
      bool enabled = (*iser_reg & (1U << iser_bit)) != 0;

      printk("  IRQ %3d:   %3d, 0x%02X, %s   0x%08X\n", irq, irq, priority, enabled ? "EN " : "DIS",
             handler_addr);
    }
  }

  printk("\n");
}

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

  // Print out NVIC vector table information
  prv_dump_nvic_ipr();
  // prv_print_nvic_vector_table();

  return 0;
}
