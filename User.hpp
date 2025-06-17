#pragma once
#include "Server.hpp"

class Chatroom;

class User
{
    public:
        User(const std::string &nickname, const std::string &password);
        ~User();
        static void newclient(int client_fd,std::vector<pollfd> &fds);
        void HSwelcome();
        void HSNick(const std::string &oldname, const std::string &newname, std::vector<pollfd> &fds);

        void HSTopicQuery(Chatroom &chatroom, std::vector<pollfd> &fds);
        void HSSetTopic(std::vector<std::string> tokens, Chatroom &chatroom, std::vector<pollfd> &fds);
        int getFD();
        std::string getNickname();
		std::string getHostname();
		std::string getUsername();
        std::string getRealname();
		std::string getModeFlags() const;
        void setNickname(const std::string &nick);
        void setHostname(const std::string &host);
        void setUser(const std::string &user_str);
        void setRegis(bool status);
        bool isRegis();
        void appendToBuffer(const std::string &data);
        bool hasCompleteLine() const;
        std::string extractLine();
        std::string getBuffer() const;
        void appendToSendBuffer(const std::string& data);
        const std::string& getSendBuffer() const;
        bool hasDataToSend() const;
        void consumeSendBuffer(size_t bytes);
        bool isPassValid() const;
        void setPassValid(bool ok);
        Chatroom* getChatroom(const std::string& name);
        void addNewMemberToChatroom(Chatroom* room);
        void removeChatroom(const std::string& name);
        std::map<std::string, Chatroom*>& getChatrooms();
        void leaveAllChatrooms();
        void setInvisible(bool on);
        bool isInvisible() const;
        void markDead();
		bool isDead() const;
        

    private:
        bool _rdyToWrite;
        std::string _sendBuffer;
        std::string _buffer;
        bool _isOP;
        bool _isRegis;
        std::string _username;
        std::string _nickname;
        std::string _password;
		std::string _hostname;
		std::string _realname;
        std::map<std::string, Chatroom*> roomsThisUserIsMemberIn;
		int _FD;
		std::string _auth_state;
        bool _passValid;
        bool        _invisible;
		bool		_dead;
};
