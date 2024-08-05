# SPDX-License-Identifier: Apache-2.0

# Default board
set(BOARD stm32h7b3i_dk)

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wifi)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
zephyr_include_directories(config)
