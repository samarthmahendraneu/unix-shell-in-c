/*
 * Skeleton code for Lab 2 - Shell processing
 * This file contains skeleton code for executing parsed commands.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "cmdparse.h"
#include "cmdrun.h"

/* cmd_exec(cmd, pass_pipefd)
 *
 *   Execute the single command specified in the 'cmd' command structure.
 *
 *   The 'pass_pipefd' argument is used for pipes.
 *
 *     On input, '*pass_pipefd' is the file descriptor that the
 *     current command should use to read the output of the previous
 *     command. That is, it's the "read end" of the previous
 *     pipe, if there was a previous pipe; if there was not, then
 *     *pass_pipefd will equal STDIN_FILENO.
 *
 *     On output, cmd_exec should set '*pass_pipefd' to the file descriptor
 *     used for reading from THIS command's pipe (so that the next command
 *     can use it). If this command didn't have a pipe -- that is,
 *     if cmd->controlop != PIPE -- then this function should set
 *     '*pass_pipefd = STDIN_FILENO'.
 *
 *   Returns the process ID of the forked child, or < 0 if some system call
 *   fails.
 *
 *   Besides handling normal commands, redirection, and pipes, you must also
 *   handle three internal commands: "cd", "exit", and "our_pwd".
 *   (Why must "cd", "exit", and "our_pwd" (a version of "pwd") be implemented
 *   by the shell, versus simply exec()ing to handle them?)
 *
 *   Note that these special commands still have a status!
 *   For example, "cd DIR" should return status 0 if we successfully change
 *   to the DIR directory, and status 1 otherwise.
 *   Thus, "cd /tmp && echo /tmp exists" should print "/tmp exists" to stdout
 *      if and only if the /tmp directory exists.
 *   Not only this, but redirections should work too!
 *   For example, "cd /tmp > foo" should create an empty file named 'foo';
 *   and "cd /tmp 2> foo" should print any error messages to 'foo'.
 *
 *   Some specifications:
 *
 *       --subshells:
 *         the exit status should be either 0 (if the last
 *         command would have returned 0) or 5 (if the last command
 *         would have returned something non-zero). This is not the
 *         behavior of bash.
 *
 *       --cd:
 *
 *          this builtin takes exactly one argument besides itself (this
 *          is also not bash's behavior). if it is given fewer
 *          ("cd") or more ("cd foo bar"), that is a syntax error.  Any
 *          error (syntax or system call) should result in a non-zero
 *          exit status. Here is the specification for output:
 *
 *                ----If there is a syntax error, your shell should
 *                display the following message verbatim:
 *                   "cd: Syntax error! Wrong number of arguments!"
 *
 *                ----If there is a system call error, your shell should
 *                invoke perror("cd")
 *
 *       --our_pwd:
 *
 *          This stands for "our pwd", which prints the working
 *          directory to stdout, and has exit status 0 if successful and
 *          non-zero otherwise. this builtin takes no arguments besides
 *          itself. Handle errors in analogy with cd. Here, the syntax
 *          error message should be:
 *
 *              "pwd: Syntax error! Wrong number of arguments!"
 *
 *       --exit:
 *
 *          As noted in the lab, exit can take 0 or 1 arguments. If it
 *          is given zero arguments (besides itself), then the shell
 *          exits with command status 0. If it is given one numerical
 *          argument, the shell exits with that numerical argument. If it
 *          is given one non-numerical argument, do something sensible.
 *          If it is given more than one argument, print an error message,
 *          and do not exit.
 *
 *
 *   Implementation hints are given in the function body.
 */
static pid_t
cmd_exec(command_t *cmd, int *pass_pipefd)
{
    pid_t pid = -1;
    int pipefd[2];

    // 1️⃣ Create a pipe if this command is the left-hand side of a pipe
    if (cmd->controlop == CMD_PIPE) {
        if (pipe(pipefd) < 0) {
            perror("pipe");
            return -1;
        }
    } else {
        pipefd[0] = pipefd[1] = -1;
    }

    // 2️⃣ Fork the child
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // ---------- CHILD ----------

        // If reading from previous pipe
        if (*pass_pipefd != STDIN_FILENO) {
            dup2(*pass_pipefd, STDIN_FILENO);
            close(*pass_pipefd);
        }

        // If writing to next pipe
        if (cmd->controlop == CMD_PIPE) {
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
        }

        // ---------- Redirections ----------
        if (cmd->redirect_filename[STDIN_FILENO]) {
            int fd = open(cmd->redirect_filename[STDIN_FILENO], O_RDONLY);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (cmd->redirect_filename[STDOUT_FILENO]) {
            int fd = open(cmd->redirect_filename[STDOUT_FILENO],
                          O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (cmd->redirect_filename[STDERR_FILENO]) {
            int fd = open(cmd->redirect_filename[STDERR_FILENO],
                          O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        // ---------- Subshell ----------
        if (cmd->subshell) {
            int status = cmd_line_exec(cmd->subshell);
            exit(status == 0 ? 0 : 5);
        }

        // ---------- Null command ----------
        if (!cmd->argv[0]) exit(0);

        // ---------- Built-ins ----------
        if (strcmp(cmd->argv[0], "cd") == 0) {
            if (!cmd->argv[1] || cmd->argv[2]) {
                fprintf(stderr, "cd: Syntax error! Wrong number of arguments!\n");
                exit(1);
            }
            if (chdir(cmd->argv[1]) < 0) {
                perror("cd");
                exit(1);
            }
            exit(0);
        }

        if (strcmp(cmd->argv[0], "our_pwd") == 0) {
            if (cmd->argv[1]) {
                fprintf(stderr, "pwd: Syntax error! Wrong number of arguments!\n");
                exit(1);
            }
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd))) {
                printf("%s\n", cwd);
                exit(0);
            } else {
                perror("pwd");
                exit(1);
            }
        }

        if (strcmp(cmd->argv[0], "exit") == 0) {
            int code = 0;
            if (cmd->argv[1]) code = atoi(cmd->argv[1]);
            exit(code);
        }

        // ---------- External command ----------
        execvp(cmd->argv[0], cmd->argv);
        perror(cmd->argv[0]);
        exit(1);
    }

    // ---------- PARENT ----------
    if (pid > 0) {
        // 1️⃣ Handle "cd" in parent so it persists across commands
        if (cmd->argv[0] && strcmp(cmd->argv[0], "cd") == 0) {
            if (cmd->argv[1] && !cmd->argv[2]) {
                chdir(cmd->argv[1]);  // silently ignore errors in parent
            }
        }

        // 2️⃣ Handle "exit" in parent so shell terminates
        if (cmd->argv[0] && strcmp(cmd->argv[0], "exit") == 0) {
            int code = 0;
            if (cmd->argv[1]) code = atoi(cmd->argv[1]);
            exit(code);
        }

        // 3️⃣ Close write end of pipe (if any)
        if (cmd->controlop == CMD_PIPE) {
            close(pipefd[1]);
        }

        // 4️⃣ Close previous pipe read end if not STDIN
        if (*pass_pipefd != STDIN_FILENO) {
            close(*pass_pipefd);
        }

        // 5️⃣ Set up next stage of pipeline
        if (cmd->controlop == CMD_PIPE) {
            *pass_pipefd = pipefd[0];
        } else {
            *pass_pipefd = STDIN_FILENO;
        }
    }

    return pid;
}



/* cmd_line_exec(cmdlist)
 *
 *   Execute the command list.
 *
 *   Execute each individual command with 'cmd_exec'.
 *   String commands together depending on the 'cmdlist->controlop' operators.
 *   Returns the exit status of the entire command list, which equals the
 *   exit status of the last completed command.
 *
 *   The operators have the following behavior:
 *
 *      CMD_END, CMD_SEMICOLON
 *                        Wait for command to exit.  Proceed to next command
 *                        regardless of status.
 *      CMD_AND           Wait for command to exit.  Proceed to next command
 *                        only if this command exited with status 0.  Otherwise
 *                        exit the whole command line.
 *      CMD_OR            Wait for command to exit.  Proceed to next command
 *                        only if this command exited with status != 0.
 *                        Otherwise exit the whole command line.
 *      CMD_BACKGROUND, CMD_PIPE
 *                        Do not wait for this command to exit.  Pretend it
 *                        had status 0, for the purpose of returning a value
 *                        from cmd_line_exec.
 */
int
cmd_line_exec(command_t *cmdlist)
{
    int cmd_status = 0;           // exit status of last command
    int pipefd = STDIN_FILENO;    // read end of previous pipe

    while (cmdlist) {
        int wp_status;
        pid_t pid = cmd_exec(cmdlist, &pipefd);
        if (pid < 0)
            abort();  // something went wrong inside cmd_exec

        // Decide whether to wait for this process
        if (cmdlist->controlop != CMD_BACKGROUND &&
            cmdlist->controlop != CMD_PIPE) {
            // wait for it to finish
            waitpid(pid, &wp_status, 0);
            cmd_status = WIFEXITED(wp_status)
                ? WEXITSTATUS(wp_status)
                : 1;
            } else {
                // background/piped: don’t wait
                cmd_status = 0;
            }

        // ✅ Handle "exit" in parent so shell terminates
        if (cmdlist->argv[0] && strcmp(cmdlist->argv[0], "exit") == 0) {
            exit(cmd_status);
        }

        // conditional logic for && and ||
        if (cmdlist->controlop == CMD_AND && cmd_status != 0)
            break; // stop if failed
        if (cmdlist->controlop == CMD_OR && cmd_status == 0)
            break; // stop if succeeded

        cmdlist = cmdlist->next;
    }

    // reap any background processes
    while (waitpid(0, 0, WNOHANG) > 0)
        ;

    return cmd_status;
}
