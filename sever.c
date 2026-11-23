#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

struct Packet
{
    uint32_t packet_id;
    char source[20];
    char destination[20];
    char payload[100];
};

/* Send all bytes, even if send() sends only part of the data */
int send_all(int fd, const void *buffer, size_t length)
{
    const char *ptr = buffer;
    size_t total = 0;

    while (total < length)
    {
        ssize_t sent = send(fd, ptr + total, length - total, 0);

        if (sent <= 0)
            return -1;

        total += (size_t)sent;
    }

    return 0;
}

/* Receive all bytes required for the structure */
int recv_all(int fd, void *buffer, size_t length)
{
    char *ptr = buffer;
    size_t total = 0;

    while (total < length)
    {
        ssize_t received = recv(fd, ptr + total, length - total, 0);

        if (received <= 0)
            return -1;

        total += (size_t)received;
    }

    return 0;
}

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    struct Packet pkt;

    /* 1. Create TCP socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    /* 2. Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    /* 3. Bind socket to IP address and port */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    /* 4. Listen for incoming connections */
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("TCP Server listening on port 8080...\n");

    /* 5. Accept client connection */
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd < 0)
    {
        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("Client connected successfully!\n");

    /* 6. Receive packet from client */
    if (recv_all(client_fd, &pkt, sizeof(pkt)) < 0)
    {
        printf("Failed to receive packet\n");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    /* Ensure strings are terminated */
    pkt.source[sizeof(pkt.source) - 1] = '\0';
    pkt.destination[sizeof(pkt.destination) - 1] = '\0';
    pkt.payload[sizeof(pkt.payload) - 1] = '\0';

    printf("\n--- Received Packet ---\n");
    printf("Packet ID   : %u\n", pkt.packet_id);
    printf("Source      : %s\n", pkt.source);
    printf("Destination : %s\n", pkt.destination);
    printf("Payload     : %s\n", pkt.payload);

    /* 7. Prepare acknowledgment */
    struct Packet response;

    memset(&response, 0, sizeof(response));

    response.packet_id = pkt.packet_id;
    strcpy(response.source, "Server");
    strcpy(response.destination, pkt.source);
    strcpy(response.payload, "Message received successfully!");

    /* 8. Send acknowledgment to client */
    if (send_all(client_fd, &response, sizeof(response)) < 0)
    {
        perror("send");
    }
    else
    {
        printf("\nAcknowledgment sent to client.\n");
    }

    /* 9. Close sockets */
    close(client_fd);
    close(server_fd);

    return 0;
}