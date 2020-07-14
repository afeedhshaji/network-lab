#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#define MAX 80
#define PORT 8081
#define MAX_LINE 512
#define SA struct sockaddr
ssize_t total = 0;
#define TAG_LEN 3

void randomString(char *buff)
{
    static const int a = 'a', z = 'z';
    int iseed, i;

    iseed = atoi("networkinglab");
    if (!iseed)
        iseed = (unsigned int)time(NULL);

    srand(iseed);
    char str[MAX];
    for (i = 0; i < TAG_LEN; i++)
        str[i] = rand() % (z - a + 1) + a;
    str[i] = '\n';
    str[++i] = '\0';
    strcat(buff, str);
}

void sendFile(FILE *fp, int sockfd)
{
    int n;
    int i = 0;
    char sendline[MAX_LINE] = {0};

    while (1)
    {
        n = fread(sendline, 1, MAX_LINE, fp);
        printf("Size read from file : %d\n", n);
        if (n < MAX_LINE)
        {
            total += n;
            send(sockfd, sendline, n, 0);
            break;
        }
        send(sockfd, sendline, n, 0);
        bzero(sendline, MAX_LINE);

        total += n;
    }
    printf("Finished\n");
}

// Function designed for chat between client and server.
void func(int sockfd)
{
    char buff[MAX];
    char test1[MAX] = {'p', 'o'};
    int n;

    // infinite loop for chat
    for (;;)
    {
        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        // printf("recieving from client..\n");
        read(sockfd, buff, sizeof(buff));
        printf("From client: %s", buff);
        if (strncmp("video", buff, 5) == 0)
        {
            char *filename = "demo.mp4";

            FILE *fp = fopen(filename, "rb");
            if (fp == NULL)
            {
                perror("Can't open file");
                exit(1);
            }

            printf("Start receive file: %s\n", filename);
            sendFile(fp, sockfd);
            printf("Send Success, NumBytes = %ld\n", total);
            fclose(fp);
            // break;
            // sleep(10);
            // write(sockfd, test1, sizeof(test1));
        }
        // if msg contains "Exit" then server exit and chat ended.
        else if (strncmp("exit", buff, 4) == 0)
        {
            printf("Server Exit...\n");
            break;
        }
        else
        {
            // print buffer which contains the client contents
            bzero(buff, MAX);
            n = 0;
            // copy server message in the buffer
            char temp[MAX] = "";
            randomString(temp);

            printf("\tTo client: %s", temp);

            // and send that buffer to client
            write(sockfd, temp, sizeof(temp));
        }
    }
}

// Driver function
int main()
{
    int sockfd, connfd, len;
    int opt = 1;

    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0)
    {
        printf("server acccept failed...\n");
        exit(0);
    }
    else
        printf("server acccept the client...\n");

    // Function for chatting between client and server
    func(connfd);

    // After chatting close the socket
    close(sockfd);
}