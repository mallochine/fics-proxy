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

void doit(int clientfd) 
{
    //
    // Code to deal with the client
    //
    char buf[MAXLINE];
    rio_t client_rio;

    printf("INFO: Preparing JARVIS to receive from the client\n");
    Rio_readinitb(&client_rio, clientfd);

    //
    // Code to deal with FICS
    // 
    int serverfd = 0;
    rio_t server_rio;

    printf("INFO: Preparing JARVIS to receive from FICS\n");
    Rio_readinitb(&server_rio, serverfd);

    printf("INFO: Opening connection to FICS...\n");
    serverfd = open_clientfd("freechess.org", 5000);

    if (serverfd < 0) {
        printf("ERROR: Couldn't open connection to FICS!\n");
        return;
    }
    else
        printf("INFO: Connection to FICS was successful!\n");

    //
    // Code to deal with FICS <-> client back and forth
    //
    int numReadFromFICS = 0;
    int numReadFromClient = 0;

    printf("INFO: Creating threads to route traffic between FICS and client...\n");

    pthread_t fromClientId;
    pthread_t fromFICSId;

    route clientToFICS = {clientfd, serverfd};
    route FICSToClient = {serverfd, clientfd};

    // fromFICSId thread routes traffic from FICS to Client.
    // fromClientId thread routes traffic from Client to FICS.
    pthread_create(&fromFICSId, NULL, route_to, &FICSToClient);
    pthread_create(&fromClientId, NULL, route_to, &clientToFICS);

//    pthread_join(fromFICSId, NULL);
    pthread_join(fromClientId, NULL);

    Close(serverfd);

    printf("INFO: exiting doit\n");
}
