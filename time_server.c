#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

void signalHandler(int signo) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Child process terminated, pid = %d\n", pid);
    }
}

char* getCurrentTime(char* format) {
    time_t rawtime;
    struct tm * timeinfo;
    char *buffer = (char *)malloc(80 * sizeof(char));

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    if (strcmp(format, "dd/mm/yyyy") == 0) {
        strftime(buffer, 80, "%d/%m/%Y\n", timeinfo);
    } else if (strcmp(format, "dd/mm/yy") == 0) {
        strftime(buffer, 80, "%d/%m/%y\n", timeinfo);
    } else if (strcmp(format, "mm/dd/yyyy") == 0) {
        strftime(buffer, 80, "%m/%d/%Y\n", timeinfo);
    } else if (strcmp(format, "mm/dd/yy") == 0) {
        strftime(buffer, 80, "%m/%d/%y\n", timeinfo);
    } else {
        strcpy(buffer, "Invalid format\n");
    }

    return buffer;
}

int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    signal(SIGCHLD, signalHandler);

    while (1) {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        if (fork() == 0) {
            // Tien trinh con, xu ly yeu cau tu client
            // Dong socket listener
            close(listener);

            char buf[256];
            while (1) {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;

                buf[ret] = 0;
                printf("Received: %s", buf);

                // Xử lý lệnh từ client
                if (strncmp(buf, "GET_TIME", 8) == 0) {
                    char *format = strtok(buf + 9, " \r\n");
                    char *timeStr = getCurrentTime(format);
                    send(client, timeStr, strlen(timeStr), 0);
                    free(timeStr);
                }
                else {
                    char* message = "Invalid command\n";
                    send(client, message, strlen(message), 0);
                }
            }

            // Ket thuc tien trinh con
            close(client);
            exit(0);
        }

        // Dong socket client o tien trinh cha
        close(client);
    }

    return 0;
}
