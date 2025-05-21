#include "Chatroom.hpp"
#include "Server.hpp"


Chatroom::Chatroom(const std::string &name)
{
    std::cout << "Chatroom " << name << " has been created.";
}


void Chatroom::displayTopic()
{
    std::cout << _topic << std::endl;
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

void Chatroom::broadcast(const std::string &msg)
{
    std::cout << "this needs to go to everyone\n" << msg << std::endl;
}