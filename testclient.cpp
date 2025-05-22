// 🌸 Nyaa~ Welcome to the cutest IRC Bot ever~ UwU!! 🐾✨
// 🐱 This bot connects to an IRC server and meow-nitors messages~ nya~ 💬💕
// By the power of kawaii, let's chat~!! ^w^ 🎀

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

int main() {
    // 🌈 Server infos~ Meowster setup!!! 🐾💖
    const char* server   = "127.0.0.1"; // (≧◡≦) Local kitty server!
    const char* port     = "6667";      // Nyaa~ your local IRC server port! :3

    // 🌸 Our bot's adorable identity nya~!!
    const std::string nickname = "TestBotUwU";
    const std::string username = "uwubot";
    const std::string realname = "UwU Cat-Bot desu~ 🐱✨";

    // 🌼 Let's resolve the server address~! nyaaan~ 🌐
    addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int gai = getaddrinfo(server, port, &hints, &res);
    if (gai != 0) {
        std::cerr << "Oh noes~ >w< getaddrinfo failed: " << gai_strerror(gai) << " 😿\n";
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
        if (::send(sock, out.data(), out.size(), 0) < 0) {
            std::perror("Meowww~ sending failed nya >w<");
        }
    };

    // 🐾 Time to say hewwo to da server~!! Nyaa! :3
    send_raw("NICK " + nickname);
    send_raw("USER " + username + " 0 * :" + realname);

    // 🐱 Listening for messages like a curious kitten~!! 💌🐾
    std::thread reader([&]() {
        char buf[512];
        while (true) {
            ssize_t n = recv(sock, buf, sizeof(buf)-1, 0);
            if (n < 0) {
                std::perror("Nyaa~ couldn't receive message 😿");
                break;
            }
            if (n == 0) {
                std::cerr << "Oh noes~ the server has left the cat café... T^T\n";
                break;
            }
            buf[n] = '\0';
            std::cout << buf;

            // 💖 Ping pong time~ like a game! (o^▽^o) nya~!
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
    std::string line, current_chan;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        if (line[0] == '/') {
            std::string cmd = line.substr(1);
            send_raw(cmd);

            if (cmd.rfind("JOIN ", 0) == 0) {
                current_chan = cmd.substr(5);
                std::cout << "Nyaa~ Joined nya channel: " << current_chan << " 🐾💖\n";
            }
            if (cmd.rfind("QUIT", 0) == 0) {
                std::cout << "Bai bai nya~! Leaving with paw-shion~ 💕🐾\n";
                break;
            }
        }
        else if (!current_chan.empty()) {
            send_raw("PRIVMSG " + current_chan + " :" + line);
        }
        else {
            std::cerr << "Nyaa~! You need to /join a channel first nya! >w<\n";
        }
    }

    // 🧹 Time to clean up the kitty litter! I mean... IRC session~ nyaaa~ 🧼🐾
    if (reader.joinable()) reader.join();
    close(sock);
    std::cout << "UwU bot signing off... nyaaa~! 💖🐱✨\n";
    return 0;
}
