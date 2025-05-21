#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/time.h>  // for timeval

// Simple IRC test client for parser verification

// Reads lines (terminated by CRLF) from the server
std::vector<std::string> readReplies(int sock) {
    std::vector<std::string> replies;
    char buffer[512];
    std::string partial;
    int n;
    // Read until no more data (with small timeout)
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        partial += buffer;
        size_t pos;
        while ((pos = partial.find("\r\n")) != std::string::npos) {
            replies.push_back(partial.substr(0, pos));
            partial.erase(0, pos + 2);
        }
    }
    return replies;
}

int testParser() {
    const char* server_ip = "127.0.0.1";
    const int server_port = 6667;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        std::cerr << "inet_pton() failed" << std::endl;
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "connect() failed: " << strerror(errno) << std::endl;
        close(sock);
        return 1;
    }
    std::cout << "Connected to " << server_ip << ":" << server_port << std::endl;

    // Send registration sequence
    std::vector<std::string> cmds;
    cmds.push_back("PASS secret");
    cmds.push_back("NICK UwU-Chan");
    cmds.push_back("USER testuser 0 * :Real Test User");

    for (size_t i = 0; i < cmds.size(); ++i) {
        std::string msg = cmds[i] + "\r\n";
        if (send(sock, msg.c_str(), msg.size(), 0) < 0) {
            std::cerr << "send() failed: " << strerror(errno) << std::endl;
            close(sock);
            return 1;
        }
        std::cout << "Sent: " << cmds[i] << std::endl;
    }

    // Read server replies
    std::vector<std::string> replies = readReplies(sock);
    std::cout << "Server replies:" << std::endl;
    for (size_t i = 0; i < replies.size(); ++i) {
        std::cout << replies[i] << std::endl;
    }

    close(sock);
    return 0;
}

int main() {
    return testParser();
}