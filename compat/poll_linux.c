/*
 *  poll_linux.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/inotify.h>

#include "../imk.h"
#include "../log.h"

#define EVENT_SIZE      (sizeof(struct inotify_event))
#define BUF_LEN         (1024 * (EVENT_SIZE + 16))

#define FILTERS              IN_MODIFY | IN_ONESHOT

static int s_ifd = -1;
static volatile bool s_running = false;

static void sig_handler(int);

int
fd_register(const struct config *cfg, const char *path)
{
    int rv;

    if (s_ifd == -1) {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        if ((s_ifd = inotify_init()) == -1) {
            LOG_PERROR("inotify_init");
            exit(1);
        }
    }

    rv = inotify_add_watch(s_ifd, path, FILTERS);

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

    s_running = true;
    while (s_running) {
        if ((len = read(s_ifd, buf, BUF_LEN)) == -1) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            LOG_PERROR("read");
            exit(1);
        }

        for (int i = 0; i < len;) {
            struct inotify_event *event = (struct inotify_event *)&buf[i];
            const char *name;

            int idx;
            for (idx = 0; idx < cfg->fds.size; ++idx) {
                if (cfg->fds.data[idx] == event->wd) {
                    break;
                }
            }

            int wd;
            if ((wd = inotify_add_watch(s_ifd, cfg->files[idx], FILTERS)) == -1) {
                LOG_PERROR("inotify_add_watch");
            }
            cfg->fds.data[idx] = wd;

            LOG_INFO_VA("[====== %s (%u) =====]",
                    cfg->files[idx], event->wd);

            i += EVENT_SIZE + event->len;
        }

        system(cfg->cmd);
    }

    return 0;
}

void
fd_close(void)
{
    if (s_ifd != -1) {
        close(s_ifd);
    }
}

void
sig_handler(int sig)
{
    s_running = false;
    LOG_ERR("interrupted");
}

