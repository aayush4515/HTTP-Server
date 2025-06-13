#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  //
  // Server File Descriptor
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  // Client file descriptor
  int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::cout << "Client connected\n";

  // The following was for the challenge "Respond with 200"

  // Response to be sent to the client
  // const char* response = "HTTP/1.1 200 OK\r\n\r\n";
  // Send the response
  // send(client_fd, response, strlen(response), 0);

  /*

  // buffer stored the HTTP request string
  char buffer[4096] = {0};

  // receives the HTTP request and stores in buffer
  recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  // display the request string for debugging
  // std:: cout << "The request string is: " << buffer << std::endl << std::endl;

  // stores first six characters of the request strings
  std::string reqSubStr = "";

  // actual first six characters to send a 200 OK
  std::string actualStr = "GET / ";

  // use a loop to parse the request string's first six characters
  for (int i = 0; i < 6; i++) {
    reqSubStr += buffer[i];
  }

  // display the request sub string for debugging
  // std:: cout << "The request sub-string is: " << reqSubStr << std::endl << std::endl;

  */

  char buffer[4096] = {0};
  recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  std::string bufferStr = "";
  for (int i = 0; i < 4095; i ++) {
    bufferStr += buffer[i];
  }

  // display bufferStr
  std::cout << "Buffer string is: " << bufferStr << std::end << std::endl;

  int pos1 = bufferStr.find('/');
  int pos2 = bufferStr.find('H');

  std::string reqString = bufferStr.substr(pos1 + 1, pos2 - pos1 - 1);
  std::string rootStr = "";

  if (reqString == rootStr) {
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", strlen("HTTP/1.1 200 OK\r\n\r\n"), 0);
  }
  else {
    send(client_fd, "HTTP/1.1 404 Not Found\r\n\r\n", strlen("HTTP/1.1 404 Not Found\r\n\r\n"), 0);
  }

  close(server_fd);

  return 0;
}
