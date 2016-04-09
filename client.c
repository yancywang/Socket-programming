/*
** client.c -- a stream socket client demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define SERVERA "21000209" // the serverA port number 
#define SERVERB "22000209" // the serverB port number 
#define SERVERC "23000209" // the serverC port number 
#define SERVERD "24000209" // the serverD port number 
#define MAXDATASIZE 100 // max number of bytes we can get at once 
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct matrix{
    int mat[4][4];
};

int main(int argc, char *argv[])
{
    struct matrix adj_matrix[5];
    int index=0;
    char port[4][100]={SERVERA,SERVERB,SERVERC,SERVERD};
    int test[4];
    int sockfd, numbytes,i,j;  
    char buf[MAXDATASIZE];
    char hostname[128];
    struct addrinfo hints, *servinfo, *p;
        int rv;
    char s[INET6_ADDRSTRLEN];
    //if (argc != 2) {
    //    fprintf(stderr,"usage: client hostname\n");
    //    exit(1);
    //}
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    gethostname(hostname,sizeof hostname);
    for(index=0;index<4;index++)
    {    
    if ((rv = getaddrinfo(hostname, port[index], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
                    }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    if ((numbytes = recv(sockfd, &adj_matrix[index], MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    
    //buf[numbytes] = '\0';
    //printf("client: received '%s'\n",buf);
    close(sockfd);
}

    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            adj_matrix[4].mat[i][j] = 0;
            for(index=0;index<4;index++){
                if(adj_matrix[index].mat[i][j]>0){
                    adj_matrix[4].mat[i][j]= adj_matrix[index].mat[i][j];
                }
            }
            
            printf("%d\t",adj_matrix[index].mat[i][j]);
        }
        printf("\n");
    }

    return 0;

}
