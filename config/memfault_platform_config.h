#pragma once

//! @file
//!
//! @brief
//! Platform overrides for the default configuration settings in the memfault-firmware-sdk.
//! Default configuration settings can be found in "memfault/config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Collect all populated NVIC IRQs when saving a coredump. Pad to nearest
// multiple of 32.
#define MEMFAULT_NVIC_INTERRUPTS_TO_COLLECT (((CONFIG_NUM_IRQS) + 31) & ~31)

#ifdef __cplusplus
}
#endif
