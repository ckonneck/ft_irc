#pragma once
#include "Server.hpp"

class Chatroom;

class User
{
    public:
        User(const std::string &nickname, const std::string &password);
        ~User();
        static void newclient(int client_fd,std::vector<pollfd> &fds);
        void HSwelcome(int &client_fd);
        void HSNick(const std::string &oldname, const std::string &newname);
        void HSKick(const std::string &target);
        void HSInvite(const std::string &whotoinv);
        void HSTopicQuery(Chatroom &chatroom);
        void HSSetTopic(const std::string &topicstring, Chatroom &chatroom);
        void Mode(char &modeChar);
        int getFD();
        std::string getNickname();
		std::string getHostname();
		std::string getUsername();
        void setNickname(const std::string &nick);
        void setHostname(const std::string &host);
        void setUser(const std::string &user_str);
        void setRegis(bool status);
        bool isRegis();
    private:
        bool _isOP;
        bool _isRegis;
        std::string _username;
        std::string _nickname;
        std::string _password;
		std::string _hostname;
		std::string _realname;
		int _FD;
		std::string _auth_state;
};
