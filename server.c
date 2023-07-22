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

#define TARGET_FILE "fractaljulia.bmp"
// key and key schedule
DES_cblock key = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
DES_key_schedule keysched;

// About TCP (from tcp man page):
// TCP guarantees that the data arrives in order and retransmits lost packets.  It generates  and
// checks a per-packet checksum to catch transmission errors.  TCP does not preserve record boundaries.

void send_response(int fd, char *file_content) {
    char response[BUFF_SIZE] = {0};
    printf("ok\n");
    strcat(response, file_content);
    if(send(fd, response, sizeof(response), 0) < 0) {
        perror("send");
        return;
    }
}

void print_data(const char *tittle, const void* data, int len)
{
    printf("%s : ",tittle);
    const unsigned char * p = (const unsigned char*)data;
    int i = 0;

    for (; i<len;++i)
        printf("%02X ", *p++);

    printf("\n");
}

int read_fractal(char *header, char *data) {
    FILE *fp; 
    fp = fopen(TARGET_FILE, "r");
    // read header
    for(int i=0; i<54; i++) {
        char c = fgetc(fp);
        //printf("%d ", c);
        header[i] = c;
    }

    // read body
    int dataCnt = 0;
    int maxSize = 246;
    while(1) {
        if(dataCnt >= maxSize) {
            char *new;
            maxSize *= 2;
            new = realloc(data, maxSize * sizeof(char));
            if (new == NULL) {
                perror("realloc");
                free(data);
                return 2;
            }
            data  = new;
        }
        int cnt = fscanf(fp, "%c", &data[dataCnt]);
        if(cnt == EOF)
            break;
        if(cnt < 1) {
            printf("Error reading data\n");
            return 1;
        }
        dataCnt++;
    }
    fclose(fp);
    return dataCnt;
}

char *get_current_time() {
    time_t curr_time = time(NULL);
    return strtok(ctime(&curr_time), "\n");
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

    // Bind the socket to the server address
    // Traditionally, this  operation  is  called “assigning a name to a socket”. bind(2)
    if(bind(tcp_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        exit(1);
    }

    // Listen for connections.
    if(listen(tcp_socket, LISTEN_BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }

    // Accept a connection.
    namelen = sizeof(client);
    if((ns = accept(tcp_socket, (struct sockaddr *)&client, (socklen_t *) &namelen)) < 0) {
        perror("accept");
        exit(1);
    }

    // Successful connection
    printf("[%s] %s:%d Accepted\n", get_current_time(), inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    // read fractal and get the length
    char header[55];
    char *data;
    data = malloc(sizeof(char) * 256);
    int len = read_fractal(header, data);

    // encrypt fractal
    char cipher[len];
    DES_cblock iv = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    DES_ncbc_encrypt((unsigned char *)data, (unsigned char *)cipher, len,&keysched, &iv, DES_ENCRYPT);

    //print_data("Original data: ", (const void *)data, len);
    //print_data("Encrypted data: ", (const void *)cipher, len);

    // Send the header (not encrypted)
    if(send(ns, header, 55, 0) < 0) {
        perror("send");
        exit(0);
    }

    int pos = 0;
    while(pos < len) {
        char buff[1024];
        int i;
        for(i=0; i<1023; i++) {
            if(pos >= len) break;
            buff[i] = cipher[pos];
            pos++;
        }
        buff[i++] = '\0';
        printf("%s\n", buff);
        // Send encrypted fractal in parts of 1024 bits
        if(send(ns, buff, i, 0) < 0) {
            perror("send");
            exit(0);
        }
    }
    printf("body size: %d", pos);

    //send_response(ns, data);
    printf("[%s] %s:%d Closed\n", get_current_time(), inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    close(ns);


    printf("[%s] Server closing\n", get_current_time());
    close(tcp_socket);
    exit(0);
}

