/*
 *  author: Aleksei Kozadaev (2018)
 */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "log.h"
#include "cmd.h"

int run_system(const struct command *cmd);
int run_spawn(const struct command *cmd);
int fork_wait(pid_t pid, int timeout_ms, int *status);
long current_time_ms();

#define KILLSIG SIGINT

enum {
    RET_OK,
    RET_SIGNALED,
    RET_TIMEOUT,
    RET_ERROR
};


struct command
cmd_make()
{
    struct command ret = {
        .no_spawn = false,
        .timeout_ms = 0,
        .path = NULL,
        .teardown = NULL
    };

    return ret;
}

int
cmd_run(const struct command *cmd)
{
    if (cmd->no_spawn) {
        return run_system(cmd);
    } else {
        return run_spawn(cmd);
    }
}

int
run_system(const struct command *cmd)
{
    int retcode = system(cmd->path);

    if (retcode < 0) {
        LOG_PERROR("system");
    } else {
        LOG_INFO_VA("=== exit code %d ===", retcode);
    }

    return retcode;
}

int
run_spawn(const struct command *cmd)
{
    int status;
    pid_t pid = 0;

    if ((pid = fork()) == -1) {
        LOG_PERROR("fork");
        return pid;
    }

    if (pid == 0) {
        exit(system(cmd->path));
    }

    switch (fork_wait(pid, cmd->timeout_ms, &status)) {
        case RET_ERROR:
            LOG_PERROR("waitpid");
            break;

        case RET_OK:
            LOG_INFO_VA("=== exit code %d ===", WEXITSTATUS(status));
            break;

        case RET_SIGNALED:
            LOG_INFO_VA("=== pid %d, killed with: %d ===", pid, WTERMSIG(status));
            break;

        case RET_TIMEOUT:
            if (cmd->teardown) {
                char buf[32];
                sprintf(buf, "%d", pid);
                setenv("CMD_PID", buf, true);

                LOG_INFO_VA("=== teardown: [%s] ===", cmd->teardown);

                int status = system(cmd->teardown);

                if (status == -1) {
                    LOG_PERROR("system teardown");
                } else {
                    LOG_INFO_VA("=== teardown: exit code %d ===", status);
                }

            } else {
                if (kill(pid, KILLSIG) == -1) {
                    LOG_PERROR("timeout kill");
                } else {
                    LOG_INFO_VA("=== pid %d, killed with: %d (timeout) ===", pid, KILLSIG);
                }
            }

            break;

        default:
            LOG_ERROR("unexpected result");
            abort();
    }

    return status;
}

int
fork_wait(pid_t pid, int timeout_ms, int *status)
{
    long start = current_time_ms();
    *status = -1;

    for (;;) {
        if (waitpid(pid, status, WNOHANG) == -1) {
            return RET_ERROR;
        }

        if (WIFEXITED(*status)) {
            return RET_OK;
        } else if (WIFSIGNALED(*status)) {
            *status = WTERMSIG(*status);
            return RET_SIGNALED;
        }

        if (timeout_ms > 0 && current_time_ms() - start > timeout_ms) {
            return RET_TIMEOUT;
        }

        usleep(100);
    }
}

long
current_time_ms()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}
