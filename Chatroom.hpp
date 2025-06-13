#pragma once
#include "Server.hpp"
#include <set>
class User;

class Chatroom
{
    public:
        Chatroom(const std::string &name);
        void setTopic(const std::string &topicstring, const std::string &lastsetter);
		void broadcast(const std::string &msg, User *sender, std::vector<pollfd> &fds);
        void broadcastdb(const std::string &msg, User *sender, std::vector<pollfd> &fds);
        void broadcastToYou(const std::string &msg, User *sender, std::vector<pollfd> &fds);

        std::string getName();

        std::string getTopic();
        std::string getLastTopicSetter();
        bool hasTopic();
        time_t getTopicTime();
        void setTopicOnlyOps(bool on);
        bool isTopicOnlyOps() const;

        void addUser(User *user);
        void removeUser(User* user);
		bool isMember(User* u) const;
        bool isOperator(User* u) const;
        User* findUserByNick(const std::string& nick);
        void addOperator(User* u);
        void removeOperator(User* u);

        bool isInvited(User* u) const;
        void inviteUser(User* u);
        bool isInviteOnly() const;
        void uninviteUser(User* u);
        void setInviteOnly(bool on);
        
        void setLimit(int max);
        void unsetLimit();
        bool hasLimit() const;
        int  getLimit() const;

        void setKey(const std::string& key);
        void unsetKey();
        bool hasKey() const;
        const std::string& getKey() const;

        const std::vector<User*>& getMembers() const;
        void removeUserFromChatroom(User *user);
        void broadcastonce(const std::string &msg, User *sender, std::vector<pollfd> &fds, std::set<int>& alreadyNotifiedFDs);
        void passOperatorOn(User *partinguser, std::vector<pollfd>& fds);
        void checkIfEmpty();
        bool isEmpty() const;


    private:
        time_t _topicTime;
        bool _hasTopic;
        std::string _password;
        std::string _topic;
        std::string _channelname;
        std::string _channelmode;
        std::string _lastTopicSetter;
		std::vector<User*> members_in_room;
		std::vector<User*> operators_of_room;
        std::vector<User*> invited_to_room;
        bool invite_only;
        bool topic_only_ops;
        bool key_set;
        std::string channel_key;
        bool limit_set;
        int  user_limit;
};

class PrivateChatroom : public Chatroom {
public:
    PrivateChatroom(User* a, User* b);
    virtual ~PrivateChatroom();

    virtual void   setLimit    (int limit);
    virtual void   unsetLimit  ();
    virtual void   setKey      (const std::string& key);
    virtual void   unsetKey    ();
    virtual void   setInviteOnly(bool flag);
    virtual void   setTopicOnlyOps(bool flag);

};
