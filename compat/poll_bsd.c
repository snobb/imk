/*
 *  poll_bsd.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/event.h>

#include "../cfg.h"
#include "../cmd.h"
#include "../log.h"

static int g_kq = -1;

static int set_watch(const char *path);
static void sig_handler(int);

int
fd_register(struct config *cfg, const char *path)
{
    if (g_kq == -1) {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        if ((g_kq = kqueue()) == -1) {
            LOG_PERROR("kqueue");
            exit(1);
        }
    }

    int fd = set_watch(path);
    cfg_add_fd(cfg, fd);
    return fd;
}

int
fd_dispatch(const struct config *cfg)
{
    struct kevent ev;
    time_t next = { 0 };

    for (;;) {
        memset(&ev, 0, sizeof(ev));

        if (kevent(g_kq, NULL, 0, &ev, 1, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            }

            LOG_PERROR("kevent");
            exit(1);
        }

        /* sometimes NOTE_DELETE or NOTE_RENAME mean the file is being
         * processed by some program (eg. vim or gofmt), so if imk tries
         * to reattach to the file immediately it may not exist. So sleep
         * for a while before try to reattach to the file.*/
        if ((ev.filter == EVFILT_VNODE) &&
                ((ev.fflags & NOTE_DELETE) || (ev.fflags & NOTE_RENAME))) {
            usleep(cfg->sleep_delay * 1000);
        }

        int idx;

        for (idx = 0; idx < cfg->nfds; ++idx) {
            if (cfg->fds[idx] == (int)(intptr_t)ev.udata) {
                break;
            }
        }

        if (idx == cfg->nfds) {
            break; /* not found */
        }

        int fd = set_watch(cfg->files[idx]);

        if (fd == -1) {
            LOG_INFO_VA("=== %s deleted ===", cfg->files[idx]);
        } else {
            LOG_INFO_VA("=== %s (%u) ===", cfg->files[idx], cfg->fds[idx]);
            cfg->fds[idx] = fd;
        }

        if (time(NULL) > next || cfg->onerun) {
            int rv = cmd_run(cfg->cmd);

            if (cfg->onerun) {
                cfg_free(cfg);
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
    if (g_kq != -1) {
        close(g_kq);
    }

    for (int i = 0; i < cfg->nfds; ++i) {
        close(cfg->fds[i]);
    }
}

int
set_watch(const char *path)
{
    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        return fd;
    }

    struct kevent ev;

    int filter = EVFILT_VNODE;

    EV_SET(&ev, fd, filter, EV_ADD | EV_ONESHOT, NOTE_WRITE |
           NOTE_DELETE | NOTE_RENAME, 0, (void *)(intptr_t)fd);

    if (kevent(g_kq, &ev, 1, NULL, 0, NULL) == -1) {
        LOG_PERROR("kevent");
        exit(1);
    }

    return fd;
}

void
sig_handler(int sig)
{
    LOG_ERROR("interrupted");
    exit(1);
}

