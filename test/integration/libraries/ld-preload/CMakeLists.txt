# SPDX-License-Identifier: MIT
add_library(
  itest-stop-cmd-overrides SHARED
  itest-stop-cmd-overrides.c
)

install(TARGETS itest-stop-cmd-overrides
  LIBRARY DESTINATION ${ROBOT_TEST_RESOURCE_DIR}/ld_preload/
)
