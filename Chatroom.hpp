#pragma once
#include "Server.hpp"

class User;

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