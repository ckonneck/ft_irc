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
class Chatroom;

class OP
{
    public:
        OP(std::string &nickname, std::string &password);
        ~OP();
        void Kick(std::string &target);
        void Invite(std::string &whotoinv);
        void Topic(std::string &topicstring, Chatroom &name);
        void Mode(char &modeChar);
    protected:
        bool _isOP;
        std::string _nickname;
        std::string _password;
		std::string _hostname;
		std::string _realname;
		std::string _FD;
		std::string _auth_state;
};

class User : public OP
{
    public:
        User(std::string &nickname, std::string &password);
        ~User();
        static void newclient(int &server_fd, std::vector<pollfd> &fds);
        
};

class Chatroom
{
    public:
        void setTopic(std::string &topicstring);
        void displayTopic();
		void broadcast();
    private:
        std::string _password;
        std::string _topic;
        std::string channelname;
        std::string channelmode;
		int			members_in_room;
};

void newclient(int &server_fd, std::vector<pollfd> &fds);
bool serverexit();
void cleanup(std::vector<pollfd> &fds);
void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd);
void welcomemessage();
void messagehandling(std::vector<pollfd> &fds, size_t i);
void validatePort(char *argv);
bool isDigit(char *strnum);
void commandParsing(char *messagebuffer, std::vector<pollfd> &fds, size_t i);
std::vector<std::string> split(const std::string &input, char delimiter);