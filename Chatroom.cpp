#include "Chatroom.hpp"
#include "Server.hpp"


Chatroom::Chatroom(const std::string &name)
    : _topicTime(0),
      _hasTopic(false),
      _password(""),
      _topic(""),
      _channelname(name),
      _channelmode(""),
      _lastTopicSetter(""),
      members_in_room()
{
    this->_channelname = name;
    std::cout << "Chatroom " << name << " has been created.";

}


void Chatroom::displayTopic()
{
    std::cout << _topic << std::endl;
}

void Chatroom::addUser(User *user)
{
    members_in_room.push_back(user);
    std::cout << user->getNickname() << "got pushed backinto the members" << std::endl;
}

void Chatroom::removeUser(User* user)
{
    std::vector<User*>::iterator it = std::find(members_in_room.begin(), members_in_room.end(), user);
    if (it != members_in_room.end())
    {
        members_in_room.erase(it);
    }
}
std::string Chatroom::getTopic()
{
    return this->_topic;
}

std::string Chatroom::getName()
{
    return this->_channelname;
}

std::string Chatroom::getLastTopicSetter()
{
    return this->_lastTopicSetter;
}

bool Chatroom::hasTopic()
{
    return this->_hasTopic;
}

time_t Chatroom::getTopicTime()
{
    return _topicTime;
}

void Chatroom::setTopic(const std::string &topicstring, const std::string &lastsetter)
{
    _topic = topicstring;
    _lastTopicSetter = lastsetter;
    _topicTime = std::time(NULL);  // sets to current time
    _hasTopic = true;
}

void Chatroom::broadcast(const std::string &msg, User *sender)
{
    {
        for (size_t i = 0; i < members_in_room.size(); ++i) {
            if (members_in_room[i] != sender) {
                send_to_client(members_in_room[i]->getFD(), msg);
            }
        }
        std::cout << "Broadcast to " << this->_channelname << ": " << msg;
    }
}