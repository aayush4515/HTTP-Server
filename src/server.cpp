#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

std::string fileDirectory = ".";  // Default to current directory

void handleClient(int client_fd) {
  // buffer stores the HTTP request string
  char buffer[4096] = {0};
  // receives the HTTP request and stores in buffer
  recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  // converts char array to string
  std::string bufferStr(buffer);

  // checks whether GET or POST request
  bool isGET = false;
  bool isPOST = false;

  std::string temp = bufferStr.substr(0, 4);
  if (temp == "GET ") {
    isGET = true;
  }
  else if (temp == "POST") {
    isPOST = true;
  }

  // positions between which the substring lies
  int pos1 = bufferStr.find('/');
  int pos2 = bufferStr.find(' ', pos1);
  // finds the second '/'
  int pos3 = bufferStr.find('/', pos1 + 1);

  // find the substring after '/'
  std::string reqString = bufferStr.substr(pos1 + 1, pos2 - pos1 - 1);

  // checks if it's an echo request by finding the "echo" string
  bool isEcho = false;
  std::string echoStr = "";
  if (pos3 != std::string::npos) {
    echoStr = bufferStr.substr(pos1 + 1, pos3 - pos1 -1);
  }
  if (echoStr == "echo") {
    isEcho = true;
  }

  // chekcs if it's a user-agent request by finding the "user-agent" string
  bool isUserAgent = false;
  std::string userAgentStr = "";
  if (pos1 != std::string::npos && pos2 != std::string::npos) {
    userAgentStr = bufferStr.substr(pos1 + 1, pos2 - pos1 - 1);
  }
  if (userAgentStr == "user-agent") {
    isUserAgent = true;
  }

  // checks if it's a file request by finding the "files" string
  bool isFileRequest = false;
  std::string fileStr = "";
  if (pos1 != std::string::npos && pos2 != std::string::npos) {
    fileStr = bufferStr.substr(pos1 + 1, pos3 - pos1 - 1);
  }
  if (fileStr == "files") {
    isFileRequest = true;
  }

  // checks if it has compression headers
  bool acceptsEncoding = false;
  if (bufferStr.find("Accept-Encoding") != std::string::npos) {
    acceptsEncoding = true;
  }
  // std::string compressionScheme = "";

  // size_t encPos = bufferStr.find("Accept-Encoding: ");
  // if (encPos != std::string::npos and acceptsEncoding) {
  //   size_t valStart = encPos + strlen("Accept-Encoding: ");
  //   size_t valEnd = bufferStr.find("\r\n", valStart);
  //   compressionScheme = bufferStr.substr(valStart, valEnd - valStart);
  // }

  // handles multiple comma-separated encodings
  std::vector<std::string> compressionSchemes;                                    // stores encoding schemes
  size_t encPos = 0;
  if (acceptsEncoding) {
    encPos = bufferStr.find("Accept-Encoding: ");                                 // first index of "Accpet-Encoding" string
  }
  size_t valueStart = encPos + strlen("Accept-Encoding: ");                       // starting index of encodings
  size_t lineEnd = bufferStr.find("\r\n", valueStart);                            // final index of encodings

  std::string encodingLine = bufferStr.substr(valueStart, lineEnd - valueStart);  // stores the encoding line
  std::stringstream ss(encodingLine);                                             // stringstream instance with encodingLine for string parsing
  std::string item;                                                               // temp string to hold individual strings temporarily

  // reads from stringstream ss until the next comma, and stores each encoding in item in each loop
  while(std::getline(ss, item, ',')) {
    size_t firstChar = item.find_first_not_of(" ");                       // stores the index of the first character which is not a whitespace; i.e. the first encoding
    compressionSchemes.push_back(item.substr(firstChar));                 // stores each encoding in the compression schemes vector, firstChar specifies the starting position, in this case
                                                                          // used for specifiying the first char after space... in short, removing the leading space
  }

  //std::cout << "Compression Scheme before entering is statements: " << compressionScheme << std::endl << std::endl;


  // empty string, if there is nothing after '/', return OK
  std::string rootStr = "";

  // extract the string after echo/
  std::string contentStr = bufferStr.substr(pos3 + 1, pos2 - pos3 -1);

  // extracts the string after 'User-Agent:' header

    std::string userAgentContent = "";

    // stores the first occurence of the string "User-Agent"
    int start = bufferStr.find("User-Agent:") + strlen("User-Agent:");
    // stores the index of last character after the user-agent header
    int end = bufferStr.find("\r\n", start);
    // stores the content of the user-agent header
    if (start != std::string::npos && end != std::string::npos) {
      userAgentContent = bufferStr.substr(start + 1, end - start - 1);
    }

  // extract the filename for the /files endpoint
  std::string fileName = bufferStr.substr(pos3 + 1, pos2 - pos3 - 1);

  // string used to store the response message to send back to the client
  std::string response = "";

  if (isGET) {
    if (isEcho) {

      // check if it accepts encoding
      if (acceptsEncoding) {
        //std::cout << "Accepts Encoding!!!" << std::endl << std::endl;
        //std::cout << "Compression Scheme: " << compressionScheme << "Size: " << strlen(compressionScheme.c_str()) << std::endl;
        // if (strcmp(compressionScheme.c_str(), "gzip") == 0) {
        //   //std::cout << "Content Encoding added to response!!!" << std::endl << std::endl;
        //   response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Encoding: " + compressionScheme + "\r\n\r\n";
        // } else {
        //   response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
        //   //std::cout << "Content Encoding not added to response!!!" << std::endl << std::endl;
        // }

        // check for gzip
        bool hasGzip = false;

        // check for every single encoding
        for (const auto& encoding : compressionSchemes) {
          if (strcmp(encoding.c_str(), "gzip") == 0) {
            hasGzip = true;
          }
        }
        if (hasGzip) {
          response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Encoding: gzip\r\n\r\n";
        }
        else {
          response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
        }
      }
      else {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(contentStr.length()) + "\r\n\r\n" + contentStr;
      }
      send(client_fd, response.c_str(), strlen(response.c_str()), 0);
    }
    else if (isUserAgent) {
      response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(userAgentContent.length()) + "\r\n\r\n" + userAgentContent;
      send(client_fd, response.c_str(), strlen(response.c_str()), 0);
    }
    else if (isFileRequest) {
      // try to open the file
      // if the file exists, provide proper 200 OK response with file content
      // else, return 404 Not Found error
      std::string fullPath = fileDirectory + fileName;
      std::ifstream file(fullPath);

      if (!file) {
        // send a 404 error
        response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_fd, response.c_str(), strlen(response.c_str()), 0);
      }
      else {
        // determine the file size using the path of the file; uses the filesystem library
        auto size = std::filesystem::file_size(fullPath);

        // read the content of the file and store it in the buffer, use ifstream instance 'file' here
        std::stringstream buffer;
        buffer << file.rdbuf();

        // send 200 OK with different headers and file content
        response = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: " + std::to_string(size) + "\r\n\r\n" + buffer.str();
        send(client_fd, response.c_str(), strlen(response.c_str()), 0);
      }

      file.close();

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
  }
  else if (isPOST) {
    // check if it is a file request
    if (isFileRequest) {
      // extract content to add to the file from the request stirng
      int startIdx = bufferStr.find_last_of("\r\n");
      int endIdx = bufferStr.length() - 1;
      std::string fileContent = bufferStr.substr(startIdx + 1, endIdx - startIdx);

      // create the file using the file path
      std::string fullPath = fileDirectory + fileName;
      std::ofstream outFile(fullPath);

      if (outFile) {
        // write to the file
        outFile << fileContent;

        // close the file
        outFile.close();

        // response string
        response = "HTTP/1.1 201 Created\r\n\r\n";
      }
      else {
        response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
      }
      send(client_fd, response.c_str(), strlen(response.c_str()), 0);
    }
  }

  close(client_fd);

  // display the request string for debugging
  std::cout << "Request string: " << bufferStr << std::endl << std::endl;
}




int main(int argc, char **argv) {

  // Parse command-line arguments
  for (int i = 1; i < argc - 1; ++i) {
    if (std::string(argv[i]) == "--directory") {
        fileDirectory = argv[i + 1];
        break;
    }
  }

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

  while(true)  {
    // Client file descriptor
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    std::cout << "Client connected\n";

    std::thread t([client_fd] {
      handleClient(client_fd);
    });
    t.detach();

  }
  close(server_fd);
  return 0;
}
