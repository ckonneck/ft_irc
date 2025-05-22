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

class Chatroom;

class User
{
    public:
        User(const std::string &nickname, const std::string &password);
        ~User();
        static void newclient(int client_fd,std::vector<pollfd> &fds);
        void HSwelcome(int &client_fd);
        void HSNick(const std::string &newname);
        void HSKick(const std::string &target);
        void HSInvite(const std::string &whotoinv);
        void HSTopicQuery(Chatroom &chatroom);
        void HSSetTopic(const std::string &topicstring, Chatroom &chatroom);
        void Mode(char &modeChar);
        int getFD();
        std::string getNickname();
        void setNickname(const std::string &nick);
    private:
        bool _isOP;
        std::string _nickname;
        std::string _password;
		std::string _hostname;
		std::string _realname;
		int _FD;
		std::string _auth_state;
};

class Chatroom
{
    public:
        Chatroom(const std::string &name);
        void setTopic(const std::string &topicstring, const std::string &lastsetter);
        void displayTopic();
		void broadcast(const std::string &msg, User *sender);
        std::string getName();
        std::string getTopic();
        std::string getLastTopicSetter();
        bool hasTopic();
        time_t getTopicTime();
        void addUser(User *user);
        void removeUser(User* user);

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
extern std::map<std::string, Chatroom*> g_chatrooms;
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
std::string parseNick(const std::string &msg);
std::string intToString(int value);
void removeUser(User* target);
void send_to_client(int client_fd, const std::string& message);