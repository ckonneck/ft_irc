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


class Chatroom;

class OP
{
    public:
        OP(const std::string &nickname,const std::string &password);
        ~OP();
        void Kick(std::string &target);
        void Invite(std::string &whotoinv);
        void Topic(std::string &topicstring, Chatroom &name);
        void Mode(char &modeChar);
        int getFD();
        std::string getNickname();
    protected:
        bool _isOP;
        std::string _nickname;
        std::string _password;
		std::string _hostname;
		std::string _realname;
		int _FD;
		std::string _auth_state;
        
};

class User : public OP
{
    public:
        User(const std::string &nickname, const std::string &password);
        ~User();
        static void newclient(int &server_fd, std::vector<pollfd> &fds);
        void HSwelcome(int &client_fd);
        void HSNick(const std::string &newname);
        
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