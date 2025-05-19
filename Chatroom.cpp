#include "Chatroom.hpp"
#include "Server.hpp"
void Chatroom::displayTopic()
{
    std::cout << _topic << std::endl;
}

void Chatroom::setTopic(std::string &topicstring)
{
    this->_topic = topicstring;
}


