# SPDX-License-Identifier: MIT
#
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_AR aarch64-linux-gnu-ar)
add_definitions("-march=armv8-a+crc -mcpu=cortex-a53 -DUintMaxTypePrintfFormat='\"%#lx\"' -DUintMaxTypePrintfFormatDecimal='\"%lu\"'")
