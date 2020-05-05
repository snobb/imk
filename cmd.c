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
int fork_wait(pid_t pid, int timeout_ms, int *status, int signal);
long current_time_ms();

int
cmd_run(const struct command *cmd)
{
    if (cmd->spawn || cmd->timeout_ms > 0) {
        return run_spawn(cmd);
    } else {
        return run_system(cmd);
    }
}

int
run_system(const struct command *cmd)
{
    int retcode = system(cmd->path);

    if (retcode < 0) {
        LOG_PERROR("[=== system ===]");
    } else {
        LOG_INFO_VA("[=== (exit code %d) ===]", retcode);
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

    switch (fork_wait(pid, cmd->timeout_ms, &status, SIGINT)) {
        case -1:
            LOG_PERROR("waitpid");
            break;

        case 0:
            LOG_INFO_VA("[=== (exit code %d) ===]", WEXITSTATUS(status));
            break;

        default:
            LOG_INFO_VA("[=== (pid %d, killed with: %d) ===]", pid,
                        WTERMSIG(status));
            break;
    }

    return status;
}

int
fork_wait(pid_t pid, int timeout_ms, int *status, int signal)
{
    long start = current_time_ms();
    *status = -1;

    for (;;) {
        if (waitpid(pid, status, WNOHANG) == -1) {
            return -1;
        }

        if (status == NULL) {
            continue;
        }

        if (WIFEXITED(*status)) {
            return 0;
        } else if (WIFSIGNALED(*status)) {
            return WTERMSIG(*status);
        }

        if (current_time_ms() - start > timeout_ms) {
            kill(pid, signal);
            return signal;
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
