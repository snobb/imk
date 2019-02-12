/*
 *  poll_linux.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include "../cfg.h"
#include "../cmd.h"
#include "../log.h"

#define EVENT_SIZE      (sizeof(struct inotify_event))
#define BUF_LEN         (8 * (EVENT_SIZE + 16))

#define FILTERS         (IN_DELETE_SELF | IN_MOVE_SELF | IN_MODIFY | IN_ONESHOT)

static int g_ifd = -1;

int set_watch(const char *path);
static void sig_handler(int);

int
fd_register(struct config *cfg, const char *path)
{
    if (g_ifd == -1) {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        if ((g_ifd = inotify_init()) == -1) {
            LOG_PERROR("inotify_init");
            exit(1);
        }
    }

    int rv = 0;
    struct stat st;
    if ((rv = stat(path, &st)) != 0) {
        return -1;
    }

    rv = set_watch(path);
    cfg_add_fd(cfg, rv);

    return rv;
}

int
fd_dispatch(const struct config *cfg)
{
    int len;
    char buf[BUF_LEN];
    time_t next = { 0 };

    for (;;) {
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

            /* sometimes IN_DELETE_SELF or IN_MOVE_SELF mean the file is being
             * processed by some program (eg. vim or gofmt), so if imk tries
             * to reattach to the file immediately it may not exist. So sleep
             * for a while before try to reattach to the file.*/
            if ((ev->mask & IN_DELETE_SELF) || (ev->mask & IN_MOVE_SELF)) {
                usleep(cfg->sleep_del * 1000);
            }

            int idx;
            for (idx = 0; idx < cfg->nfds; ++idx) {
                if (cfg->fds[idx] == ev->wd) {
                    break;
                }
            }

            if (idx == cfg->nfds) {
                break;
            }

            LOG_INFO_VA("[====== %s (%u) =====]", cfg->files[idx], ev->wd);
            cfg->fds[idx] = set_watch(cfg->files[idx]);

            i += EVENT_SIZE + ev->len;
        }

        if (time(NULL) > next || cfg->onerun) {
            int rv = cmd_run(cfg->cmd);

            if (cfg->onerun) {
                cfg_clean(cfg);
                exit(rv);
            }

            next = time(NULL) + cfg->threshold;
        }
    }

    return 0;
}

void
fd_close(struct config *cfg)
{
    if (g_ifd != -1) {
        close(g_ifd);
    }

    for (int i = 0; i < cfg->nfds; ++i) {
        close(cfg->fds[i]);
    }

    cfg_clean(cfg);
}

int
set_watch(const char *path)
{
    int wd = inotify_add_watch(g_ifd, path, FILTERS);

    if (wd == -1) {
        LOG_PERROR("inotify_add_watch");
        exit(1);
    }

    return wd;
}

void
sig_handler(int sig)
{
    LOG_ERR("interrupted");
    exit(1);
}

