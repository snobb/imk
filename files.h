#ifndef __FILES_H__
#define __FILES_H__

#include <stdlib.h>

char **files_parse(const char **args, size_t *nfiles);
void files_free(void);

#endif /* __FILES_H__ */
