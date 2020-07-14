#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

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
    int recvSocket, err;
    struct pkt recvPacket, ackPacket;
    struct sockaddr_in recieverAddr, senderAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    int drop = 0;
    int counter = 0;

    recvSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (recvSocket < 0)
        printf("ERROR opening socket\n");

    recieverAddr.sin_family = AF_INET;
    recieverAddr.sin_port = htons(atoi(PORT));
    recieverAddr.sin_addr.s_addr = inet_addr(HOST);

    memset(recieverAddr.sin_zero, '\0', sizeof recieverAddr.sin_zero);

    addr_size = sizeof senderAddr;
    err = bind(recvSocket, (struct sockaddr *)&recieverAddr, addr_size);

    if (err == -1)
    {
        perror("bind error: ");
        exit(0);
    }
    while (1)
    {
        //wait for a packet
        err = recvfrom(recvSocket, &recvPacket, sizeof(recvPacket), 0, (struct sockaddr *)&senderAddr, &addr_size);
        if (err > 0)
        {
            int drop = rand() % 10;
            if (drop < 2)
            {
                //drop pkt occasionally, for testing purposes
                ackPacket.ACK = counter; //last correctly recieved
                printf("Dropping %s, %d\n", recvPacket.data, recvPacket.seqnum);

                continue; //go back to wait for a packet (this one never reached)
            }
            else if (drop < 5)
            {
                //drop ack
                counter;
                printf("Correctly recieved: %s, with seq number :%d ack: %d, but the ack will drop\n", recvPacket.data, recvPacket.seqnum, counter);
                continue;
            }
            else if (recvPacket.seqnum == counter)
            {
                counter++;
                ackPacket.ACK = counter;
                printf("Correctly recieved: %s, with seq number :%d ack: %d\n", recvPacket.data, recvPacket.seqnum, counter);
            }
            else if (recvPacket.seqnum < counter)
            {
                //duplicate old packet
                printf("Duplicate %d, expecting %d\n", recvPacket.seqnum, counter);
                ackPacket.ACK = counter; //reACK
            }
            else
            {
                ackPacket.ACK = counter;
                printf("Out of order %d, expected %d\n", recvPacket.seqnum, counter);
            }

            sendto(recvSocket, &ackPacket, sizeof(ackPacket), 0, (struct sockaddr *)&senderAddr, addr_size);
        }
        else
        {
            perror("recv failed\n");
        }
    }
    return 0;
}