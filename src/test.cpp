#include <iostream>
#include <string>

using namespace std;

int main() {

    string str = "GET /echo/blueberry HTTP/1.1\r\nHost: localhost:4221\r\nAccept-Encoding: encoding-1, gzip, encoding-2\r\n\r\n";

    size_t encPos = str.find("Accept-Encoding: ");
    size_t valueStart = encPos + strlen("Accept-Encoding: ");
    size_t lineEnd = str.find("\r\n", valueStart);

    string encodingLine = str.substr(valueStart, lineEnd - valueStart);

    cout << "Encoding line: " << encodingLine << endl;

    return 0;
}