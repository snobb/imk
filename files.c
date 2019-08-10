/*
 *  files.c
 *  author: Aleksei Kozadaev (2019)
 */

#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include "log.h"
#include "files.h"

#define ALLOC 64
#define MAXPATH 1024

static int is_ignored(const char *name);
static void add_file(const char *file);
static int is_directory(const char *fname);
static int walk_dir(const char *dir);

char **fstore = NULL;
size_t fsize = 0;
size_t fcap = ALLOC;
const char *ignored[] = { "/.git", "/.hg", NULL };

char **
files_parse(const char **args, size_t *nfiles)
{
    assert(!fstore);

    fstore = malloc(sizeof(*fstore) * fcap);
    if (!fstore) {
        LOG_PERROR("malloc");
        abort();
    }

    while (*args != NULL) {
        const char *fname = *args++;

        if (is_directory(fname)) {
            if (walk_dir(fname) == -1) {
                abort();
            }
        }

        add_file(fname);
    }

    *nfiles = fsize;
    return fstore;
}

void
files_free(void)
{
    for (int i = 0; i < fsize; i++) {
        free(fstore[i]);
    }

    free(fstore);
}

static void
add_file(const char *file)
{
    if (fsize == fcap) {
        fcap *= 2;

        fstore = realloc(fstore, sizeof(*fstore) * fcap);
        if (!fstore) {
            LOG_PERROR("realloc");
            abort();
        }
    }

    fstore[fsize++] = strdup(file);
}

static int
is_directory(const char *fname)
{
    struct stat stbuf;

    if (stat(fname, &stbuf) == -1) {
        LOG_PERROR_VA("%s", fname);
        return 0;
    }

    return S_ISDIR(stbuf.st_mode);
}

static int
is_ignored(const char *name)
{
    for (int i = 0; ignored[i] != NULL; i++) {
        if (strstr(name, ignored[i]) != NULL) {
            return 1;
        }
    }

    return 0;
}

/* got to the directory, make necessary checks and call (handle_files) to
 * handle the contents */
static int
walk_dir(const char *dir)
{
    char name[MAXPATH];
    struct dirent *dp;
    DIR *dfd;
    size_t dirlen;

    if ((dfd = opendir(dir)) == NULL) {
        // not a folder - nothing to do.
        return 0;
    }

    dirlen = strlen(dir);
    while ((dp = readdir(dfd)) != NULL) {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
            continue;
        }

        if ((dirlen + strlen(dp->d_name) + 2) > MAXPATH) {
            LOG_ERR("directory name is too long");
            goto error;
        }

        if (dp->d_type == DT_DIR) {
            sprintf(name, "%s/%s", dir, dp->d_name);

            if (is_ignored(name)) {
                continue;
            }

            add_file(name);

            if (walk_dir(name) == -1) {
                goto error;
            }
        }

        /* ignore everything else */
    }

    closedir(dfd);
    return 0;

error:
    if (dfd) { closedir(dfd); }
    return -1;
}

