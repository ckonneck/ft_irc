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
#include "User.hpp"
#include <ctime>

class Chatroom;



class Chatroom
{
    public:
        void setTopic(const std::string &topicstring, const std::string &lastsetter);
        void displayTopic();
		void broadcast();
        std::string getName();
        std::string getTopic();
        std::string getLastTopicSetter();
        bool hasTopic();
        time_t getTopicTime();
    private:
        time_t _topicTime;
        bool _hasTopic;
        std::string _password;
        std::string _topic;
        std::string _channelname;
        std::string _channelmode;
        std::string _lastTopicSetter;
		std::vector<User*> members_in_room;
};
extern std::vector<User*> g_mappa;
bool serverexit();
void cleanup(std::vector<pollfd> &fds);
void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd);
void welcomemessage();
void messagehandling(std::vector<pollfd> &fds, size_t i);
void validatePort(char *argv);
bool isDigit(char *strnum);
void commandParsing(char *messagebuffer, std::vector<pollfd> &fds, size_t i);
std::vector<std::string> split(const std::string &input, char delimiter);
User* findUserByFD(int fd);
User* findUserByNickname(const std::string& nick);