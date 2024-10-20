/**
 * @file socket_utils.c
 * @brief Utility functions for socket communication.
 *
 * This file contains utility functions to send and receive messages
 * and integers over a socket, along with error handling.
 */

#include "socket_utils.h"

/**
 * @brief Prints an error message if the result is negative.
 *
 * This function checks the result of a socket operation, and if the result
 * indicates an error (negative value), it prints the corresponding error message.
 *
 * @param result The result of the socket operation.
 * @param s A string that describes the operation (used in the error message).
 */
void print_error(int result, char *s)
{
    if (result < 0)
    {
        perror(s);
        // exit(EXIT_FAILURE);
    }
}


/**
 * @brief Reads a message from a socket.
 *
 * This function attempts to read `size` bytes from a socket into the provided buffer.
 * It continues reading until the desired number of bytes is read or an error occurs.
 *
 * @param fd The socket file descriptor.
 * @param buffer The buffer to store the read data.
 * @param size The number of bytes to read.
 * @return The total number of bytes read, or 0 if the connection is closed.
 */
int read_message_from_socket(int fd, char *buffer, int size)
{
    int total_bytes_read = 0;
    int bytes_read;

    while (total_bytes_read < size)
    {
        bytes_read = read(fd, buffer + total_bytes_read, size - total_bytes_read);
        if (bytes_read == 0)
            return 0;
        print_error(bytes_read, "read_message");
        total_bytes_read += bytes_read;
    }

    return total_bytes_read;
}


/**
 * @brief Writes a string message to a socket.
 *
 * This function writes a string message to the specified socket.
 * It ensures that the entire string is written, even if multiple write
 * operations are required.
 *
 * @param fd The socket file descriptor.
 * @param s The string message to send.
 */
void write_on_socket(int fd, char *s)
{
    int size = strlen(s);
    int send = 0;

    while (send < size)
    {
        int temp_send = write(fd, s + send, size - send);
        print_error(temp_send, "write");
        send += temp_send;
    }
}


/**
 * @brief Reads an integer from a socket.
 *
 * This function reads an integer value from the specified socket. It ensures
 * that the entire integer is read, even if multiple read operations are required.
 *
 * @param fd The socket file descriptor.
 * @return The integer value read from the socket.
 */
int read_int_from_socket(int fd)
{
    int val;
    int total_bytes_read = 0;
    int bytes_read;
    int size = sizeof(val);

    while (total_bytes_read < size)
    {
        bytes_read = read(fd, ((char *)&val) + total_bytes_read, size - total_bytes_read);
        if (bytes_read == 0)
            return 0;

        print_error(bytes_read, "read_int");
        total_bytes_read += bytes_read;
    }

    return val;
}


/**
 * @brief Writes an integer as a message to a socket.
 *
 * This function sends an integer value over the specified socket. It ensures
 * that the entire integer is written, even if multiple write operations are required.
 *
 * @param fd The socket file descriptor.
 * @param val The integer value to send.
 */
void write_int_as_message(int fd, int val)
{
    int size = sizeof(val);
    int send = 0;

    while (send < size)
    {
        int temp_send = write(fd, ((char *)&val) + send, size - send);
        print_error(temp_send, "write_int");
        send += temp_send;
    }
}


/**
 * @brief Sends a message over a socket.
 *
 * This function sends a message over a socket by first writing the size of the message
 * and then the message content itself.
 *
 * @param fd The socket file descriptor.
 * @param message The message to send.
 * @param size The size of the message.
 * @param flag A flag (currently unused) for message sending options.
 */
void send_message(int fd, char *message, int size, int flag)
{
    write_int_as_message(fd, size);
    // printf("Message size: %d\n", size);
    write_on_socket(fd, message);
}


/**
 * @brief Receives a message from a socket.
 *
 * This function receives a message from a socket by first reading the size of the message,
 * then reading the message content into the provided buffer.
 *
 * @param fd The socket file descriptor.
 * @param buffer The buffer to store the received message.
 * @param size The expected size of the message.
 * @param flag A flag (currently unused) for message receiving options.
 */
void receive_message(int fd, char *buffer, int size, int flag)
{
    int message_size = read_int_from_socket(fd);
    // printf("Message size: %d\n", message_size);

    int read_status = read_message_from_socket(fd, buffer, message_size);
    if (read_status <= 0)
    {
        printf("Server disconnected or error occurred.\n");
        // exit(EXIT_FAILURE);
    }
    buffer[message_size] = '\0';
}