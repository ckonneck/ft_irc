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

bool Chatroom::isInvited(User* u) const {
    return std::find(invited_to_room.begin(),
                     invited_to_room.end(),
                     u) != invited_to_room.end();
}


bool Chatroom::isMember(User* u) const
{
    return std::find(members_in_room.begin(),
                     members_in_room.end(),
                     u)
           != members_in_room.end();
}

#include <vector>
#include <string>

bool uniqueNick(User* usr)
{
    const std::string& nick = usr->getNickname();
std::cout << "debug1" << std::endl;
    // length must be 1..9
    if (nick.empty() || nick.size() > 9)
        return false;
std::cout << "debug2" << std::endl;
    // check each character
    for (size_t i = 0; i < nick.size(); ++i)
    {
        char c = nick[i];
            // first char: must be in 'A'..'}'
            if (!((c >= 45 && c <= 57) || (c >= 65 && c<= 125)))
				//432 ERR_ERRONEUSNICKNAME "<nick> :Erroneus nickname"
				{
				std::string msg =" 432 ERR_ERRONEUSNICKNAME " + usr->getNickname() +" :Erroneous nickname\r\n";
   			    usr->sendMsg(msg);
				   return false;
				}
				
    }
std::cout << "debug3" << std::endl;
    // now ensure no other user already has that nick
    for (size_t j = 0; j < g_mappa.size(); ++j)
    {
        User* u = g_mappa[j];
        if (u != usr && u->getNickname() == nick)
		{
			//433 ERR_NICKNAMEINUSE "<nick> :Nickname is already in use"
				std::string msg =" 433 ERR_NICKNAMEINUSE " + usr->getNickname() +" :Nickname is already in use\r\n";
   			    usr->sendMsg(msg);
            return false;
		}
    }
	std::cout << "debug4" << std::endl;

    return true;
}



void Chatroom::addOperator(User* u)
{
    // avoid duplicates
    if (!isOperator(u)) {
        operators_of_room.push_back(u);
        // (optional) log it:
        std::cout << "User " << u->getNickname()
                  << " is now operator in " << _channelname
                  << std::endl;
    }
}

void Chatroom::removeOperator(User* u)
{
    // find returns an iterator to the element or end()
    std::vector<User*>::iterator it =
        std::find(operators_of_room.begin(),
                  operators_of_room.end(),
                  u);

    if (it != operators_of_room.end()) {
        operators_of_room.erase(it);
        // (optional) log it:
        std::cout << "User " << u->getNickname()
                  << " removed as operator from "
                  << _channelname << std::endl;
    }
}

bool Chatroom::isOperator(User* u) const
{
    return std::find(operators_of_room.begin(),
                     operators_of_room.end(),
                     u)
           != operators_of_room.end();
}


User* Chatroom::findUserByNick(const std::string& nick)
{
    // explicit iterator loop because C++98 has no range‐for
    for (std::vector<User*>::iterator it = members_in_room.begin();
         it != members_in_room.end(); 
         ++it)
    {
        if ((*it)->getNickname() == nick)
            return *it;
    }
    return NULL;  // use NULL in C++98
}

bool Chatroom::isInviteOnly() const {
    return invite_only;
}


void Chatroom::displayTopic()
{
    std::cout << _topic << std::endl;
}

void Chatroom::addUser(User* user) {
    members_in_room.push_back(user);
   // consume a one-time invite
   invited_to_room.erase(
       std::remove(invited_to_room.begin(),
                   invited_to_room.end(),
                   user),
       invited_to_room.end()
   );
}

void Chatroom::inviteUser(User* u) {
    // avoid duplicate invites
    if (!isInvited(u))
        invited_to_room.push_back(u);
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