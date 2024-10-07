#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1" // Adresse IP du serveur
#define SERVER_PORT 8080      // Port du serveur
#define BUFFER_SIZE 2048

int sockfd; // Socket pour la connexion avec le serveur
char name[50];
char gender[10];
int age;

// Fonction pour envoyer des messages au serveur
void *send_message(void *arg)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        // Lire le message de l'utilisateur
        fgets(buffer, BUFFER_SIZE, stdin);
        // Enlever le caractère de nouvelle ligne
        buffer[strcspn(buffer, "\n")] = '\0';

        // Envoyer le message au serveur
        if (send(sockfd, buffer, strlen(buffer), 0) < 0)
        {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }

        // Vérifier si l'utilisateur veut quitter
        if (strcmp(buffer, "/quit") == 0)
        {
            printf("Déconnexion...\n");
            close(sockfd);
            exit(EXIT_SUCCESS);
        }
    }

    return NULL;
}

// Fonction pour recevoir les messages du serveur
void *receive_message(void *arg)
{
    char buffer[BUFFER_SIZE];
    int nbytes;

    while (1)
    {
        // Recevoir un message du serveur
        nbytes = recv(sockfd, buffer, sizeof(buffer), 0);
        if (nbytes <= 0)
        {
            perror("Erreur lors de la réception du message ou connexion fermée");
            exit(EXIT_FAILURE);
        }

        buffer[nbytes] = '\0'; // Ajouter un terminateur de chaîne
        printf("Message reçu: %s\n", buffer);
    }

    return NULL;
}

int main()
{
    struct sockaddr_in server_addr;
    pthread_t send_thread, receive_thread;

    // Créer un socket pour le client
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configurer l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Erreur lors de la connexion au serveur");
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur\n");

    // Saisie des informations de l'utilisateur
    printf("Entrez votre nom: ");
    fgets(name, 50, stdin);
    name[strcspn(name, "\n")] = '\0'; // Retirer le saut de ligne

    printf("Entrez votre sexe (M/F): ");
    fgets(gender, 10, stdin);
    gender[strcspn(gender, "\n")] = '\0'; // Retirer le saut de ligne

    printf("Entrez votre âge: ");
    scanf("%d", &age);
    getchar(); // Nettoyer le buffer d'entrée

    // Envoyer les informations de matchmaking au serveur
    char auth_data[BUFFER_SIZE];
    sprintf(auth_data, "%s %d %s", name, gender, age);
    if (send(sockfd, auth_data, strlen(auth_data), 0) < 0)
    {
        perror("Erreur lors de l'envoi des informations de matchmaking");
        exit(EXIT_FAILURE);
    }

    // Créer un thread pour envoyer des messages
    if (pthread_create(&send_thread, NULL, send_message, NULL) != 0)
    {
        perror("Erreur lors de la création du thread d'envoi");
        exit(EXIT_FAILURE);
    }

    // Créer un thread pour recevoir des messages
    if (pthread_create(&receive_thread, NULL, receive_message, NULL) != 0)
    {
        perror("Erreur lors de la création du thread de réception");
        exit(EXIT_FAILURE);
    }

    // Attendre que les threads se terminent (en théorie, ils tournent à l'infini)
    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    // Fermer la connexion au serveur
    close(sockfd);

    return 0;
}
