#include "socket_utils.h"

void print_error(int result, char *s)
{
    if (result < 0)
    {
        perror(s);
        // exit(EXIT_FAILURE);
    }
}

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

void send_message(int fd, char *message, int size, int flag)
{
    write_int_as_message(fd, size);
    printf("Message size: %d\n", size);
    write_on_socket(fd, message);
}

void receive_message(int fd, char *buffer, int size, int flag)
{
    int message_size = read_int_from_socket(fd);
    printf("Message size: %d\n", message_size);

    int read_status = read_message_from_socket(fd, buffer, message_size);
    if (read_status <= 0)
    {
        printf("Server disconnected or error occurred.\n");
        // exit(EXIT_FAILURE);
    }
    buffer[message_size] = '\0';
}