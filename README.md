[![progress-banner](https://backend.codecrafters.io/progress/http-server/4620c915-4ffb-45dd-b0f5-dcbab2f71cea)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

# 🌀 HTTP Server in C++

A fully functional multithreaded HTTP/1.1 server written in modern C++, built from scratch as part of the [Codecrafters HTTP Server Challenge](https://app.codecrafters.io/).

---

## 🚀 Features Implemented

### ✅ Base HTTP Functionality

- 📦 **Bind to Port** – Listens on `localhost:4221`
- ✅ **Respond with 200 OK**
- 🔍 **Extract URL Path**
- 📄 **Respond with Body** – `/echo/{data}` returns the path fragment in the response body
- 🧠 **Read HTTP Headers** – Supports parsing headers like `User-Agent`
- 🧵 **Concurrent Connections** – Each client handled in a separate thread
- 📂 **Return a File** – Returns contents of files using `/files/{filename}`
- ✉️ **Read Request Body** – Accepts POST requests and writes body contents to disk

### 🧬 HTTP Compression

- 🧩 **Parse Compression Headers** – Detects `Accept-Encoding`
- 🔀 **Support for Multiple Compression Schemes** – Handles comma-separated encodings
- 🗜️ **Gzip Compression** – Compresses response using zlib if `gzip` is supported

### 🔁 Persistent Connections

- ♾️ **Persistent HTTP/1.1 Connections** – Maintains open TCP connection for multiple requests
- 🧵 **Multiple Concurrent Persistent Clients** – Each connection is thread-handled independently
- ❌ **Connection Closure Support** – Properly handles `Connection: close` header

---

## 🛠️ Tech Stack

- **C++17**
- **POSIX Sockets**
- **Multithreading** using `std::thread`
- **HTTP parsing** (manual, string-based)
- **Filesystem API** via `<filesystem>`
- **Compression** using `zlib`

---

## 🧪 How to Run

### Prerequisites

- Linux/macOS/WSL environment
- C++17 compatible compiler (e.g., `g++`, `clang++`)
- zlib development headers (install with: `sudo apt install zlib1g-dev`)

### Build the Server

```bash
make build
```

### Run the Server

```bash
./your_http_server --directory ./files/
```

---

## 📬 Example Endpoints

| HTTP Endpoint         | Description                                          |
| --------------------- | ---------------------------------------------------- |
| `GET /echo/hello`     | Responds with `hello` as the body                    |
| `GET /user-agent`     | Echoes back the `User-Agent` request header          |
| `GET /files/foo.txt`  | Returns contents of `foo.txt` in the files directory |
| `POST /files/foo.txt` | Writes request body to `foo.txt`                     |

---

## 🔁 Test Persistent Connection

- 🔧 How HTTP/1.1 works under the hood (including headers, status lines, and persistent connections)

- 🧵 How to manage concurrent socket clients using threads

- 🔍 How to manually parse request headers and paths from raw data

- 🗜️ How to apply gzip compression in HTTP responses using zlib

- 🧼 Clean code practices and refactoring for modular request handling

---

## 🔗 Credits
Built for the [Codecrafters HTTP Server Challenge](https://app.codecrafters.io/) using C++ from scratch without external web frameworks.
