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

#define S32G_OCOTP_BASE 0x400a4000uL   ///< Memory-mapped base address of the S32G OCOTP memory controller
#define S32G_OCOTP_OFFSET_UID 0x210uL  ///< Offset to the unique ID shadow registers (2x32-Bit consecutive)

#define S32G_SIUL21_BASE 0x44010000uL        ///< Memory-mapped base address of the S32G SIUL2_1 subsystem.
#define S32G_SIUL21_OFFSET_MIDR1 0x4uL       ///< Offset to the SIUL2 MCU ID register 1 (MIDR1).
#define S32G_SIUL21_MCUID_VAL 0x4C200000uL   ///< Upper 16-Bit of MIDR1 valid for all S32G models.
#define S32G_SIUL21_MCUID_MASK 0xFFFF0000uL  ///< Bitmask of upper 16-Bit (for MIDR1 access).

#define MID_STR_LEN 36  ///< Length of a 128-Bit UUIDv4 not including the terminating zero.

#define KERNEL_CMDLINE_PATH "/proc/cmdline"      ///< Where the (pseudo-)file containing the Kernel command line is.
#define KERNEL_CMDLINE_MAX_LEN 4096              ///< Maximum length of the Kernel command line options to be read.
#define KERNEL_CMDLINE_KEY "systemd.machine_id"  ///< Kernel command line key to set machine ID.

#define MACHINE_ID_PATH "/etc/machine-id"  ///< Path to the machine-id file to generate.

/**
 * Print application usage information (to stderr).
 *
 * @param basename  The name of this program.
 */
static void EBCL_printUsage(const char *basename);

/**
 * Tries to get the machine-id from the Kernel command line.
 *
 * Searches the Kernel command line for a value for #KERNEL_COMMAND_LINE_KEY and copies it to \a mid if found and n is
 * large enough.
 *
 * @param mid  Return pointer for the machine-id UUID string.
 * @param n    Maximum size of the string that can be returned.
 *
 * @return  0 on success, -1 on a general error, and -2 if everything went well but #KERNEL_CMDLINE_KEY is not found in
 * the Kernel commandline.
 */
static int EBCL_getMidKernelCmdLine(char *mid, size_t n);

#ifdef __aarch64__ /* It only makes sense to include special code for S32G if we compile for aarch64 */

/**
 * Read bytes from physical memory addresses.
 *
 * Will write \a len Bytes of data from physical memory address \a phyAddr to the location pointed to by \a data. Uses
 * the /dev/mem interface provided by the Linux Kernel.
 *
 * @param data     Return pointer for the data.
 * @param phyAddr  Physical memory address to read from.
 * @param len      Amount of Bytes to copy.
 *
 * @return  0 on success, -1 otherwise
 */
static int EBCL_readPhysMem(void *data, off_t phyAddr, size_t len);
/**
 * Reads the the NXP S32G's 64-Bit unique (per-chip) ID from the OTP shadow registers.
 *
 * See NXP S32G2 reference manual Ch. 63.
 *
 * @param uid  Return pointer for the 64-Bit unique ID.
 *
 * @return  0 on success, -1 otherwise
 */
static int EBCL_readS32Uid(uint64_t *uid);
/**
 * Converts S32 64-Bit unique ID to a 128-Bit UUID string format.
 *
 * It naively duplicates the number to get 128-Bits. If the unique ID is considered confidential some authenticated
 * hashing method (e.g. sha256+HMAC) would have to be used.
 *
 * @param mid  The machine (UU)ID string.
 * @param n    The maximum size of the returned string, should be at least (#MID_STR_LEN+1).
 * @param uid  The 64-Bit unique ID to convert.
 *
 * @return  The number of characters written on success or a negative number on error.
 */
static inline int EBCL_s32UidToMid(char *mid, size_t n, uint64_t uid);
/**
 * Checks if we are running on an S32G.
 *
 * Checks the S32G's MCU ID register 1 in the SIUL2_1 on-chip peripheral if it indicates the chip is an S32, see NXP
 * S32G2 reference manual Ch. 16.
 *
 * @param onS32  Return pointer, set to true if the MCU ID register is readable and contains the expected value for an
 * S32-family chip.
 *
 * @return  0 on success (meaning the memory address of the register could be read), -1 on error
 */
static int EBCL_checkS32(bool *onS32);

#endif /* __aarch64__ */

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
        printf(" None found.\n");

#ifdef __aarch64__ /* It only makes sense to include special code for S32G if we compile for aarch64 */
        printf("Will check if we are on S32 hardware... ");
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
#else
        return EXIT_FAILURE;
#endif /* __aarch64__ */
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

#ifdef __aarch64__ /* It only makes sense to include special code for S32G if we compile for aarch64 */

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

#endif /* __aarch64__ */
