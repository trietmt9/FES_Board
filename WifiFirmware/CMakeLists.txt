# SPDX-License-Identifier: Apache-2.0
#
# nRF7002DK Wi-Fi learning app.
#
# Built against the SAME vanilla Zephyr workspace as ../firmware (~/zephyrproject,
# Zephyr 4.3.99). nRF Connect SDK is NOT required: upstream Zephyr now carries the
# nrf7002dk board, the nrf_wifi driver and the nRF70 firmware blobs.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wifi_reader)

target_sources(app PRIVATE
    src/main.c
)

# Private headers live in inc/, matching ../firmware's layout.
target_include_directories(app PRIVATE inc)
