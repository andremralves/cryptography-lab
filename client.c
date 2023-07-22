#include <stdio.h>
#include <sys/socket.h> // socket()
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <openssl/des.h>

#define LISTEN_BACKLOG 5
#define BUFF_SIZE 65536 // 64k

// key and key schedule
DES_cblock key = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
DES_key_schedule keysched;

void print_data(const char *tittle, const void* data, int len)
{
    printf("%s : ",tittle);
    const unsigned char * p = (const unsigned char*)data;
    int i = 0;

    for (; i<len;++i)
        printf("%02X ", *p++);

    printf("\n");
}

void write_to_file(FILE *fp, char *bytes, int len) {
    for(int i=0; i<len; i++) {
        fputc(bytes[i], fp);
    }
}
 
int main(int argc, char *argv[]) {

    // Variables
    unsigned short port;                // port server binds to
    struct sockaddr_in client;          // client addr info
    struct sockaddr_in server;          // server addr info
    int tcp_socket;                     // socket
    int ns;                             // socket connected to client
    int namelen;                        // length og client name 
    char server_addr[20];


    // Check arguments. Should be two, the addr and the port number.
    if(argc != 3) {
        fprintf(stderr, "Usage: %s addr port\n", argv[0]);
        exit(1);
    }

    // Pass the first argument to be the port number.
    port = (unsigned short) atoi(argv[2]);
    strcpy(server_addr, argv[1]);

    // Create the socket.
    if((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(server_addr);

    // connect the client socket to server socket
    if (connect(tcp_socket, (struct sockaddr*)&server, sizeof(server)) != 0) {
        perror("connect");
        exit(1);
    }

    char header[55];
    char *body;
    // Allocate 128K to body
    body = malloc(sizeof(char) * 131072);
    // Receive header
    int msglen;
    if((msglen = recv(tcp_socket, header, sizeof(header), 0)) < 0) {
        perror("recv");
        exit(1);
    }

    int pos = 0;
    // Read the messages and concatenate to the body
    while(1) {
        char buff[1024];
        int msglen;
        if((msglen = recv(tcp_socket, buff, sizeof(buff), 0)) < 0) {
            perror("recv");
            exit(1);
        }
        if(msglen == 0) break;
        for(int i=0; i<msglen-1; i++) {
            body[pos] = buff[i];
            pos++;
        }
    }
    printf("body size: %d", pos);
    int len = pos;

    char decrypted[len];
    //print_data("encrypted: ", (const void *)body, len);
    DES_cblock iv = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    memset(iv, 0, 8);
    DES_ncbc_encrypt((unsigned char *)body, (unsigned char *)decrypted, len,&keysched, &iv, DES_DECRYPT);
    //print_data("decrypted: ", (const void *)decrypted, len);

    FILE *fp1, *fp2;

    fp1 = fopen("encrypted.bmp", "w");
    fp2 = fopen("decrypted.bmp", "w");

    write_to_file(fp1, header, 54);
    write_to_file(fp1, body, len);

    write_to_file(fp2, header, 54);
    write_to_file(fp2, decrypted, len);

    fclose(fp1);
    fclose(fp2);
    exit(0);
}


