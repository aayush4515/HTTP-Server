// Refactored C++ HTTP server in a single .cpp file with reusable helper functions
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
#include <zlib.h>
#include <algorithm> // Needed for std::find

std::string fileDirectory = "."; // Default file directory

// Compresses data using gzip
std::string gzipCompress(const std::string& data) {
  z_stream zs{};
  deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
  zs.next_in = (Bytef*)data.data();
  zs.avail_in = data.size();

  std::string out;
  char buffer[4096];
  int ret;
  do {
    zs.next_out = (Bytef*)buffer;
    zs.avail_out = sizeof(buffer);
    ret = deflate(&zs, Z_FINISH);
    out.append(buffer, sizeof(buffer) - zs.avail_out);
  } while (ret == Z_OK);
  deflateEnd(&zs);
  return out;
}

// Checks for the presence of a specific header
bool checkHeaderPresence(const std::string& bufferStr, const std::string& header) {
  return bufferStr.find(header) != std::string::npos;
}

// Extracts value of a header (e.g., "User-Agent")
std::string extractHeaderValue(const std::string& bufferStr, const std::string& header) {
  int start = bufferStr.find(header);
  if (start == std::string::npos) return "";
  start += header.size();
  int end = bufferStr.find("\r\n", start);
  if (end == std::string::npos) return "";
  std::string value = bufferStr.substr(start, end - start);
  size_t firstChar = value.find_first_not_of(" ");
  return (firstChar != std::string::npos) ? value.substr(firstChar) : "";
}

// Parses Accept-Encoding header into a vector of encodings
std::vector<std::string> parseCompressionSchemes(const std::string& bufferStr) {
  std::vector<std::string> encodings;
  std::string raw = extractHeaderValue(bufferStr, "Accept-Encoding:");
  std::stringstream ss(raw);
  std::string item;
  while (std::getline(ss, item, ',')) {
    size_t start = item.find_first_not_of(" ");
    if (start != std::string::npos) encodings.push_back(item.substr(start));
  }
  return encodings;
}

// Extracts file content from POST request body
std::string extractFileContent(const std::string& bufferStr) {
  int bodyStart = bufferStr.find("\r\n\r\n");
  return (bodyStart != std::string::npos) ? bufferStr.substr(bodyStart + 4) : "";
}

// Builds a basic HTTP response with optional close header
std::string buildHttpResponse(const std::string& body, const std::string& contentType = "text/plain", bool close = false, const std::string& encoding = "") {
  std::string response = "HTTP/1.1 200 OK\r\n";
  if (!encoding.empty()) response += "Content-Encoding: " + encoding + "\r\n";
  if (close) response += "Connection: close\r\n";
  response += "Content-Type: " + contentType + "\r\n";
  response += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
  return response;
}

// Handles a single client request
void handleRequest(const std::string& bufferStr, int client_fd, bool& closeConnection) {
  std::string method = bufferStr.substr(0, bufferStr.find(" "));
  std::string path = bufferStr.substr(bufferStr.find(" ") + 1);
  path = path.substr(0, path.find(" "));

  bool isGET = (method == "GET");
  bool isPOST = (method == "POST");
  closeConnection = checkHeaderPresence(bufferStr, "Connection: close");

  std::vector<std::string> encodings = parseCompressionSchemes(bufferStr);
  bool gzipAccepted = std::find(encodings.begin(), encodings.end(), std::string("gzip")) != encodings.end();

  std::string response;

  if (isGET) {
    if (path.find("/echo/") == 0) {
      std::string msg = path.substr(6);
      if (gzipAccepted) {
        std::string compressed = gzipCompress(msg);
        std::string header = "HTTP/1.1 200 OK\r\nContent-Encoding: gzip\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(compressed.size()) + "\r\n\r\n";
        send(client_fd, header.c_str(), header.length(), 0);
        send(client_fd, compressed.data(), compressed.size(), 0);
        return;
      }
      response = buildHttpResponse(msg, "text/plain", closeConnection);
    } else if (path == "/user-agent") {
      std::string userAgent = extractHeaderValue(bufferStr, "User-Agent:");
      response = buildHttpResponse(userAgent, "text/plain", closeConnection);
    } else if (path.find("/files/") == 0) {
      std::string filename = path.substr(7);
      std::ifstream file(fileDirectory + filename);
      if (!file) {
        response = "HTTP/1.1 404 Not Found\r\n\r\n";
      } else {
        std::stringstream buffer;
        buffer << file.rdbuf();
        response = buildHttpResponse(buffer.str(), "application/octet-stream", closeConnection);
      }
    } else if (path == "/") {
      response = buildHttpResponse("", "text/plain", closeConnection);
    } else {
      response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }
  } else if (isPOST && path.find("/files/") == 0) {
    std::string filename = path.substr(7);
    std::ofstream outFile(fileDirectory + filename);
    if (outFile) {
      outFile << extractFileContent(bufferStr);
      response = "HTTP/1.1 201 Created\r\n\r\n";
    } else {
      response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    }
  }

  send(client_fd, response.c_str(), response.size(), 0);
  std::cout << "Request string: " << bufferStr << "\n\n";
}

// Handles the lifecycle of a client connection
void handleClient(int client_fd) {
  while (true) {
    char buffer[4096] = {0};
    int bytesReceived = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) break;
    std::string bufferStr(buffer);
    bool closeConnection = false;
    handleRequest(bufferStr, client_fd, closeConnection);
    if (closeConnection) break;
  }
  close(client_fd);
}

// Entry point for the server
int main(int argc, char **argv) {
  for (int i = 1; i < argc - 1; ++i) {
    if (std::string(argv[i]) == "--directory") {
      fileDirectory = argv[i + 1];
      break;
    }
  }

  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::cout << "Logs from your program will appear here!\n";

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) return 1;

  int reuse = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) != 0) return 1;
  if (listen(server_fd, 5) != 0) return 1;

  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  while (true) {
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    std::thread t([client_fd] { handleClient(client_fd); });
    t.detach();
  }

  close(server_fd);
  return 0;
}
