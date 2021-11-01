/**
 * @file reboot.c
 * @brief Implementation of the poweroff service program using the crinit-client library.
 *
 * Program usage info:
 * ~~~
 * USAGE: reboot [-v/--verbose]
 *     Will request Crinit to perform a graceful system reboot.
 *     Specifying '-v/--verbose' will give verbose output.
 * ~~~
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>

#include "crinit-client.h"
#include "logio.h"

int main(int argc, char *argv[]) {
    EBCL_setPrintPrefix("");
    if (argc != 1 && argc != 2) {
        EBCL_errPrint("Wrong number of arguments.");
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0) {
            EBCL_crinitSetVerbose(true);
        } else {
            EBCL_errPrint("Unknown argument: %s", argv[1]);
            return EXIT_FAILURE;
        }
    }
    if (EBCL_crinitShutdown(RB_AUTOBOOT) == -1) {
        EBCL_errPrint("Could not request reboot from Crinit.");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

