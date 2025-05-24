// 🌸 Nyaa~ Welcome to the cutest IRC Bot ever~ UwU!! 🐾✨
// 🐱 This bot connects to an IRC server and meow-nitors messages~ nya~ 💬💕
// By the power of kawaii, let's chat~!! ^w^ 🎀

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <regex>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

int main() {
    // 🌈 Server infos~ Meowster setup!!! 🐾💖
    const char* server = "127.0.0.1"; // (≧◡≦) Local kitty server!
    const char* port   = "6667";
    const std::string nickname = "NYAN2";

    // 🗺️ Resolve the address~ nya!
    addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int gai = getaddrinfo(server, port, &hints, &res);
    if (gai != 0) {
        std::cerr << "Oh noes~ >w< getaddrinfo failed: "
                  << gai_strerror(gai) << " 😿\n";
        return 1;
    }

    // 🐾 Creating a magical socket~! ✨
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        std::perror("Nyaa~ socket creation failed :c ");
        freeaddrinfo(res);
        return 1;
    }

    // 🔌 Connect meow-ment! Initiating pawtocol~ nyaaan!
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        std::perror("Nyaa~ couldn't connect to the server 😿");
        close(sock);
        freeaddrinfo(res);
        return 1;
    }
    freeaddrinfo(res);

    // 🎀 Locky-wocky so multiple paws don't send messages at once nya~!
    std::mutex send_mtx;
    auto send_raw = [&](const std::string &cmd) {
        std::string out = cmd + "\r\n";
        std::lock_guard<std::mutex> lk(send_mtx);
        send(sock, out.c_str(), out.size(), 0);
    };

    // 🍮 Moved channel tracking here
    std::string current_chan;
    std::mutex chan_mtx;

    // 🐱 Listening for messages like a curious kitten~!! 💌🐾
    std::thread reader([&]() {
        char buf[512];
        // regex to detect server JOIN confirmations
        const std::regex join_re(R"(^:([^!]+)![^ ]+ JOIN :?(#\S+))");
        std::smatch m;

        while (true) {
            ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0);
            if (n < 0) {
                std::perror("Nyaa~ couldn't receive message 😿");
                break;
            }
            if (n == 0) {
                std::cerr << "Oh noes~ the server has left the cat café... T^T\n";
                break;
            }
            buf[n] = '\0';
            // print *all* server messages
            std::cout << buf;

            // 🐾 Only print the “Joined” echo when the server confirms it
            std::string s(buf);
            if (std::regex_search(s, m, join_re) && m[1] == nickname) {
                std::lock_guard<std::mutex> lock(chan_mtx);
                current_chan = m[2];
                std::cout << "Nyaa~ Joined nya channel: "
                          << current_chan << " 🐾💖\n";
            }

            // 💖 Ping-pong reply
            if (std::strncmp(buf, "PING ", 5) == 0) {
                char *eol = std::strchr(buf, '\r');
                std::string token;
                if (eol) {
                    token.assign(buf + 5, eol - (buf + 5));
                } else {
                    token = std::string(buf + 5);
                }
                send_raw("PONG " + token);
                std::cout << "✨ Pong sent~! Ping pong powaa~! 💫\n";
            }
        }
    });

    // 🌟 UwU Time to chat!! Let the kitty type~ 😽💬
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        if (line[0] == '/') {
            std::string cmd = line.substr(1);
            send_raw(cmd);

            if (cmd.rfind("QUIT", 0) == 0) {
                std::cout << "Bai bai nya~! Leaving with paw-shion~ 💕🐾\n";
                break;
            }
            // JOIN handling is now entirely driven by the reader thread
        }
        else if (!current_chan.empty()) {
            send_raw("PRIVMSG " + current_chan + " :" + line);
        }
        else {
            std::cerr << "Nyaa~! You need to /join a channel first nya! >w<\n";
        }
    }

    // 🧹 Clean up
    if (reader.joinable()) reader.join();
    close(sock);
    std::cout << "UwU bot signing off... nyaaa~! 💖🐱✨\n";
    return 0;
}
