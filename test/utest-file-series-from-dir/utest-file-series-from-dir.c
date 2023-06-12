/**
 * @file utest-file-series-from-dir.c
 * @brief Implementation of the EBCL_fileSeriesFromDir() unit test group.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "utest-file-series-from-dir.h"

#include <dirent.h>
#include <sys/types.h>

#include "common.h"
#include "fseries.h"
#include "unit_test.h"

int EBCL_initFileSeries(ebcl_FileSeries_t *fse, size_t numElements, const char *baseDir) {
    CRINIT_PARAM_UNUSED(numElements);
    CRINIT_PARAM_UNUSED(baseDir);

    if (fse == (void *)0xbaadda7a) return -1;

    return 0;
}

void EBCL_destroyFileSeries(ebcl_FileSeries_t *fse) {
    CRINIT_PARAM_UNUSED(fse);
}

void EBCL_freeScandirList(struct dirent **scanList, int size) {
    CRINIT_PARAM_UNUSED(scanList);
    CRINIT_PARAM_UNUSED(size);
}

/**
 * Runs the unit test group for EBCL_fileSeriesFromDir using the cmocka API.
 */
int main(void) {
    const struct CMUnitTest tests[] = {
        // clang-format off
        // Rationale: unreadable output of clang-format
        cmocka_unit_test(EBCL_fileSeriesFromDirTestSuccess),
        cmocka_unit_test(EBCL_fileSeriesFromDirParamNullError),
        cmocka_unit_test(EBCL_fileSeriesFromDirOpendirError),
        cmocka_unit_test(EBCL_fileSeriesFromDirDirfdError),
        cmocka_unit_test(EBCL_fileSeriesFromDirScandirError),
        cmocka_unit_test(EBCL_fileSeriesFromDirInitError),
        cmocka_unit_test(EBCL_fileSeriesFromDirNoMemError),
        // clang-format on
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
