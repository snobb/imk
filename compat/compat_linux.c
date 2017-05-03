/*
 *  poll_linux.c
 *  author: Aleksei Kozadaev (2016)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include "../imk.h"
#include "../log.h"
#include "../lua_interp.h"

#define EVENT_SIZE      (sizeof(struct inotify_event))
#define BUF_LEN         (1024 * (EVENT_SIZE + 16))

#define FILTERS         (IN_ALL_EVENTS | IN_EXCL_UNLINK)

static int g_ifd = -1;
static bool g_running = false;

ARRAY_FUNCS(fd, int)  // array handling functions

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

        array_fd_init(&cfg->fds);
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        return -1;
    }

    int fd = inotify_add_watch(g_ifd, path, FILTERS);
    if (fd == -1) {
        LOG_PERROR("inotify_add_watch");
        exit(1);
    }

    array_fd_append(&cfg->fds, fd);
    return fd;
}

int
fd_dispatch(const struct config *cfg)
{
    int len;
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

        bool need_cmd = false;
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

            if (ev->mask & IN_OPEN) {
                LOG_DEBUG("trigger open cb for %s", cfg->files[idx]);
                lua_trigger_open(cfg->lua, cfg->files[idx]);
            }
            else if (ev->mask & IN_CLOSE) {
                LOG_DEBUG("trigger close cb for %s", cfg->files[idx]);
                lua_trigger_close(cfg->lua, cfg->files[idx]);
            }
            else if (ev->mask & IN_CREATE) {
                LOG_DEBUG("trigger create cb for %s", cfg->files[idx]);
                lua_trigger_create(cfg->lua, cfg->files[idx]);
            }
            else if (ev->mask & IN_ACCESS) {
                LOG_DEBUG("trigger read cb for %s", cfg->files[idx]);
                lua_trigger_read(cfg->lua, cfg->files[idx]);
            }
            else if ((ev->mask & IN_ATTRIB) || (ev->mask & IN_MODIFY)) {
                LOG_DEBUG("trigger write cb for %s", cfg->files[idx]);
                LOG_INFO_VA("[ ====== event %s (%u) =====]", cfg->files[idx],
                        ev->wd);

                int rv = lua_trigger_write(cfg->lua, cfg->files[idx]);
                if (rv != LUA_OK) {
                    need_cmd = true;
                }
            }
            else if (ev->mask & IN_MOVE_SELF) {
                int wd = inotify_add_watch(g_ifd, cfg->files[idx], FILTERS);
                if (wd == -1) {
                    LOG_PERROR("inotify_add_watch");
                }
                cfg->fds.data[idx] = wd;
            }
            else if ((ev->mask & IN_DELETE) || (ev->mask & IN_DELETE_SELF)) {
                LOG_DEBUG("trigger delete cb for %s (%d)", cfg->files[idx],
                        ev->cookie);
                lua_trigger_delete(cfg->lua, cfg->files[idx]);
                if (ev->mask & IN_DELETE_SELF) {
                    cfg->files[idx] = NULL;
                    cfg->fds.data[idx] = -1;
                }
            }
            else if (ev->mask & IN_IGNORED) {
                LOG_DEBUG("IGNORED event for %s", cfg->files[idx]);
            }

            i += EVENT_SIZE + ev->len;
        }

        if (need_cmd && cfg->cmd != NULL) {
            /* fallback to default action */
            system(cfg->cmd);
            need_cmd = false;
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

    for (int i = 0; i < cfg->fds.size; ++i) {
        close(cfg->fds.data[i]);
    }

    array_fd_free(&cfg->fds);
}

void
sig_handler(int sig)
{
    g_running = false;
    LOG_ERR("interrupted");
}

