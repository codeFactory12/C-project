#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080
#define MAX_CONNECTIONS 35

void handle_sigint(int sig);
void *client_handler(void *arg);
void get_info(int client_socket);
void get_number_of_partitions(int client_socket);
void get_current_kernel_version(int client_socket);
void is_sshd_running(int client_socket);

int server_socket;

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket;
    pthread_t tid;

    signal(SIGINT, handle_sigint);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        if (pthread_create(&tid, NULL, client_handler, (void*)&client_socket) != 0) {
            perror("Thread creation failed");
        }
    }

    close(server_socket);
    return 0;
}

void handle_sigint(int sig) {
    printf("Caught signal %d, terminating...\n", sig);
    close(server_socket);
    exit(EXIT_SUCCESS);
}

void *client_handler(void *arg) {
    int client_socket = *(int*)arg;
    char buffer[1024];
    int bytes_read;

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Received command: %s\n", buffer);

        if (strcmp(buffer, "get info\n") == 0) {
            get_info(client_socket);
        } else if (strcmp(buffer, "get number of partitions\n") == 0) {
            get_number_of_partitions(client_socket);
        } else if (strcmp(buffer, "get current kernel version\n") == 0) {
            get_current_kernel_version(client_socket);
        } else if (strcmp(buffer, "sshd running\n") == 0) {
            is_sshd_running(client_socket);
        } else {
            write(client_socket, "Invalid command\n", 16);
        }
    }

    close(client_socket);
    return NULL;
}

void get_info(int client_socket) {
    const char *info = "Service Name: ExampleService\nVersion: 1.0.0\n";
    write(client_socket, info, strlen(info));
}

void get_number_of_partitions(int client_socket) {
    FILE *fp = popen("lsblk -l | grep part | wc -l", "r");
    if (fp == NULL) {
        write(client_socket, "Error executing command\n", 24);
        return;
    }
    char result[128];
    fgets(result, sizeof(result), fp);
    pclose(fp);
    write(client_socket, result, strlen(result));
}

void get_current_kernel_version(int client_socket) {
    FILE *fp = popen("uname -r", "r");
    if (fp == NULL) {
        write(client_socket, "Error executing command\n", 24);
        return;
    }
    char result[128];
    fgets(result, sizeof(result), fp);
    pclose(fp);
    write(client_socket, result, strlen(result));
}

void is_sshd_running(int client_socket) {
    if (access("/var/run/sshd.pid", F_OK) == 0) {
        write(client_socket, "true\n", 5);
    } else {
        write(client_socket, "false\n", 6);
    }
}
