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

/* Send all bytes */
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

/* Receive all bytes */
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
    int client_fd;
    struct sockaddr_in server_addr;
    struct Packet pkt;
    struct Packet response;

    /* 1. Create TCP socket */
    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd < 0)
    {
        perror("socket");
        return 1;
    }

    /* 2. Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    /* 3. Connect to server */
    if (connect(client_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(client_fd);
        return 1;
    }

    printf("Connected to TCP Server!\n");

    /* 4. Prepare packet */
    memset(&pkt, 0, sizeof(pkt));

    pkt.packet_id = 1001;

    strcpy(pkt.source, "Client");
    strcpy(pkt.destination, "Server");
    strcpy(pkt.payload, "Hello Server! This is a TCP client.");

    printf("\n--- Sending Packet ---\n");
    printf("Packet ID   : %u\n", pkt.packet_id);
    printf("Source      : %s\n", pkt.source);
    printf("Destination : %s\n", pkt.destination);
    printf("Payload     : %s\n", pkt.payload);

    /* 5. Send packet to server */
    if (send_all(client_fd, &pkt, sizeof(pkt)) < 0)
    {
        perror("send");
        close(client_fd);
        return 1;
    }

    printf("\nPacket sent successfully!\n");

    /* 6. Receive server acknowledgment */
    if (recv_all(client_fd, &response, sizeof(response)) < 0)
    {
        printf("Failed to receive response\n");
        close(client_fd);
        return 1;
    }

    response.source[sizeof(response.source) - 1] = '\0';
    response.destination[sizeof(response.destination) - 1] = '\0';
    response.payload[sizeof(response.payload) - 1] = '\0';

    /* 7. Display server response */
    printf("\n--- Server Response ---\n");
    printf("Packet ID   : %u\n", response.packet_id);
    printf("Source      : %s\n", response.source);
    printf("Destination : %s\n", response.destination);
    printf("Payload     : %s\n", response.payload);

    /* 8. Close socket */
    close(client_fd);

    return 0;
}