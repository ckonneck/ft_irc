#include "Chatroom.hpp"
#include "Server.hpp"
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