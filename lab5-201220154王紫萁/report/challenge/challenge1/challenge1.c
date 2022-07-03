#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#define MAX_BUF_LEN 512

int main() {
    char buffer[MAX_BUF_LEN] = { 0 };
    int pipefd[2];
    pid_t id = getpid();
    printf("father pid: %d\n", id);
    pipe(pipefd);
    int recoverfd = dup(STDOUT_FILENO);
    dup2(pipefd[1], STDOUT_FILENO);
    system("ls");
    read(pipefd[0], buffer, MAX_BUF_LEN);
    dup2(recoverfd, STDOUT_FILENO);
    printf("ls print in pid <%d>: %s", getpid(), buffer);
}
