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
      members_in_room(),
      invite_only(false),
      topic_only_ops(false),
      key_set(false),
      channel_key(""),
      limit_set(false),
      user_limit(0)
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
   			    usr->appendToSendBuffer(msg);
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
				// std::string msg =" 433 ERR_NICKNAMEINUSE " + usr->getNickname() +" :Nickname is already in use\r\n";
   			    // usr->appendToSendBuffer(msg);
                //maybe givetempnick here?
            return false;
		}
    }
	std::cout << "debug4" << std::endl;

    return true;
}


void Chatroom::addOperator(User* u)
{
    if (!isOperator(u)) {
        operators_of_room.push_back(u);
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
    for (std::vector<User*>::iterator it = members_in_room.begin();
         it != members_in_room.end(); 
         ++it)
    {
        if ((*it)->getNickname() == nick)
        {
            return *it;
        }
    }
    return NULL;
}

bool Chatroom::isInviteOnly() const {
    return invite_only;
}

void Chatroom::uninviteUser(User* u) {
    invited_to_room.erase(
        std::remove(invited_to_room.begin(),
                    invited_to_room.end(),
                    u),
        invited_to_room.end()
    );
}

void Chatroom::setInviteOnly(bool flag) {
    invite_only = flag;

    if (flag) {
        if (_channelmode.find('i') == std::string::npos)
            _channelmode.push_back('i');
    } else {
        _channelmode.erase(
            std::remove(_channelmode.begin(), _channelmode.end(), 'i'),
            _channelmode.end()
        );
    }
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

const char* ascii_kitties[] = {
    " /\\_/\\  \n( o.o ) \n > ^ <  nya~!\n",
    "  |\\---/|\n  | o_o |\n   \\_^_/  UwU\n",
    "  /\\_/\\ \n ( •‿• ) \n / >🍓   Kitty-chan desu~!\n",
    "  ／ﾌﾌ  ╰(=✪ x ✪=)╯\n（　ヽﾉ  \n　＼(＿) \n",
    "  (=^-ω-^=) \n  I iz kawaii~ nyaa~! 🌸\n",
    "  ∧,,,∧\n( ̳• · • ̳)\n/    づ♡ \nKissu~ >3< 💕\n"
};

const int kitty_count = sizeof(ascii_kitties) / sizeof(ascii_kitties[0]);

std::string random_ascii_kitty()
{
    // Initialize the random seed only once
    static bool seeded = false;
    if (!seeded)
    {
        std::srand((unsigned int)std::time(0)); // C++98 style
        seeded = true;
    }

    int index = std::rand() % kitty_count;
    return std::string(ascii_kitties[index]);
}


// void Chatroom::broadcast(const std::string &msg, User *sender)old
// {
//     for (size_t i = 0; i < members_in_room.size(); ++i)
//     {std::cout << members_in_room[i]->getNickname() << std::endl;
//     }
//         for (size_t i = 0; i < members_in_room.size(); ++i) {
//             if (members_in_room[i] != sender) {
//                 members_in_room[i]->appendToSendBuffer(msg);//EXPERIMENTAL
//             }
//         }
//         std::cout << "Broadcast to " << this->_channelname << ": " << msg;
//         std::cout << random_ascii_kitty() << std::endl; // UwU KITTYYYY~!! 🐱💕
// }
// void Chatroom::broadcastToYou(const std::string &msg, User *sender, std::vector<pollfd> &fds)
// {

// }

void Chatroom::broadcast(const std::string &msg, User *sender, std::vector<pollfd> &fds)
{
   // 1) If there is a sender, verify they are in this channel
    if (sender != NULL) {
        bool isMember = false;

        // Look through members_in_room to see if sender is present
        for (std::vector<User*>::size_type i = 0; i < members_in_room.size(); ++i) {
            if (members_in_room[i] == sender) {
                isMember = true;
                break;
            }
        }

        if (!isMember) {
            // Sender is not on this channel → ERR_NOTONCHANNEL (442)
            // Format:  "<servername> 442 <nick> <channel> :You're not on that channel"
            std::string err = ":" + servername
                            + " 442 "
                            + sender->getNickname()
                            + " " + this->_channelname
                            + " :You're not on that channel\r\n";
            sender->appendToSendBuffer(err);

            // Ensure we flag POLLOUT on sender’s fd so the client sees the error
            int sender_fd = sender->getFD();
            for (std::vector<pollfd>::size_type j = 0; j < fds.size(); ++j) {
                if (fds[j].fd == sender_fd) {
                    fds[j].events |= POLLOUT;
                    break;
                }
            }

            return;
        }
    }


         for (size_t i = 0; i < members_in_room.size(); ++i) {
            User* member = members_in_room[i];

            if (member != sender)
            {
                member->appendToSendBuffer(msg);
                int user_fd = members_in_room[i]->getFD();
                for (size_t j = 0; j < fds.size(); ++j)//put debug here idk
                {
                    if (fds[j].fd == user_fd)
                    {
                        fds[j].events |= POLLOUT;
                        break;
                    }
                }
            }
        }
        std::cout << "Broadcast to " << this->_channelname << ": " << msg;
        std::cout << random_ascii_kitty() << std::endl; // UwU KITTYYYY~!! 🐱💕
        for (size_t i = 0; i < members_in_room.size(); ++i)
    {std::cout << members_in_room[i]->getNickname() <<"   "<< members_in_room[i]->getSendBuffer() << std::endl;}
}


void Chatroom::broadcastonce(const std::string &msg, User *sender, std::vector<pollfd> &fds, std::set<int>& alreadyNotifiedFDs)
{
    //just debug message, dont forget to comment out for eval
    for (size_t i = 0; i < members_in_room.size(); ++i)
    {std::cout << members_in_room[i]->getNickname() << std::endl;}


         for (size_t i = 0; i < members_in_room.size(); ++i)
         {
            User* member = members_in_room[i];
            int user_fd = members_in_room[i]->getFD();
            if (member != sender && alreadyNotifiedFDs.find(user_fd) == alreadyNotifiedFDs.end())
            {
                member->appendToSendBuffer(msg);
                alreadyNotifiedFDs.insert(user_fd);
                
                for (size_t j = 0; j < fds.size(); ++j)
                {
                    if (fds[j].fd == user_fd)
                    {
                        fds[j].events |= POLLOUT;
                        break;
                    }
                }
            }
        }
        std::cout << "Broadcast to " << this->_channelname << ": " << msg;
        std::cout << random_ascii_kitty() << std::endl; // UwU KITTYYYY~!! 🐱💕
            //just debug message, dont forget to comment out for eval
        for (size_t i = 0; i < members_in_room.size(); ++i)
    {std::cout << members_in_room[i]->getNickname() <<"   "<< members_in_room[i]->getSendBuffer() << std::endl;}
}
// Chatroom::broadcast with debug logging
void Chatroom::broadcastdb(const std::string &msg,
                         User *sender,
                         std::vector<pollfd> &fds)
{
    std::cout << "[DEBUG] Entering broadcast on channel " << this->_channelname
              << " from sender " << sender->getNickname() << "\n";
    
    // List all members
    std::cout << "[DEBUG] Members in room:\n";
    for (size_t i = 0; i < members_in_room.size(); ++i)
    {
        std::cout << "  - [" << i << "] " 
                  << members_in_room[i]->getNickname()
                  << " (fd=" << members_in_room[i]->getFD() << ")\n";
    }

    // Send to each member except the sender
    for (size_t i = 0; i < members_in_room.size(); ++i)
    {
        User* member = members_in_room[i];
        int user_fd = member->getFD();
        
        if (member == sender)
        {
            std::cout << "[DEBUG] Skipping sender itself: " 
                      << member->getNickname() << "\n";
            continue;
        }

        // Queue the message
        std::cout << "[DEBUG] Queuing message for " 
                  << member->getNickname() 
                  << " (fd=" << user_fd << ")\n";
        member->appendToSendBuffer(msg);

        // Find the matching pollfd and set POLLOUT
        bool found = false;
        for (size_t j = 0; j < fds.size(); ++j)
        {
            if (fds[j].fd == user_fd)
            {
                std::cout << "[DEBUG]   → Found pollfd[" << j << "] for "
                          << user_fd << ", events before: " 
                          << std::hex << fds[j].events << std::dec << "\n";

                fds[j].events |= POLLOUT;

                std::cout << "[DEBUG]   → events after: " 
                          << std::hex << fds[j].events << std::dec << "\n";
                found = true;
                break;
            }
        }
        if (!found)
        {
            std::cout << "[WARN] Could not find pollfd entry for fd " 
                      << user_fd << "\n";
        }
    }

    std::cout << "[DEBUG] Finished broadcast on " << this->_channelname 
              << ", message: " << msg << "\n";
    std::cout << random_ascii_kitty() << std::endl;
}




// t: only ops may set topic
void Chatroom::setTopicOnlyOps(bool on) {
    topic_only_ops = on;
    if (on && _channelmode.find('t') == std::string::npos)
        _channelmode.push_back('t');
    else if (!on)
        _channelmode.erase(
            std::remove(_channelmode.begin(), _channelmode.end(), 't'),
            _channelmode.end());
}

bool Chatroom::isTopicOnlyOps() const {
    return topic_only_ops;
}
// k: channel key/password
void Chatroom::setKey(const std::string& key) {
    channel_key = key;
    key_set = true;
    if (_channelmode.find('k') == std::string::npos)
        _channelmode.push_back('k');
}
void Chatroom::unsetKey() {
    channel_key.clear();
    key_set = false;
    _channelmode.erase(
        std::remove(_channelmode.begin(), _channelmode.end(), 'k'),
        _channelmode.end());
}
bool Chatroom::hasKey() const { 
    return key_set; 
}
const std::string& Chatroom::getKey() const {
    return channel_key;
}
// l: user limit
void Chatroom::setLimit(int limit) {
    user_limit = limit;
    limit_set = true;
    if (_channelmode.find('l') == std::string::npos)
        _channelmode.push_back('l');
}
void Chatroom::unsetLimit() {
    user_limit = 0;
    limit_set = false;
    _channelmode.erase(
        std::remove(_channelmode.begin(), _channelmode.end(), 'l'),
        _channelmode.end());
}
bool Chatroom::hasLimit() const {
    return limit_set;
}
int Chatroom::getLimit() const {
    return user_limit;
}

const std::vector<User*>& Chatroom::getMembers() const
{
    return members_in_room;
}

void User::leaveAllChatrooms()
{
    std::map<std::string, Chatroom*> copy = roomsThisUserIsMemberIn;

    for (std::map<std::string, Chatroom*>::iterator it = copy.begin(); it != copy.end(); ++it)
    {
        Chatroom* room = it->second;
        if (room)
            room->removeUserFromChatroom(this);
    }

    roomsThisUserIsMemberIn.clear();
}

