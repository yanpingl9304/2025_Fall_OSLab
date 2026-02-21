#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../include/command.h"
#include "../include/builtin.h"

// ======================= requirement 2.3 =======================
/**
 * @brief 
 * Redirect command's stdin and stdout to the specified file descriptor
 * If you want to implement ( < , > ), use "in_file" and "out_file" included the cmd_node structure
 * If you want to implement ( | ), use "in" and "out" included the cmd_node structure.
 *
 * @param p cmd_node structure
 * 
 */
void redirection(struct cmd_node *p)
{

    // in <
    if (p->in_file) {
        int fd = open(p->in_file, O_RDONLY);
        if (fd < 0) {
            perror("open <");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    // out >
    if (p->out_file) {
        int fd = open(p->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open >");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    // pipe in 
    if (p->in != 0 && p->in != -1) {
        dup2(p->in, STDIN_FILENO);
        close(p->in);
    }

    // pipe out
    if (p->out != 1 && p->out != -1) {
        dup2(p->out, STDOUT_FILENO);
        close(p->out);
    }
}

// ===============================================================

// ======================= requirement 2.2 =======================
/**
 * @brief 
 * Execute external command
 * The external command is mainly divided into the following two steps:
 * 1. Call "fork()" to create child process
 * 2. Call "execvp()" to execute the corresponding executable file
 * @param p cmd_node structure
 * @return int 
 * Return execution status
 */
int spawn_proc(struct cmd_node *p)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child
        redirection(p);
        execvp(p->args[0], p->args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    // Parent
    int status;
    waitpid(pid, &status, 0);

    return 1;
}

// ===============================================================


// ======================= requirement 2.4 =======================
/**
 * @brief 
 * Use "pipe()" to create a communication bridge between processes
 * Call "spawn_proc()" in order according to the number of cmd_node
 * @param cmd Command structure  
 * @return int
 * Return execution status 
 */
int fork_cmd_node(struct cmd *cmd)
{

    // 指令數量
    int count = 0;
    for (struct cmd_node *t = cmd->head; t; t = t->next)
        count++;

    int pipes[count - 1][2];
    pid_t pids[count];
      
    // 建立 pipe
    for (int i = 0; i < count - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    int idx = 0;

    for (struct cmd_node *cur = cmd->head; cur; cur = cur->next, idx++)
    {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            // ---------- Child ----------

            // 不是第一個指令 stdin = 上一個 pipe 的 read 
            if (idx > 0) {
                dup2(pipes[idx - 1][0], STDIN_FILENO);
            }

            // 不是最後一個指令 stdout = 下一個 pipe 的 write 
            if (idx < count - 1) {
                dup2(pipes[idx][1], STDOUT_FILENO);
            }

            // 關閉所有 pipe
            for (int i = 0; i < count - 1; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            execvp(cur->args[0], cur->args);
            perror("execvp");
            exit(1);
        }

        pids[idx] = pid;
    }

    // 關閉所有 pipe
    for (int i = 0; i < count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // 等 child
    for (int i = 0; i < count; i++) {
        waitpid(pids[i], NULL, 0);
    }

    return 1;
}

// ===============================================================


void shell()
{
	while (1) {
		printf(">>> $ ");
		char *buffer = read_line();
		if (buffer == NULL)
			continue;

		struct cmd *cmd = split_line(buffer);
		
		int status = -1;
		// only a single command
		struct cmd_node *temp = cmd->head;
		
		if(temp->next == NULL){
			status = searchBuiltInCommand(temp);
			if (status != -1){
				int in = dup(STDIN_FILENO), out = dup(STDOUT_FILENO);
				if( in == -1 | out == -1)
					perror("dup");
				redirection(temp);
				status = execBuiltInCommand(status,temp);

				// recover shell stdin and stdout
				if (temp->in_file)  dup2(in, 0);
				if (temp->out_file){
					dup2(out, 1);
				}
				close(in);
				close(out);
			}
			else{
				//external command
				status = spawn_proc(cmd->head);
			}
		}
		// There are multiple commands ( | )
		else{
			status = fork_cmd_node(cmd);
		}
		// free space
		while (cmd->head) {
			
			struct cmd_node *temp = cmd->head;
      		cmd->head = cmd->head->next;
			free(temp->args);
   	    	free(temp);
   		}
		free(cmd);
		free(buffer);
		
		if (status == 0)
			break;
	}
}
