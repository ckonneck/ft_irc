#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

// Hard-coded configuration for IRC test client
int main() {
    const std::string server = "irc.42.fr"; // Replace with your IRC server
    const std::string port = "6667";              // Standard IRC port
    const std::string nickname = "TestBot";
    const std::string username = "testbot";
    const std::string realname = "Automated Test Bot";
    const std::string channel  = "#testchannel";

    // Resolve server address
    addrinfo hints{}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(server.c_str(), port.c_str(), &hints, &res) != 0) {
        std::cerr << "Failed to resolve address\n";
        return 1;
    }

    // Create socket and connect
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return 1;
    }
    std::cout << "Connecting to " << server << ":" << port << "...\n";
    if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }
    freeaddrinfo(res);

    auto send_cmd = [&](const std::string &cmd) {
        std::string full = cmd + "\r\n";
        std::cout << "--> " << cmd << "\n";
        send(sock, full.c_str(), full.size(), 0);
    };

    // Register with NICK and USER
    send_cmd("NICK " + nickname);
    send_cmd("USER " + username + " 0 * :" + realname);

    bool joined = false;
    auto start = std::chrono::steady_clock::now();

    char buffer[512];
    while (true) {
        ssize_t len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) {
            std::cerr << "Connection closed or error\n";
            break;
        }
        buffer[len] = '\0';
        std::istringstream iss(buffer);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            std::cout << "<-- " << line << "\n";

            // Respond to PING
            if (line.rfind("PING ", 0) == 0) {
                std::string server_ping = line.substr(5);
                send_cmd("PONG " + server_ping);
            }
            // On welcome (001), join channel
            if (!joined && line.find(" 001 ") != std::string::npos) {
                send_cmd("JOIN " + channel);
                joined = true;
                send_cmd("PRIVMSG " + channel + " :Hello from TestBot!");
            }
            // Echo command
            std::string priv = "PRIVMSG " + channel + " :";
            auto pos = line.find(priv);
            if (pos != std::string::npos) {
                std::string msg = line.substr(pos + priv.size());
                if (msg.rfind("!echo ", 0) == 0) {
                    std::string echo_text = msg.substr(6);
                    send_cmd("PRIVMSG " + channel + " :Echo: " + echo_text);
                }
            }
        }
        // Exit after 60 seconds
        auto now = std::chrono::steady_clock::now();
        if (joined && std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 60) {
            break;
        }
    }

    // Quit
    send_cmd("QUIT :Test complete");
    close(sock);
    std::cout << "Disconnected.\n";
    return 0;
}
