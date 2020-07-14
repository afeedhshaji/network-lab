#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>

#define HOST "127.0.0.1"
#define PORT "8080"

struct pkt
{
    int ACK;
    int seqnum;
    char data[25];
};

int main(int argc, char *argv[])
{
    int senderSocket, i, n;
    char input[25];
    struct timeval timeout = {2, 0}; //set timeout for 2 seconds
    int recvlen;
    struct sockaddr_in receiverAddr, senderAddr;
    socklen_t addr_size;
    struct hostent *host;
    int counter = 0;

    host = gethostbyname(HOST);

    senderSocket = socket(AF_INET, SOCK_DGRAM, 0); //using UDP sockets
    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(atoi(PORT));
    receiverAddr.sin_addr = *((struct in_addr *)host->h_addr);

    memset(receiverAddr.sin_zero, '\0', sizeof receiverAddr.sin_zero);

    addr_size = sizeof receiverAddr;

    printf("\nEnter a string to send, q to quit\n ");

    while (1)
    {
        int err;
        struct pkt packet, recvPacket;

        scanf("%s", input);

        if (strcmp("q", input) == 0)
        {
            printf("Quiting\n");
            return 0;
        }

        strcpy(packet.data, input);
        packet.seqnum = counter;

        sendto(senderSocket, &packet, sizeof(packet), 0, (struct sockaddr *)&receiverAddr, addr_size);

        printf("Sent pkt with seq number %d\n", packet.seqnum);

        /* set receive UDP message timeout */
        setsockopt(senderSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

        recvlen = recvfrom(senderSocket, &recvPacket, sizeof(recvPacket), 0, (struct sockaddr *)&receiverAddr, &addr_size);
        while (1)
        {
            if (recvlen >= 0)
            {
                // Message Received
                if (recvPacket.ACK == counter + 1)
                {
                    printf("Ack received for %d\n\n", recvPacket.ACK);
                    counter++;
                    break;
                }
                else
                { // won't be sent out of order, this is unneccesary
                    printf("Resending pkt %d:%s\n", packet.seqnum, packet.data);
                    sendto(senderSocket, &packet, sizeof(packet), 0, (struct sockaddr *)&receiverAddr, addr_size);
                    recvlen = recvfrom(senderSocket, &recvPacket, sizeof(recvPacket), 0, (struct sockaddr *)&receiverAddr, &addr_size);
                }
            }
            else
            {
                //Message Receive Timeout
                printf("Timeout! Resending pkt %d:%s\n", packet.seqnum, packet.data);
                sendto(senderSocket, &packet, sizeof(packet), 0, (struct sockaddr *)&receiverAddr, addr_size);
                recvlen = recvfrom(senderSocket, &recvPacket, sizeof(recvPacket), 0, (struct sockaddr *)&receiverAddr, &addr_size);
            }
        }
    }
    return 0;
}