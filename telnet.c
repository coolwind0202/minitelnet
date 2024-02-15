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

int wait_connection(int server_socket)
{
}

int main()
{
  struct sockaddr_in client_name;
  socklen_t client_name_size = sizeof(client_name);

  struct sockaddr_in server_name;

  int client_socket = 0;
  int server_socket = create_socket();
  char buf[50];
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
    recv_len = recv(client_socket, buf, 10, 0);
    if (recv_len <= 0)
    {
      close(client_socket);
      close(server_socket);
      if (recv_len == 0)
      {
        exit(EXIT_SUCCESS);
      }
      else
      {
        perror("Failed to recv");
        exit(EXIT_FAILURE);
      }
    }

    fwrite(buf, recv_len, 1, stdout);
  }

  return 0;
}