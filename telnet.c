#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
// for socket API
#include <sys/socket.h>

// for sockaddr_in
#include <netinet/in.h>

// for true
#include <stdbool.h>

// for close()
#include <unistd.h>

#define PORT 3000
#define NAWS 31

#define SE 240
#define NOP 241
#define SB 250
#define WILL 251
#define WONT 252
#define DO 253
#define DONT 254
#define IAC 255

int create_socket()
{
  int sock = 0;
  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  return sock;
}

int server_socket = 0;

void exit_and_close_socket(int client_socket, int return_code)
{
  close(client_socket);
  close(server_socket);
  exit(return_code);
}

ssize_t recv_with_handling(int socket, void *buf, size_t n)
{
  ssize_t recv_len = 0;
  recv_len = recv(socket, buf, n, 0);
  if (recv_len < 0)
  {
    perror("Failed to receive");
    exit_and_close_socket(socket, EXIT_FAILURE);
  }

  if (recv_len == 0)
  {
    fputs("Connection was closed.\n", stderr);
    exit_and_close_socket(socket, EXIT_SUCCESS);
  }

  return recv_len;
}

unsigned char *recv_until(int socket, unsigned char c)
{
  size_t memsize = 100;
  char *input = calloc(memsize, sizeof(unsigned char));
  ssize_t recv_len = 0, total_len = 0;
  if (input == NULL)
  {
    fputs("Failed to allocate memory.\n", stderr);
    exit_and_close_socket(socket, EXIT_FAILURE);
  }

  while (input[total_len] != c)
  {
    recv_len = recv_with_handling(socket, input + total_len, 1);
    total_len += recv_len;

    if (total_len + 1 >= memsize)
      input = realloc(input, memsize += 100);
  }

  input[++total_len] = 0;
  return input;
}

int main()
{
  struct sockaddr_in client_name;
  socklen_t client_name_size = sizeof(client_name);

  struct sockaddr_in server_name;

  int client_socket = 0;
  server_socket = create_socket();
  unsigned char buf[50];
  ssize_t recv_len = 0;

  server_name.sin_family = AF_INET,
  server_name.sin_port = htons(PORT);
  server_name.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(server_socket, (struct sockaddr *)&server_name, sizeof(server_name)) < 0)
  {
    perror("Failed to bind an address to server socket");
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, 1) < 0)
  {
    perror("Failed to listen");
    exit(EXIT_FAILURE);
  }

  client_socket = accept(server_socket, (struct sockaddr *)&client_name, &client_name_size);

  if (client_socket < 0)
  {
    perror("Faild to accpet connection");
    exit(EXIT_FAILURE);
  }

  while (true)
  {
    recv_len = recv_with_handling(client_socket, buf, 1);

    /* Process Command following Iac(0xff) */
    if (buf[0] == IAC)
    {
      unsigned char response[3] = {IAC, 0, buf[1]};
      recv_len = recv_with_handling(client_socket, buf, 2);

      if (recv_len < 2)
      {
        fputs("Iac code is comming, but command or option is not provided.\n", stderr);
        exit_and_close_socket(client_socket, EXIT_FAILURE);
      }

      switch (buf[0])
      {
      case WILL:
        // Request from client whether we will use the option
        response[1] = DONT;
        send(client_socket, response, 3, 0);
        break;
      case DO:
        response[1] = WONT;
        send(client_socket, response, 3, 0);
        break;
      case DONT:
      case WONT:
        break;
      case SB:
        // read until coming SE
        recv_until(client_socket, SE);
        break;
      default:
        fputs("unknown command", stderr);
        break;
      }
    }
    else
    {
      fwrite(buf, recv_len, 1, stdout);
    }
  }

  return 0;
}