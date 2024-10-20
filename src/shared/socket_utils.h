/**
 * @file socket_utils.h
 * @brief Header file containing utility functions for socket communication.
 *
 * This header file declares functions for sending and receiving messages and integers
 * over a socket, along with error handling utilities.
 */

#ifndef HELPER_H
#define HELPER_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief Prints an error message if the result is negative.
 *
 * Checks the result of a socket operation, and if the result
 * is negative, it prints the corresponding error message using `perror`.
 *
 * @param result The result of the socket operation.
 * @param s A description of the operation to be printed in case of an error.
 */
void print_error(int result, char *s);

/**
 * @brief Reads a message from a socket into a buffer.
 *
 * This function attempts to read `size` bytes of data from the socket
 * into the provided buffer. It reads until the requested number of bytes
 * is read or an error occurs.
 *
 * @param fd The socket file descriptor.
 * @param buffer The buffer where the read data will be stored.
 * @param size The number of bytes to read from the socket.
 * @return The total number of bytes read, or 0 if the connection is closed.
 */
int read_message_from_socket(int fd, char *buffer, int size);

/**
 * @brief Writes a string message to a socket.
 *
 * Writes a string to the specified socket, ensuring that the entire string is sent,
 * even if multiple write operations are required.
 *
 * @param fd The socket file descriptor.
 * @param s The string message to be sent.
 */
void write_on_socket(int fd, char *s);

/**
 * @brief Reads an integer from a socket.
 *
 * Reads an integer value from the socket. This function ensures that the entire
 * integer is read even if multiple read operations are required.
 *
 * @param fd The socket file descriptor.
 * @return The integer value read from the socket.
 */
int read_int_from_socket(int fd);

/**
 * @brief Sends an integer over a socket.
 *
 * Sends an integer value over the specified socket, ensuring that the entire integer
 * is transmitted even if multiple write operations are required.
 *
 * @param fd The socket file descriptor.
 * @param val The integer value to be sent.
 */
void write_int_as_message(int fd, int val);

/**
 * @brief Sends a message over a socket.
 *
 * Sends a message by first writing the message size, followed by the message content
 * itself. It uses the provided socket for communication.
 *
 * @param fd The socket file descriptor.
 * @param message The message to be sent.
 * @param size The size of the message.
 * @param flag A flag (currently unused) for message sending options.
 */
void send_message(int fd, char *message, int size, int flag);

/**
 * @brief Receives a message from a socket.
 *
 * Receives a message by first reading the message size, then reading the message content
 * into the provided buffer. The buffer will be null-terminated after reading.
 *
 * @param fd The socket file descriptor.
 * @param buffer The buffer to store the received message.
 * @param size The expected size of the message.
 * @param flag A flag (currently unused) for message receiving options.
 */
void receive_message(int fd, char *buffer, int size, int flag);

#endif
