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

  // buffer stores the HTTP request string
  char buffer[4096] = {0};
  // receives the HTTP request and stores in buffer
  recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  // converts char array to string
  std::string bufferStr(buffer);

  // positions between which the substring lies
  int pos1 = bufferStr.find('/');
  int pos2 = bufferStr.find(' ', pos1);
  // finds the second '/'
  int pos3 = bufferStr.find('/', pos1 + 1);

  // find the substring after '/'
  std::string reqString = bufferStr.substr(pos1 + 1, pos2 - pos1 - 1);

  // checks if it's an echo request by finding the "echo" string
  bool isEcho = false;
  std::string tempStr = "";
  std::string echoStr = "";
  if (pos3 != std::string::npos) {
    echoStr = bufferStr.substr(pos1 + 1, pos3 - pos1 -1);
  }
  if (echoStr == "echo") {
    isEcho = true;
  }

  // empty string, if there is nothing after '/', return OK
  std::string rootStr = "";

  // extract the string after echo/
  std::string contentStr = bufferStr.substr(pos3 + 1, pos2 - pos3 -1);

  // if there is a space after '/', send OK
  std::string response = "";

  if (isEcho) {
    response = "HTTP/1.1 200 OK\r\n\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(contentStr.length()) + "\r\n\r\n" + contentStr;
    send(client_fd, response.c_str(), strlen(response.c_str()), 0);
  }
  else if (reqString == rootStr) {
    response = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, response.c_str(), strlen(response.c_str()), 0);
  }
  // else, send the error message
  else {
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
    send(client_fd, response.c_str(), strlen(response.c_str()), 0);
  }

  // debug prints

  // pos1 and pos3
  std::cout << "Pos1: " << pos1 << " Pos3: " << pos3 << std::endl << std::endl;

  // echo string
  std::cout << "the echo string is: " << echoStr << std::endl << std::endl;

  // echo bool
  std::cout << "echo bool is(expected true): " << isEcho << std::endl << std::endl;

  close(server_fd);

  return 0;
}
