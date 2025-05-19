#include "Server.hpp"

OP::OP(std::string &nickname, std::string &password) :
            _isOP(true), _nickname(nickname), _password(password)
{
    std::cout << "Operator has been created" << std::endl;
}

OP::~OP()
{
    std::cout << "OP "<< this->_nickname <<" fucked off to somewhere else" <<std::endl;
}

void OP::Kick(std::string &target)
{
    if (this->_isOP != true)
    {
        std::cout<<  this->_nickname << " doesn't have the rights to kick "<< target << std::endl;
    }
    //if(searching through available nicknames returns a match)
    //actually kick
}

void OP::Invite(std::string &whotoinv)
{
    if (this->_isOP != true)
    {
        std::cout<< whotoinv << " doesn't exist" << std::endl;
    }
    //if(searching through available nicknames returns a match)
    //actually invite
}

void OP::Topic(std::string &topicstring, Chatroom &name)
{

    if (this->_isOP != true)
    {
        std::cout<< "initiating topicchange" << std::endl;
        name.setTopic(topicstring);
    }
    else
    {
        std::cout << "current Topic is: " << std::endl;
        name.displayTopic();
    }
}

void OP::Mode(char &modeChar)//could just do a map tbh.
{
    if (this->_isOP != true)
    {
        std::cout<< "initiating modechange" << std::endl;
        if (modeChar == 'i')
        {
            //set/remove invite only channel
        }
        else if(modeChar == 't')
        {
            //set/remove restrictions of topic command to channel op's
        }
        else if(modeChar == 'k')
        {
            //set/remove channel password
        }
        else if(modeChar == 'o')
        {
            //give/take channel op rights
        }
        else if(modeChar == 'l')
        {
            //set/remove userlimit on channel
        }
        else
        {
            std::cout << "invalid mode" << std::endl;
        }
    }
    else
        std::cout << "get some rights pleb." << std::endl;
}

