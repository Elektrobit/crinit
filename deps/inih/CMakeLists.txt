# SPDX-License-Identifier: MIT
project(
    inih-local
    VERSION 56
    DESCRIPTION "The inih parser library with a custom compile-time configuration for crinit."
    HOMEPAGE_URL "https://github.com/benhoyt/inih"
)

add_library(inih-local STATIC ini.c)
target_include_directories(inih-local PUBLIC ./)
target_compile_definitions(
    inih-local 
    PUBLIC
    INI_MAX_LINE=4096
    INI_ALLOW_INLINE_COMMENTS=0
)
