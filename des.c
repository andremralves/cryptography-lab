#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <openssl/des.h>

#define LISTEN_BACKLOG 5
#define BUFF_SIZE 65536 // 64k

#define TARGET_FILE "fractaljulia.bmp"

#define ENC 0
#define DEC 1

// key and key schedule
DES_cblock key = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
DES_key_schedule keysched;

typedef struct {
    char header[55];
    char* data;
} BMPImage;

/* Print Encrypted and Decrypted data packets */
void print_data(const char *tittle, const void* data, int len);

unsigned char *des_encrypt(unsigned char *input, DES_key_schedule keysched) {
    unsigned char *cipher[sizeof(input)];
    DES_ecb_encrypt(input, cipher, &keysched, DES_ENCRYPT);
    return cipher;
}
void des_decrypt() {
}

int main() {
    DES_set_key(&key, &keysched);
    unsigned char text[20] = "hello world!";
    unsigned char cipher[sizeof(text)];
    unsigned char dec[sizeof(text)];

    DES_cblock iv = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    DES_ncbc_encrypt(text, cipher, sizeof(text),&keysched, &iv, DES_ENCRYPT);
    memset(iv, 0, 8);
    DES_ncbc_encrypt(cipher, dec, sizeof(text),&keysched, &iv, DES_DECRYPT);
    //DES_ecb_encrypt(cipher, dec, &keysched, DES_DECRYPT);
    //cipher = des_encrypt(text, keysched);
    print_data("original: ", (const void*)text, sizeof(text));
    print_data("encrypted: ", (const void*)cipher, sizeof(text));
    print_data("decrypted: ", (const void*)dec, sizeof(text));
    printf("%s\n", text);
    printf("%s\n", cipher);
    return 0;
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

