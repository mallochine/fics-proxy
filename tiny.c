// vim: set et ts=4 sts=4 sw=4:
#include "csapp.h"
#include <pthread.h>

typedef struct route {
    int fromfd;
    int tofd;
} route;

void doit(int fd);

int main(int argc, char **argv) 
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);

    puts("INFO: setting SIGPIPE to SIG_IGN");
    signal(SIGPIPE, SIG_IGN);

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);

        printf("INFO: JARVIS is listening on port %d...\n", port);
        fflush(stdout);

        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        doit(connfd);
        Close(connfd);
    }
}
/* $end tinymain */

/* Returns NULL, routes bytes in args->fromfd to args->tofd.
 */
void *route_to(void *args) {
    pthread_detach(pthread_self());

    route *R = (route *) args;
    printf("INFO: route_to was called\n");
    printf("INFO: fromfd is %d\n", R->fromfd);
    printf("INFO: tofd is %d\n", R->tofd);
    fflush(stdout);

    char buf[MAXLINE];
    int numread;
    while ((numread = recv(R->fromfd, buf, 1, 0)) > 0) {
        // To turn on DEBUG mode, uncomment the following line
        //putchar(buf[0]);
        Rio_writen(R->tofd, buf, numread);
    }

    printf("INFO: exiting route_to...\n");

    return NULL;
}

void *routeClientToFICS(void *args) {
    pthread_detach(pthread_self());

    route *R = (route *) args;
    printf("INFO: route_to was called\n");
    printf("INFO: fromfd is %d\n", R->fromfd);
    printf("INFO: tofd is %d\n", R->tofd);
    fflush(stdout);

    // Put the client bytes into the logs
//    FILE *client_logs;
//    client_logs = fopen("./logs.txt","wb");

    char buf[MAXLINE];
    int numread;
    while ((numread = recv(R->fromfd, buf, 1, 0)) > 0) {
        // To turn on DEBUG mode, uncomment the following line
        //putchar(buf[0]);
        Rio_writen(R->tofd, buf, numread);
//        fputc(buf[0], client_logs);
    }

//    fclose(client_logs);

    printf("INFO: exiting route_to...\n");

    return NULL;
}

void authenticateClient(int clientfd, int serverfd) {
    char buf[2];
    int numread;

    puts("INFO: Flushing out the initial key that the client sends...");
    while ((numread = recv(clientfd, buf, 1, 0)) > 0) {
        // Simply printing out what the client sent, nothing else.
        //putchar(buf[0]);

        // The end of the message is with byte '\x80', so break the wihle loop
        // when we get End Of Message
        if (buf[0] == '\x0a')
            break;
    }

    FILE *login_key;

    puts("INFO: Giving login_key.txt to FICS...");
    login_key = fopen("../keys/login_key.txt", "r");
    if (login_key) {
        while ((buf[0] = fgetc(login_key)) != EOF) {
            write(serverfd, buf, 1);
            //putchar(buf[0]);
        }
        fclose(login_key);
    }

    puts("INFO: Finished authenticating with FICS!");
}

void doit(int clientfd) 
{
    //
    // Code to deal with the client
    //
    char buf[MAXLINE];
    rio_t client_rio;

    puts("INFO: Preparing JARVIS to receive from the client");
    Rio_readinitb(&client_rio, clientfd);

    //
    // Code to deal with FICS
    // 
    int serverfd = 0;
    rio_t server_rio;

    puts("INFO: Preparing JARVIS to receive from FICS");
    Rio_readinitb(&server_rio, serverfd);

    puts("INFO: Opening connection to FICS...");
    serverfd = open_clientfd("freechess.org", 5000);

    if (serverfd < 0) {
        puts("ERROR: Couldn't open connection to FICS!");
        return;
    }
    else
        puts("INFO: Connection to FICS was successful!");

    puts("INFO: Authenticating with FICS using login_key.txt...");
    authenticateClient(clientfd, serverfd);

    //
    // Code to deal with FICS <-> client back and forth
    //
    int numReadFromFICS = 0;
    int numReadFromClient = 0;

    puts("INFO: Creating threads to route traffic between FICS and client...");

    pthread_t fromClientId;
    pthread_t fromFICSId;

    route clientToFICS = {clientfd, serverfd};
    route FICSToClient = {serverfd, clientfd};

    // fromFICSId thread routes traffic from FICS to Client.
    // fromClientId thread routes traffic from Client to FICS.
    pthread_create(&fromFICSId, NULL, route_to, &FICSToClient);
    pthread_create(&fromClientId, NULL, routeClientToFICS, &clientToFICS);

//    pthread_join(fromFICSId, NULL);
    pthread_join(fromClientId, NULL);

    pthread_detach(fromClientId);
    pthread_detach(fromFICSId);

    Close(serverfd);

    printf("INFO: exiting doit\n");
}
