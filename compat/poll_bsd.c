/*
 *  poll_bsd.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/event.h>

#include "../imk.h"
#include "../log.h"

static int g_kq = -1;
static bool g_running = false;

static int set_watch(const char *path);
static void sig_handler(int);

int
fd_register(const struct config *cfg, const char *path)
{
    if (g_kq == -1) {
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        if ((g_kq = kqueue()) == -1) {
            LOG_PERROR("kqueue");
        }
    }

    return set_watch(path);
}

int
fd_dispatch(const struct config *cfg)
{
    struct kevent ev;

    g_running = true;
    while (g_running) {
        memset(&ev, 0, sizeof(ev));

        if (kevent(g_kq, NULL, 0, &ev, 1, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            }
            LOG_PERROR("kevent");
        }

        int idx;
        for (idx = 0; idx < cfg->fds.size; ++idx) {
            if (cfg->fds.data[idx] == (int)(intptr_t)ev.udata) {
                break;
            }
        }

        int fd = set_watch(cfg->files[idx]);
        cfg->fds.data[idx] = fd;

        LOG_INFO_VA("[====== %s (%u) =====]", cfg->files[idx], fd);
        system(cfg->cmd);
    }

    return 0;
}

void
fd_close(const struct config *cfg)
{
    close(g_kq);

    for (int i = 0; i < cfg->fds.size; ++i) {
        close(cfg->fds.data[i]);
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
    EV_SET(&ev, fd, filter, EV_ADD | EV_ONESHOT,
            NOTE_WRITE | NOTE_DELETE, 0, (void*)(intptr_t)fd);

    if (kevent(g_kq, &ev, 1, NULL , 0, NULL) == -1) {
        LOG_PERROR("kevent");
    }

    return fd;
}

void
sig_handler(int sig)
{
    g_running = false;
    LOG_ERR("interrupted");
}

