/**
 * @file minsetup.h
 * @brief Header related to minimal early system setup.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __MINSETUP_H__
#define __MINSETUP_H__

/**
 * Sets up devtmpfs, devpts, sysfs, and procfs.
 *
 * Meant to be used during early startup, so that necessary system interfaces are available.
 *
 * @return 0 on success, -1 on error
 */
int EBCL_setupSystemFs(void);

/**
 * Forks the calling process, parent will enter a wait-loop, child will return.
 *
 * Meant to be used during early startup if the calling process is PID 1. After the call, PID 1 will be in a permanent
 * wait-loop to take care of orphaned processes. The new process will return from the funtion and go on.
 *
 * @return -1 if fork() fails, 0 on success in the child process, parent process will never return
 */
int EBCL_forkZombieReaper(void);

#endif /* __MINSETUP_H__ */
