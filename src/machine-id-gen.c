/**
 * @file crinit-ctl.c
 * @brief Example implementation of the machine-id-gen service program.
 *
 * In a production system, the implementation is customer-defined to offer flexibility. The general requirement for this
 * program is to write a system-unique identifier (in any format, i.e. unique arbitrary data) to /etc/machine-id. The
 * identifier shall not have a random component as it may not be persisted to disk (in order to support read-only file-
 * systems).
 *
 * Program usage info:
 *
 * ~~~
 * This program shall be called without additional arguments through a Crinit task. It will generate an appropriate
 * /etc/machine-id file if it is either run on S32G or the Kernel command line contains a value for systemd.machine_id.
 * The latter will take precedence over the S32G ROM serial number if set.
 * ~~~
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define S32G_OCOTP_BASE 0x400a4000uL
#define S32G_OCOTP_OFFSET_UID 0x210uL
#define S32G_SIUL21_BASE 0x44010000uL
#define S32G_SIUL21_OFFSET_MIDR1 0x4uL
#define S32G_SIUL21_MCUID_VAL 0x4C200000uL
#define S32G_SIUL21_MCUID_MASK 0xFFFF0000uL

#define MID_STR_LEN 36

#define KERNEL_CMDLINE_PATH "/proc/cmdline"
#define KERNEL_CMDLINE_MAX_LEN 4096
#define KERNEL_CMDLINE_KEY "systemd.machine_id"

#define MACHINE_ID_PATH "/etc/machine-id"

static void EBCL_printUsage(const char *basename);

static int EBCL_getMidKernelCmdLine(char *mid, size_t n);
static int EBCL_readPhysMem(void *data, off_t phyAddr, size_t len);
static int EBCL_readS32Uid(uint64_t *uid);
static inline int EBCL_s32UidToMid(char *mid, size_t n, uint64_t uid);
static int EBCL_checkS32(bool *onS32);

int main(int argc, char *argv[]) {
    if (argc != 1) {
        EBCL_printUsage(argv[0]);
        return 0;
    }

    char machId[MID_STR_LEN + 1] = {'\0'};

    printf("Checking Kernel command line for machine ID...");
    if (EBCL_getMidKernelCmdLine(machId, sizeof(machId)) == 0) {
        printf(" Found!\n");
    } else {
        printf(" None found.\nWill check if we are on S32 hardware... ");
        bool onS32 = false;
        if (EBCL_checkS32(&onS32) == -1) {
            printf("Could not determine if we are running on an S32G (real hardware).\n");
            return EXIT_FAILURE;
        }

        if (!onS32) {
            printf("We do not seem to be on an NXP S32-based host system.\n");
            return EXIT_FAILURE;
        }

        printf("Yes.\nWill generate machine ID from unique ID in OTP memory... ");

        uint64_t uid = 0;

        if (EBCL_readS32Uid(&uid) == -1) {
            printf("Could not read UID from S32G OTP memory.\n");
            return EXIT_FAILURE;
        }

        EBCL_s32UidToMid(machId, sizeof(machId), uid);
    }
    FILE *machIdF = fopen(MACHINE_ID_PATH, "w");
    if (machIdF == NULL) {
        perror("Could not open \'" MACHINE_ID_PATH "\' for writing");
        return EXIT_FAILURE;
    }
    if (fprintf(machIdF, "%s\n", machId) < 0) {
        fprintf(stderr, "Could not write to \'" MACHINE_ID_PATH "\'.\n");
        return EXIT_FAILURE;
    }
    printf("Done.\nMachine ID: %s\n", machId);
    return EXIT_SUCCESS;
}

static void EBCL_printUsage(const char *basename) {
    fprintf(stderr,
            "USAGE: %s"
            "This program shall be called without additional arguments through a Crinit task. It will generate an "
            "appropriate /etc/machine-id file if it is either run on S32G or the Kernel command line contains a value\n"
            "for systemd.machine_id. The latter will take precedence over the S32G ROM serial number if set.\n",
            basename);
}

static int EBCL_checkS32(bool *onS32) {
    uint32_t mcuIdReg = 0;
    if (EBCL_readPhysMem(&mcuIdReg, S32G_SIUL21_BASE + S32G_SIUL21_OFFSET_MIDR1, sizeof(uint32_t)) == -1) {
        fprintf(stderr, "Could not read MCU ID register from physical memory address %lu.",
                S32G_SIUL21_BASE + S32G_SIUL21_OFFSET_MIDR1);
        return -1;
    }

    *onS32 = (mcuIdReg & S32G_SIUL21_MCUID_MASK) == S32G_SIUL21_MCUID_VAL;
    return 0;
}

static int EBCL_readS32Uid(uint64_t *uid) {
    if (EBCL_readPhysMem(uid, S32G_OCOTP_BASE + S32G_OCOTP_OFFSET_UID, sizeof(uint64_t)) == -1) {
        fprintf(stderr, "Could not read unique ID OCOTP shadow registers from physical memory address %lu.",
                S32G_OCOTP_BASE + S32G_OCOTP_OFFSET_UID);
        return -1;
    }
    return 0;
}

static int EBCL_getMidKernelCmdLine(char *mid, size_t n) {
    char cmdLine[KERNEL_CMDLINE_MAX_LEN] = {'\0'};

    FILE *kCmdF = fopen(KERNEL_CMDLINE_PATH, "rb");
    if (kCmdF == NULL) {
        perror("Could not open " KERNEL_CMDLINE_PATH);
        return -1;
    }
    if (fread(cmdLine, sizeof(cmdLine) - 1, 1, kCmdF) != sizeof(cmdLine) - 1 && ferror(kCmdF) != 0) {
        fprintf(stderr, "Error reading from Kernel command line.");
        fclose(kCmdF);
        return -1;
    }
    fclose(kCmdF);
    char *midPtr = strstr(cmdLine, KERNEL_CMDLINE_KEY "=");
    if (midPtr == NULL) {
        return -2;
    }

    // Find beginning of value for machine ID.
    midPtr = strchr(midPtr, '=') + 1;
    char *midEnd = strchr(midPtr, ' ');
    // Find end of value for machine ID. Either a space or the end of the string.
    if (midEnd != NULL) {
        n = (n > (size_t)(midEnd - midPtr) + 1) ? (size_t)(midEnd - midPtr) + 1 : n;
    }
    strncpy(mid, midPtr, n);
    mid[n - 1] = '\0';

    return 0;
}

static inline int EBCL_s32UidToMid(char *mid, size_t n, uint64_t uid) {
    uint8_t bytes[sizeof(uint64_t)];
    memcpy(bytes, &uid, sizeof(uint64_t));
    return snprintf(mid, n, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", bytes[0], bytes[1],
                    bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], bytes[0], bytes[1], bytes[2], bytes[3],
                    bytes[4], bytes[5], bytes[6], bytes[7]);
}

static int EBCL_readPhysMem(void *data, off_t phyAddr, size_t len) {
    size_t pageSize = sysconf(_SC_PAGE_SIZE);
    off_t pageBase = (phyAddr / pageSize) * pageSize;
    off_t mapLen = (phyAddr - pageBase) + len;

    int memFd = open("/dev/mem", O_SYNC);
    if (memFd < 0) {
        perror("Could not open /dev/mem");
        return -1;
    }

    uint8_t *mem = mmap(NULL, mapLen, PROT_READ, MAP_PRIVATE, memFd, pageBase);
    if (mem == MAP_FAILED) {
        perror("Could not map memory from /dev/mem");
        close(memFd);
        return -1;
    }
    memcpy(data, mem + (phyAddr - pageBase), len);
    munmap(mem, mapLen);
    close(memFd);
    return 0;
}
