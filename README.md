# Laboratório de criptografia

## Como rodar

Primeiro, gere o arquivo fractal:

```bash
gcc fractal.c -o fractal -lm
./fractal 100
```

Segundo, rode o servidor TCP:

```bash
gcc server.c -o server -lcrypto
./server 127.0.0.1 8080
```

Terceiro, rode o cliente TCP:

```bash
gcc client.c -o client -lcrypto
./client 127.0.0.1 8080
```

## Resultados

### Arquivo de entrada (fractaljulia.bmp):

### Arquivo encriptado (encrypted.bmp):
### Arquivo desencriptado (decrypted.bmp):
