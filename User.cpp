#include "Server.hpp"
#include <cerrno>

std::vector<User*> g_mappa;
std::map<std::string, Chatroom*> g_chatrooms;

User::User(const std::string &nickname,const std::string &password) 
    : _username(""), _nickname(nickname), _password(password), _hostname(""), _passValid(false),_invisible(false)
{
    this->_isOP = false;
    std::cout << "User "<< this->_nickname <<" has been created" <<std::endl;
}

User::~User()
{
    std::cout << "User "<< this->_nickname <<" fucked off to somewhere else" <<std::endl;
}
void User::newclient(int server_fd, std::vector<pollfd> &fds)
{
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cout << "strangerdanger" << std::endl;
        }
    }
    else
    {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        pollfd client_pollfd = { client_fd, POLLIN, 0 };
        fds.push_back(client_pollfd);
        std::cout << "New client connected: FD nr " << client_fd << "\n";

        // Create a temporary user with default nickname
        // We'll update the nickname when we receive a NICK command
        User* newUser = new User("","");
        newUser->_FD = client_fd;
        newUser->setRegis(false);
        g_mappa.push_back(newUser);
        std::cout << "we done" <<std::endl;

        // newUser->HSwelcome(client_fd);
    }
    

}

bool User::isPassValid() const 
{ 
    return _passValid; 
} 

void User::setPassValid(bool ok) 
{ 
    _passValid = ok; 
}

// for write() obsolete
// void User::sendMsg(const std::string& msg)
// {
//     int fd = _FD;
//     write(fd, msg.c_str(), msg.size());
// }

void User::setInvisible(bool on)
{
    _invisible = on;
}

bool User::isInvisible() const
{
    return _invisible;
}

std::string parseNick(const std::string &msg)
{
    size_t pos = msg.find("NICK");
    if (pos == std::string::npos)
        return ""; // NICK not found

    // Move past "NICK" and any space
    pos += 4;

    // Skip any whitespace after "NICK"
    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Extract until end of line or space
    size_t end = msg.find_first_of("\r\n ", pos);
    std::string nickname = msg.substr(pos, end - pos);

    return nickname;
}

std::string parseUser(const std::string &msg)
{
    size_t pos = msg.find("USER");
    if (pos == std::string::npos)
        return ""; // USER not found

    pos += 4;
    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Extract the first word after USER (the username)
    size_t end = msg.find_first_of("\r\n ", pos);
    return msg.substr(pos, end - pos);
}

std::string parseHost(const std::string &msg)
{
    size_t pos = msg.find("USER");
    if (pos == std::string::npos)
        return ""; // USER not found

    pos += 4;
    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Skip username (first word)
    pos = msg.find_first_of(" \r\n", pos);
    if (pos == std::string::npos)
        return "";

    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Skip mode/unused (second word)
    pos = msg.find_first_of(" \r\n", pos);
    if (pos == std::string::npos)
        return "";

    while (pos < msg.length() && std::isspace(msg[pos]))
        ++pos;

    // Now we're at the host (third word)
    size_t end = msg.find_first_of("\r\n ", pos);
    return msg.substr(pos, end - pos);
}


void User::HSwelcome()
{
    std::string msg1 = ":localhost 001 " + this->_nickname + " :Welcome to the UWURC server " + this->_nickname + "!" + this->_username + "@" + this->_hostname + "\r\n";
    appendToSendBuffer(msg1);

    std::string msg2 = ":localhost 002 " + this->_nickname + " :Your host is UWUCHAN running version 1.0\r\n";
    appendToSendBuffer(msg2);

    std::string msg3 = ":localhost 003 " + this->_nickname + " :This server was created at " + serverStartTime() + " DA NYA\r\n";
    appendToSendBuffer(msg3);

    std::string msg4 = ":localhost 004 " + this->_nickname + " owo please don't be mean\r\n";
    appendToSendBuffer(msg4);
}

std::string serverStartTime()
{   
    time_t now = std::time(NULL);
    struct tm *t = std::localtime(&now);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%a %b %e %H:%M:%S %Y", t);
    return std::string(buf);
}


void printChatrooms() {
    std::map<std::string, Chatroom*>::iterator it;
    for (it = g_chatrooms.begin(); it != g_chatrooms.end(); ++it) {
        std::cout << "Chatroom name: " << it->first << ", pointer: " << it->second << std::endl;
    }
}

void User::HSNick(const std::string &oldname, const std::string &newname, std::vector<pollfd> &fds)
{
// std::cout << "[DEBUG-HSnick] &fds=" << &fds
//               << ", first_fd=" << (fds.empty() ? -1 : fds[0].fd)
//               << ", size=" << fds.size() << "\n";
    std::string nickMsg = ":" + oldname + "!user@localhost NICK :" + newname + "\r\n";
    appendToSendBuffer(nickMsg);//maybe this needs to go, maybe not
    std::set<int> alreadyNotifiedFDs;
    std::map<std::string, Chatroom*>::iterator it;
    for (it = roomsThisUserIsMemberIn.begin(); it != roomsThisUserIsMemberIn.end(); ++it)
    {
        std::string channelName = it->first;
        Chatroom* room = it->second;
        if (room)
        {
            it->second->broadcastonce(nickMsg, this, fds, alreadyNotifiedFDs);
        }
    }
}


void User::HSNickdb(const std::string &oldname,
                  const std::string &newname,
                  std::vector<pollfd> &fds)
{
    std::string nickMsg = ":" + oldname
                        + "!user@localhost NICK :" 
                        + newname 
                        + "\r\n";

    std::cout << "[DEBUG] HSNick called for user fd=" 
              << this->getFD()
              << ", oldnick=" << oldname 
              << ", newnick=" << newname 
              << "\n";

    // Broadcast into each room the user is actually in
    for (std::map<std::string, Chatroom*>::iterator it = g_chatrooms.begin();
         it != g_chatrooms.end();
         ++it)
    {
        Chatroom* room = it->second;

        // Only broadcast if this user is a member
        const std::vector<User*>& members = room->getMembers();
        bool in_room = false;
        for (size_t i = 0; i < members.size(); ++i)
        {
            if (members[i] == this)
            {
                in_room = true;
                break;
            }
        }
        if (in_room)
        {
            std::cout << "[DEBUG] Broadcasting nick change to room "
                      << it->first << "\n";
            room->broadcastdb(nickMsg, this, fds);
        }
        else
        {
            std::cout << "[DEBUG] Skipping room " << it->first 
                      << ": user not present\n";
        }
    }

    // Also send directly to self
    std::cout << "[DEBUG] Queuing nick change for self (fd=" 
              << this->getFD() << ")\n";
    appendToSendBuffer(nickMsg);

    // And set POLLOUT for self
    int my_fd = this->getFD();
    bool found = false;
    for (size_t i = 0; i < fds.size(); ++i)
    {
        if (fds[i].fd == my_fd)
        {
            std::cout << "[DEBUG] Setting POLLOUT on self pollfd[" 
                      << i << "], before=" 
                      << std::hex << fds[i].events << std::dec << "\n";
            fds[i].events |= POLLOUT;
            std::cout << "[DEBUG]             after=" 
                      << std::hex << fds[i].events << std::dec << "\n";
            found = true;
            break;
        }
    }
    if (!found)
    {
        std::cout << "[WARN] Could not find pollfd entry for self fd " 
                  << my_fd << "\n";
    }
}



User* findUserByFD(int fd)
{
    // std::cout << "Starting search for user with FD: " << fd << std::endl;
    for (size_t i = 0; i < g_mappa.size(); ++i)
    {
        User* user = g_mappa[i];

        // Debug print of nickname and FD being checked
        // std::cout << "Checking user: " << user->getNickname()
        //           << " (FD: " << user->getFD() << ")" << std::endl;

        if (user->getFD() == fd) {
            // std::cout << "Match found: " << user->getNickname() << std::endl;
            return user;
        }
    }
    // std::cout << "No user found for FD: " << fd << std::endl;
    return NULL;
}


User* findUserByNickname(const std::string& nick)
{
    for (size_t i = 0; i < g_mappa.size(); ++i)
    {
        User* user = g_mappa[i];
        std::cout << "found: " << user->getNickname() << std::endl;
        if (user->getNickname() == nick)
            return user;
    }
    std::cout << "returning NULL" << std::endl;
    return NULL;
}


void User::HSInvite(const std::string &whotoinv)
{
    //check if user has invite rights i guess
    //server sends response back to the client 
    std::string channel = "yeanoidea";//parsing plss
    std::string msg = ":localhost 341 " + this->_nickname + " " + whotoinv + " " + channel + "\r\n";
    appendToSendBuffer(msg);

    //server sends message to the person being invited
    std::string msg2 = ":" + this->_nickname + "!user@localhost INVITE " + whotoinv + " :" + channel + "\r\n";
    User *targetuser = findUserByNickname(whotoinv);
    targetuser->appendToSendBuffer(msg2);
}

void User::HSTopicQuery(Chatroom &chatroom, std::vector<pollfd> &fds)//this is for when client
//sends: /Topic #channelname
{
    if (chatroom.hasTopic() == true) {
        std::string topic = chatroom.getTopic();
        printStringHex(topic);
        std::string setter = chatroom.getLastTopicSetter();
        std::ostringstream oss;
        oss << chatroom.getTopicTime();
        std::string timestamp = oss.str();

        std::string msg332 = ":localhost 332 " + this->_nickname + " " + chatroom.getName() + " :" + topic + "\r\n";
        appendToSendBuffer(msg332);

        std::string msg333 = ":localhost 333 " + this->_nickname + " " + chatroom.getName() + " " + setter + " " + timestamp + "\r\n";
        appendToSendBuffer(msg333);
    } else {

        
        // std::string msg = ":localhost 331 " + this->_nickname + " " + chatroom.getName() + " :No topic is set\r\n";
        (void)fds;
        // chatroom.broadcastToYou(msg, this, fds);
        //appendToSendBuffer(msg);
    }
}

void User::HSSetTopic(std::vector<std::string> tokens, Chatroom &chatroom, std::vector<pollfd> &fds)
{
    std::string topicstring = extractAfterHashBlock(tokens);
    topicstring.erase(0,1);
    if (chatroom.isOperator(this) || chatroom.isTopicOnlyOps() == false) // set MODE Flag
    {
        chatroom.setTopic(topicstring, this->_nickname);
        std::string topicChangeMsg = ":" + this->_nickname + "!" + this->_username + "@" + this->_hostname +
                             " TOPIC " + chatroom.getName() + " :" + topicstring + "\r\n";

        chatroom.broadcast(topicChangeMsg, NULL, fds);
    }
    else
        appendToSendBuffer("You don't have the rights to set the topic.\n");
}

void User::setNickname(const std::string &nick)
{
    this->_nickname = nick;
}
 void User::setHostname(const std::string &host)
 {
    this->_hostname = host;
 }
void User::setUser(const std::string &user_str)
{
    this->_username = user_str;
}

std::string User::getHostname()
{
    return this->_hostname;
}

std::string User::getUsername()
{
    return this->_username;
}

void User::appendToBuffer(const std::string &data)
{
    this->_buffer += data;
}

bool User::hasCompleteLine() const
{
    return _buffer.find("\r\n") != std::string::npos;
}

std::string User::extractLine()
{
    size_t pos = _buffer.find("\r\n");
    if (pos == std::string::npos)
        return "";

    std::string line = _buffer.substr(0, pos);
    _buffer = _buffer.substr(pos + 2); // remove the processed line
    return line;
}

std::string User::getBuffer() const { return _buffer; }

bool User::hasDataToSend() const
{
    return !_sendBuffer.empty();
}

const std::string& User::getSendBuffer() const
{
    return _sendBuffer;
}

void User::appendToSendBuffer(const std::string& data)
{
    _sendBuffer += data;
}

void User::consumeSendBuffer(size_t bytes)
{
    // std::cout << "erasing sendbuffer for " << this->_FD << std::endl;
    _sendBuffer.erase(0, bytes);
}

bool User::isRegis()
{
    return (this->_isRegis);
}

void User::setRegis(bool status)
{
    this->_isRegis = status;
}

void removeUser(User* target) {
 
    if (!target) return; 
    for (std::map<std::string, Chatroom*>::iterator mit = g_chatrooms.begin();
         mit != g_chatrooms.end(); ++mit)
    {
        Chatroom* chan = mit->second;
        if (chan->isMember(target))
            chan->removeUser(target);
        if (chan->isOperator(target))
            chan->removeOperator(target);
        chan->uninviteUser(target);
    }

    std::vector<User*>::iterator uit
        = std::find(g_mappa.begin(), g_mappa.end(), target);
        if (uit != g_mappa.end())
        {
        delete *uit;
        g_mappa.erase(uit);
    }
}

void join_channel(int client_fd, const std::string& nickname, const std::string& channel) {
    // Simulate JOIN message
    User * us = findUserByFD(client_fd);
    std::string msg1 =  ":" + nickname + "!" + nickname + "@localhost JOIN :" + channel + "\r\n";
    us->appendToSendBuffer(msg1);

    // RPL_NAMREPLY (353): list of users in channel
    std::string msg2 = ":localhost 353 " + nickname + " = " + channel + " :" + nickname + "\r\n";
    us->appendToSendBuffer(msg2);

    // RPL_ENDOFNAMES (366): end of names list
    std::string msg3 = ":localhost 366 " + nickname + " " + channel + " :End of /NAMES list." + "\r\n";
    us->appendToSendBuffer(msg3);
}

int User::getFD()
{
    return this->_FD;
}

std::string User::getNickname()
{
    return this->_nickname;
}

void User::addNewMemberToChatroom(Chatroom* room)
{
    if (room)
        roomsThisUserIsMemberIn[room->getName()] = room;
}

Chatroom* User::getChatroom(const std::string& name)
{
    std::map<std::string, Chatroom*>::iterator it = roomsThisUserIsMemberIn.find(name);
    if (it != roomsThisUserIsMemberIn.end())
        return it->second;
    return NULL;
}

void User::removeChatroom(const std::string& name)
{
    std::map<std::string, Chatroom*>::iterator it = roomsThisUserIsMemberIn.find(name);
    if (it != roomsThisUserIsMemberIn.end())
        roomsThisUserIsMemberIn.erase(it);
}

std::map<std::string, Chatroom*>& User::getChatrooms()
{
    return roomsThisUserIsMemberIn;
}


void Chatroom::removeUserFromChatroom(User* user)
{
if (!user)
        return;

    for (std::vector<User*>::iterator it = members_in_room.begin(); it != members_in_room.end(); ++it)
    {
        if (*it == user)
        {
            members_in_room.erase(it);
            break;
        }
    }

    user->removeChatroom(this->getName());
}