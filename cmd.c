/*
 *  author: Aleksei Kozadaev (2018)
 */

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "log.h"
#include "cmd.h"

static long current_time_ms();
static int exec_command(const struct command *cmd);
static int fork_wait(pid_t pid, int timeout_ms, int *status);
static void parse_args(char *line, char **argv, int maxlen);
static int run_system(const struct command *cmd);
static int run_fork(const struct command *cmd);
static void set_env_var(pid_t pid);
static void teardown(const char *teardown);

#define KILLSIG SIGINT
#define CMD_WORDS 255

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
        .spawn = false,
        .wrap_shell = true,
        .timeout_ms = 0,
        .path = NULL,
        .teardown = NULL
    };

    return ret;
}

int
cmd_run(const struct command *cmd)
{
    if (cmd->spawn) {
        return run_fork(cmd);
    } else {
        return run_system(cmd);
    }
}

void
cmd_print_header(const struct command *cmd)
{
    printf("cmd[%s] ", cmd->path);

    if (cmd->timeout_ms > 0) {
        printf("cmd-timeout[%d] ", cmd->timeout_ms);

        if (cmd->teardown) {
            printf("cmd-teardown[%s] ", cmd->teardown);
        }
    }

    if (cmd->spawn) {
        printf("cmd-spawn ");
    }

    if (cmd->wrap_shell) {
        printf("cmd-wrap-shell ");
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
run_fork(const struct command *cmd)
{
    int status;
    pid_t pid = 0;

    if ((pid = fork()) == -1) {
        LOG_PERROR("fork");
        return pid;
    }

    if (pid == 0) {
        if (setsid() == -1) {
            LOG_PERROR("setsid");
            return 1;
        }

        exit(exec_command(cmd));
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
                set_env_var(pid);
                teardown(cmd->teardown);
            } else {
                if (killpg(pid, KILLSIG) == -1) {
                    LOG_PERROR("killpg");
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

int
exec_command(const struct command *cmd)
{
    int rv = 0;

    if (cmd->wrap_shell) {
        rv = execl("/bin/sh", "sh", "-c", cmd->path, NULL);
    } else {
        char *argv[CMD_WORDS] = {0};
        parse_args(cmd->path, argv, CMD_WORDS);
        rv = execvp(*argv, argv);
    }

    return rv;
}

void
parse_args(char *line, char **argv, int maxlen)
{
    int i = 0;
    const char *delim = " ";
    char *token = strtok(line, delim);
    *argv++ = token;

    while (token != NULL && ++i < maxlen) {
        token = strtok(NULL, delim);
        *argv++ = token;
    }
}

void
set_env_var(pid_t pid) {
    char buf[32];
    sprintf(buf, "%d", pid);
    setenv("CMD_PID", buf, true);
}

void
teardown(const char *teardown) {
    LOG_INFO_VA("=== teardown: [%s] ===", teardown);
    int status = system(teardown);

    if (status == -1) {
        LOG_PERROR("system teardown");
    } else {
        LOG_INFO_VA("=== teardown: exit code %d ===", status);
    }
}
