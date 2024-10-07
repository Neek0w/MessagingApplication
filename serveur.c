#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

typedef struct {
    int socket;
    char gender[10];
    int age;
    int group_id;
} User;

User clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Ajoute un utilisateur à la liste des clients connectés
void add_user(User user) {
    pthread_mutex_lock(&clients_mutex);
    clients[client_count++] = user;
    pthread_mutex_unlock(&clients_mutex);
}

// Supprime un utilisateur de la liste des clients connectés
void remove_user(int socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    close(socket);  // Fermer le socket après avoir supprimé l'utilisateur
}

// Fonction qui forme des groupes de 3 utilisateurs ayant les mêmes critères
void matchmake_users() {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < client_count; i++) {
        if (clients[i].group_id == -1) { // Pas encore assigné à un groupe
            for (int j = i + 1; j < client_count; j++) {
                if (clients[j].group_id == -1 && strcmp(clients[i].gender, clients[j].gender) == 0 && abs(clients[i].age - clients[j].age) <= 10) {
                    for (int k = j + 1; k < client_count; k++) {
                        if (clients[k].group_id == -1 && strcmp(clients[i].gender, clients[k].gender) == 0 && abs(clients[i].age - clients[k].age) <= 10) {
                            // Assigner à un groupe
                            int group_id = i + 1; // Génération d'un ID de groupe simple
                            clients[i].group_id = group_id;
                            clients[j].group_id = group_id;
                            clients[k].group_id = group_id;
                            printf("Groupe formé: %d avec utilisateurs %d, %d et %d\n", group_id, clients[i].socket, clients[j].socket, clients[k].socket);
                            break;
                        }
                    }
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// Fonction gérant la messagerie instantanée au sein d'un groupe
void *group_chat(void *arg) {
    int sockfd = *((int *)arg);
    free(arg); // Libérer la mémoire allouée pour le socket
    char buffer[BUFFER_SIZE];
    int nbytes;

    while ((nbytes = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        pthread_mutex_lock(&clients_mutex);

        // Rechercher le groupe auquel appartient le client
        int group_id = -1;
        for (int i = 0; i < client_count; i++) {
            if (clients[i].socket == sockfd) {
                group_id = clients[i].group_id;
                break;
            }
        }

        // Diffuser à tous les membres du groupe
        if (group_id != -1) {
            for (int i = 0; i < client_count; i++) {
                if (clients[i].group_id == group_id && clients[i].socket != sockfd) {
                    if (send(clients[i].socket, buffer, nbytes, 0) < 0) {
                        perror("Erreur lors de l'envoi du message au membre du groupe");
                    }
                }
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    remove_user(sockfd);
    pthread_exit(NULL);
}

// Gère la connexion d'un client et reçoit les informations de matchmaking
void *client_handler(void *arg) {
    int sockfd = *((int *)arg);
    free(arg);  // Libérer l'argument alloué dynamiquement

    char buffer[BUFFER_SIZE];
    int nbytes;

    // Authentification (simple pour l'exemple)
    User user;
    user.socket = sockfd;
    user.group_id = -1;  // Pas encore dans un groupe

    // Recevoir les données du formulaire : sexe et âge
    if ((nbytes = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0) {
        perror("Erreur lors de la réception des données de matchmaking");
        remove_user(sockfd);
        pthread_exit(NULL);
    }
    sscanf(buffer, "%s %d", user.gender, &user.age);

    add_user(user);

    // Appeler la fonction de matchmaking
    matchmake_users();

    // Démarrer la messagerie instantanée pour ce client
    pthread_t chat_thread;
    int *sockfd_ptr = malloc(sizeof(int)); // Allouer de la mémoire pour stocker le descripteur de socket
    *sockfd_ptr = sockfd;
    if (pthread_create(&chat_thread, NULL, group_chat, (void *)sockfd_ptr) != 0) {
        perror("Erreur lors de la création du thread de messagerie");
        free(sockfd_ptr); // Libérer la mémoire en cas d'échec
        pthread_exit(NULL);
    }

    pthread_detach(chat_thread);  // Détacher le thread pour libérer les ressources automatiquement
    pthread_exit(NULL);
}

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur lors du bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 10);
    printf("Serveur en écoute sur le port 8080\n");

    while (1) {
        cli_len = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_len);

        if (newsockfd < 0) {
            perror("Erreur lors de l'acceptation de la connexion");
            continue;
        }

        // Créer un nouveau thread pour chaque client
        pthread_t tid;
        int *newsockfd_ptr = malloc(sizeof(int)); // Allouer dynamiquement pour éviter des conflits
        *newsockfd_ptr = newsockfd;
        if (pthread_create(&tid, NULL, client_handler, (void *)newsockfd_ptr) != 0) {
            perror("Erreur lors de la création du thread");
            free(newsockfd_ptr);  // Libérer la mémoire en cas d'échec
            continue;
        }

        pthread_detach(tid);  // Détacher le thread pour ne pas accumuler des threads zombies
    }

    close(sockfd);
    return 0;
}
