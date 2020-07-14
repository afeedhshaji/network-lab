#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <math.h>
#define SA struct sockaddr
#define PORT "9035"
#define MAXDATALEN 500
#define MAX_LINE 4096
#define MAXDATASIZE 100

void *get_in_addr(SA *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void sendFile(FILE *fp, int sockfd)
{
    int n;
    int i = 0;
    int total = 0;
    char buff[MAX_LINE] = {0};

    while (1)
    {
        n = fread(buff, 1, MAX_LINE, fp);
        printf("[+]Size read from file : %d\n", n);
        if (n < MAX_LINE)
        {
            total += n;
            send(sockfd, buff, n, 0);
            break;
        }
        send(sockfd, buff, n, 0);
        bzero(buff, MAX_LINE);

        total += n;
    }
    printf("[+]Finished. NumBytes sent: %d\n", total);
}

void recFile(int sockfd, FILE *fp)
{
    int n;
    int i;
    int count = 0;
    int total = 0;
    char buff[MAX_LINE] = {0};
    while (1)
    {
        n = recv(sockfd, buff, MAX_LINE, 0);
        printf("[+]Size written to file : %d\n", n);
        fwrite(buff, 1, n, fp);
        bzero(buff, MAX_LINE);
        if (n < MAX_LINE)
        {
            total += n;
            break;
        }
        total += n;
    }
    printf("[+]Finished. NumBytes recieved: %d\n", total);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    char message[MAXDATASIZE];
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    struct pollfd pfds[2];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((SA *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);

    pfds[0].fd = 0;
    pfds[0].events = POLLIN;

    pfds[1].fd = sockfd;
    pfds[1].events = POLLIN;

    for (;;)
    {
        int poll_status = poll(pfds, 2, 10000);
        if (pfds[1].revents & POLLIN)
        {
            int numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
            if (numbytes <= 0)
            {
                if (numbytes == 0)
                {
                    printf("Server Disconnected\n");
                }
                else
                {
                    perror("recv");
                }

                close(sockfd);
                exit(0);
            }
            else
            {
                buf[numbytes] = '\0';
                printf("Message recieved : %s", buf);
                if ((strncmp(buf, "send_file", 9)) == 0)
                {
                    FILE *fp;
                    fp = fopen("out.txt", "w");
                    recFile(sockfd, fp);
                    fclose(fp);
                }
                bzero(buf, sizeof(buf));
            }
        }

        if (pfds[0].revents & POLLIN)
        {
            int nbytes = read(0, message, MAXDATASIZE);
            message[nbytes - 1] = '\0';

            if ((strncmp(message, "send_file", 9)) == 0)
            {
                send(sockfd, message, strlen(message), 0);
                char file_path[20];
                strcpy(file_path, message + 10);
                printf("Filename - %s\n", file_path);
                FILE *fp;
                fp = fopen(file_path, "r");
                sendFile(fp, sockfd);
                fclose(fp);
            }
            else
            {
                printf("Sending message:%s\n", message);
                int msg_len = strlen(message);
                message[msg_len] = '\n';
                message[msg_len + 1] = '\0';

                send(sockfd, message, strlen(message), 0);
            }
            bzero(message, sizeof(message));
        }
    }

    close(sockfd);

    return 0;
}