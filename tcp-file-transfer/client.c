#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#define MAX 80
#define PORT 8081
#define SA struct sockaddr
#define MAX_LINE 512
#define SA struct sockaddr
ssize_t total = 0;
ssize_t chunk = 0;
#define TIME_SLOT 0.1

void recFile(int sockfd, FILE *fp)
{
    int n;
    int i;
    int count = 0;
    char buff[MAX_LINE] = {0};
    clock_t begin, end;
    begin = clock();
    double plots[MAX];
    while (1)
    {
        n = recv(sockfd, buff, MAX_LINE, 0);
        chunk += n;
        end = clock() - begin;
        double time_taken = ((double)end) / CLOCKS_PER_SEC; // calculate the elapsed time
        if (time_taken >= 0.1)
        {
            plots[count++] = chunk;
            begin = clock(); // Reset clock
            chunk = 0;       // Reset chunk
        }

        printf("Size written to file : %d\n", n);
        fwrite(buff, 1, n, fp);
        bzero(buff, MAX_LINE);
        if (n < MAX_LINE)
        {
            total += n;
            break;
        }
        total += n;
    }
    // Plot function
    char *commandsForGNUPlot[] = {"set title \"RATE MB/0.01 sec\"", "plot 'data.temp'"};
    FILE *temp = fopen("data.temp", "w");
    FILE *gnuplotPipe = popen("gnuplot -persistent", "w");

    for (i = 0; i < count; i++)
    {
        fprintf(temp, "%lf %lf \n", TIME_SLOT * i, plots[i] / 1000000);
    }
    fflush(temp);
    for (i = 0; i < 2; i++)
    {
        fprintf(gnuplotPipe, "%s \n", commandsForGNUPlot[i]);
    }
    fflush(gnuplotPipe);

    printf("Finished\n");
}

void func(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;)
    {
        bzero(buff, sizeof(buff));
        printf("\tEnter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        if ((strncmp(buff, "video", 5)) == 0)
        {
            write(sockfd, buff, sizeof(buff));
            // printf("Recieving Video\n");
            char *filename = "output.mp4";

            FILE *fp = fopen(filename, "wb");
            if (fp == NULL)
            {
                perror("Can't open file");
                exit(1);
            }

            printf("Start receive file: %s\n", filename);
            recFile(sockfd, fp);
            printf("Receive Success, NumBytes = %ld\n", total);
            fclose(fp);
            // break;
        }
        else if ((strncmp(buff, "exit", 4)) == 0)
        {
            write(sockfd, buff, sizeof(buff));
            printf("Client Exit...\n");
            break;
        }
        else
        {
            write(sockfd, buff, sizeof(buff));
            bzero(buff, sizeof(buff));
            read(sockfd, buff, sizeof(buff));
            printf("From Server : %s", buff);
        }
    }
}

int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);
}