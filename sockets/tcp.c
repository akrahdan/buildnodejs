#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#define SERVER_PORT 3000

void service (int in, int out) {
    unsigned char buf [1024];
    int count;
    while ((count = read(in, buf, 1024))>0)
    {
        write(out, buf, count);
    }
}

int main (){
    int sock, fd, client_len;
    struct sockaddr_in server, client;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(SERVER_PORT);
    bind(sock, (struct sockaddr *)&server, sizeof(server));
    listen(sock, 10);
    printf("listening ....\n");
    
    while (1) {
        client_len = sizeof(client);
        fd = accept(sock, (struct sockaddr *)&client, &client_len);
        printf("got connection\n");
        service(fd, fd);
        close(fd);
    }
    
}