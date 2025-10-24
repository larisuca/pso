#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PORT 9000
#define XML_FILE "logs.xml"
#define MAX_CLIENTS 50

pthread_mutex_t file_lock; // protejează scrierea în fișierul comun

void write_log_to_xml(const char *log_msg) {
    pthread_mutex_lock(&file_lock);

    FILE *f = fopen(XML_FILE, "a");
    if (!f) {
        perror("Eroare la deschiderea fisierului XML");
        pthread_mutex_unlock(&file_lock);
        return;
    }

    // Extrage componentele din logul primit
    char timestamp[64], app[64], level[32], message[512];

    // Formatul logului este: [timestamp] [AppX] [LEVEL]: message
    if (sscanf(log_msg, "[%63[^]]] [%63[^]]] [%31[^]]]: %[^\n]",
               timestamp, app, level, message) == 4) {
        fprintf(f,
            "    <log>\n"
            "        <timestamp>%s</timestamp>\n"
            "        <app>%s</app>\n"
            "        <level>%s</level>\n"
            "        <message>%s</message>\n"
            "    </log>\n",
            timestamp, app, level, message);
    } else {
        // fallback dacă formatul nu e recunoscut
        fprintf(f, "    <log raw=\"true\">%s</log>\n", log_msg);
    }

    fclose(f);
    pthread_mutex_unlock(&file_lock);
}
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    // debug: ID thread + timestamp
    printf("[INFO] Thread ID %lu pornit pentru un client nou.\n", pthread_self());

    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(client_socket, buffer, sizeof(buffer) - 1);
        if (valread <= 0)
            break;

        printf("Log primit: %s", buffer);
        write_log_to_xml(buffer);
    }

    printf("[INFO] Thread %lu – Client deconectat.\n", pthread_self());
    close(client_socket);
    return NULL;
}

int main() {
    int server_fd, *new_sock;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    pthread_mutex_init(&file_lock, NULL);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Eroare la crearea socketului");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Eroare la setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Eroare la bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Eroare la listen");
        exit(EXIT_FAILURE);
    }

    printf(" Server (multithreaded) pornit pe portul %d...\n", PORT);

    // Inițializare fișier XML
    FILE *f = fopen(XML_FILE, "w");
    if (f) {
        fprintf(f, "<logs>\n");
        fclose(f);
    }

    // buclă principală – acceptă clienți
    while (1) {
        int client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            perror("Eroare la accept");
            continue;
        }

        printf("Client conectat.\n");

        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, new_sock) != 0) {
            perror("Eroare la crearea thread-ului");
            close(client_socket);
            free(new_sock);
            continue;
        }

        pthread_detach(tid);
    }

    pthread_mutex_destroy(&file_lock);
    close(server_fd);
    return 0;
}
