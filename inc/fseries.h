/**
 * @file fseries.h
 * @brief Header defining functions related to the handling of a series of filenames.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __FSERIES_H__
#define __FSERIES_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct ebcl_FileSeries_t {
    char **fnames;
    size_t size;
    char *baseDir;
} ebcl_FileSeries_t;

int EBCL_fileSeriesFromDir(ebcl_FileSeries_t *fse, const char *path, const char *fileSuffix, bool followLinks);
void EBCL_destroyFileSeries(ebcl_FileSeries_t *fse);
int EBCL_initFileSeries(ebcl_FileSeries_t *fse, size_t numElements, const char *baseDir);
int EBCL_resizeFileSeries(ebcl_FileSeries_t *fse, size_t numElements);

#endif /* __FSERIES_H__ */
