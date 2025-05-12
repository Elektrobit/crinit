// SPDX-License-Identifier: MIT
/**
 * @file minsetup.h
 * @brief Header related to minimal early system setup.
 */
#ifndef __MINSETUP_H__
#define __MINSETUP_H__

/**
 * Sets up devtmpfs
 *
 * Meant to be used during early startup, so that necessary dev interfaces are available.
 * For kmsg logging the devtmpfs needs to be very early available, therefore it is separated
 * from crinitSetupSystemFs().
 *
 * @return 0 on success, -1 on error
 */
int crinitMountDevtmpfs(void);

/**
 * Sets up devpts, sysfs, procfs, and a tmpfs on /run
 *
 * Meant to be used during early startup, so that necessary system interfaces are available.
 *
 * @return 0 on success, -1 on error
 */
int crinitSetupSystemFs(void);

/**
 * Forks the calling process, parent will enter a wait-loop, child will return.
 *
 * Meant to be used during early startup if the calling process is PID 1. After the call, PID 1 will be in a permanent
 * wait-loop to take care of orphaned processes. The new process will return from the funtion and go on.
 *
 * @return -1 if fork() fails, 0 on success in the child process, parent process will never return
 */
int crinitForkZombieReaper(void);

#endif /* __MINSETUP_H__ */
