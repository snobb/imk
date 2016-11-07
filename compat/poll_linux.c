/*
 *  poll_linux.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include "../imk.h"
#include "../log.h"

#define EVENT_SIZE      (sizeof(struct inotify_event))
#define BUF_LEN         (1024 * (EVENT_SIZE + 16))

#define FILTERS         (IN_MODIFY | IN_ONESHOT)

static int g_ifd = -1;
static bool g_running = false;

static void sig_handler(int);

int
fd_register(const struct config *cfg, const char *path)
{
    int rv;

    if (g_ifd == -1) {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        if ((g_ifd = inotify_init()) == -1) {
            LOG_PERROR("inotify_init");
            exit(1);
        }
    }

    struct stat st;
    if ((rv = stat(path, &st)) != 0) {
        return -1;
    }

    rv = inotify_add_watch(g_ifd, path, FILTERS);

    if (rv == -1) {
        LOG_PERROR("inotify_add_watch");
        exit(1);
    }

    return rv;
}

int
fd_dispatch(const struct config *cfg)
{
    int rv, len;
    char buf[BUF_LEN];

    g_running = true;
    while (g_running) {
        if ((len = read(g_ifd, buf, BUF_LEN)) == -1) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOG_PERROR("read");
            exit(1);
        }

        if (len == 0) {
            continue;
        }

        for (int i = 0; i < len;) {
            struct inotify_event *ev = (struct inotify_event *)&buf[i];

            int idx;
            for (idx = 0; idx < cfg->fds.size; ++idx) {
                if (cfg->fds.data[idx] == ev->wd) {
                    break;
                }
            }

            if (idx == cfg->fds.size) {
                break;  /* not found */
            }

            LOG_INFO_VA("[====== %s (%u) =====]",
                    cfg->files[idx], ev->wd);

            int wd = inotify_add_watch(g_ifd, cfg->files[idx], FILTERS);
            if (wd == -1) {
                LOG_PERROR("inotify_add_watch");
            }
            cfg->fds.data[idx] = wd;

            i += EVENT_SIZE + ev->len;
        }

        system(cfg->cmd);
    }

    return 0;
}

void
fd_close(const struct config *cfg)
{
    if (g_ifd != -1) {
        close(g_ifd);
    }
}

void
sig_handler(int sig)
{
    g_running = false;
    LOG_ERR("interrupted");
}

