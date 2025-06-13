#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <map>
#include <ctime>
#include <algorithm>
#include <cerrno>
#include <set>
#include <string>
#include "Chatroom.hpp"
#include "User.hpp"
#include "Parsing.hpp"
class Chatroom;

extern std::map<std::string, Chatroom*> g_chatrooms;
extern std::vector<User*> g_mappa;
extern std::string servername;

class PasswordManager {
public:
    static void setPassword(const std::string& pw);
    static const std::string& getPassword();
private:
    static std::string _password;
};

int polling(User *user, std::vector<pollfd> &fds, size_t &i);
void leParse(User *user, char *buffer, std::vector<pollfd> &fds, size_t &i);
void disconnect(std::vector<pollfd> &fds, size_t &i);
bool serverexit();
void cleanup(std::vector<pollfd> &fds);
void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd);
void welcomemessage();
void validatePort(char *argv);
bool isDigit(char *strnum);
void commandParsing(const std::string &messagebuffer, std::vector<pollfd> &fds, size_t i);
std::vector<std::string> split(const std::string &input, char delimiter);
User* findUserByFD(int fd);
User* findUserByNickname(const std::string& nick);
std::string intToString(int value);
void removeUser(User* target);
std::string sanitize(const std::string& str);
std::string parseNick(const std::string &msg);
std::string parseUser(const std::string &msg);
std::string parseHost(const std::string &msg);
void join_channel(int client_fd, const std::string& nickname, const std::string& channel);
void registrationParsing(User *user, std::string msg);
void debugPrintPolloutSendBuffers(const std::vector<pollfd>& fds, const std::vector<User*>& users);
void printStringHex(const std::string& str);
std::string extractAfterHashBlock(const std::vector<std::string>& words);
std::string serverStartTime();
User* findUserByNicknameInsensitive(const std::string& nick, User* self);
