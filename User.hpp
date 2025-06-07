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
                void HSNickdb(const std::string &oldname, const std::string &newname, std::vector<pollfd> &fds);

        void HSInvite(const std::string &whotoinv);
        void HSTopicQuery(Chatroom &chatroom, std::vector<pollfd> &fds);
        void HSSetTopic(std::vector<std::string> tokens, Chatroom &chatroom, std::vector<pollfd> &fds);
        void Mode(char &modeChar);
        int getFD();
        std::string getNickname();
		std::string getHostname();
		std::string getUsername();
        std::string getRealname();
        void setNickname(const std::string &nick);
        void setHostname(const std::string &host);
        void setUser(const std::string &user_str);
        void setRegis(bool status);
        bool isRegis();
		void sendMsg(const std::string& msg);
        void appendToBuffer(const std::string &data);
        bool hasCompleteLine() const;
        std::string extractLine();
        std::string getBuffer() const;
        void appendToSendBuffer(const std::string& data);
        const std::string& getSendBuffer() const;
        bool hasDataToSend() const;
        void consumeSendBuffer(size_t bytes);
        bool get_rdyToWrite();
        void set_rdyToWrite(bool);
        bool isPassValid() const;
        void setPassValid(bool ok);
        Chatroom* getChatroom(const std::string& name);
        void addNewMemberToChatroom(Chatroom* room);
        void removeChatroom(const std::string& name);
        std::map<std::string, Chatroom*>& getChatrooms();
        void leaveAllChatrooms();
        void setInvisible(bool on);
        bool isInvisible() const;
        

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
};
