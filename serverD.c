/*
** server.c -- a stream socket server demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT "24000209"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
#define FILENAME "serverD.txt" //the input server file
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
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
//input file serverA.txt
struct matrix get_in_servertxt(FILE *f)
{   
    struct matrix m;
    int matrix[4][4]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
    if(!f){
        printf("cannot open server file");
        exit(1);
    }
    char buf[4][20];
    int cost[4];
    int index = 0,i,j;
    while(!feof(f))
    {
        fscanf(f,"%s%d",buf[index],&cost[index]);
        if(strncmp(buf[index],"serverB",7)==0)
        {
            matrix[3][1]=cost[index];
            matrix[1][3]=cost[index];
        }
        else if(strncmp(buf[index],"serverC",7)==0)
        {
            matrix[3][2]=cost[index];
            matrix[2][3]=cost[index];
        }
        else if(strncmp(buf[index],"serverA",7)==0)
        {
            matrix[0][3]=cost[index];
            matrix[3][0]=cost[index];
        }
        index++;
    }
    index--;
    for(i=0;i<4;i++)
    {
        for(j=0;j<4;j++){
            m.mat[i][j]=matrix[i][j];
        }
    }
    return m;
}
int main(void)
{
    FILE *f = fopen(FILENAME,"r");
    struct matrix adj_matrix = get_in_servertxt(f);
    int i,j;
    for(i=0;i<4;i++)
    {
        for(j=0;j<4;j++){
            printf("%d\t",adj_matrix.mat[i][j]);
        }
        printf("\n");
    }
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int test[4] = {1,2,3,4};
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
                break;
    }
    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("server: waiting for connections...\n");
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
                inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            if (send(new_fd, &adj_matrix, sizeof(adj_matrix)+1, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }
    return 0;
}