#include "User.hpp"

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
    std::cout << "User "<< this->_nickname <<" has been destroyed" <<std::endl;
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

        User* newUser = new User("","");
        newUser->_FD = client_fd;
        newUser->setRegis(false);
        g_mappa.push_back(newUser);

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
    std::string msg1 = ":server-chan 001 " + this->_nickname + " :Welcome to the UWURC server " + this->_nickname + "!" + this->_username + "@" + this->_hostname + "\r\n";
    appendToSendBuffer(msg1);

    std::string msg2 = ":server-chan 002 " + this->_nickname + " :Your host is UWUCHAN running version 1.0\r\n";
    appendToSendBuffer(msg2);

    std::string msg3 = ":server-chan 003 " + this->_nickname + " :This server was created at " + serverStartTime() + " DA NYA\r\n";
    appendToSendBuffer(msg3);

    std::string msg4 = ":server-chan 004 " + this->_nickname + " owo please don't be mean\r\n";
    appendToSendBuffer(msg4);

	std::string msg5 = ":server-chan 005 " + this->_nickname + " enter password if set up, by typing \" /quote PASS <password>\"\r\n";
    appendToSendBuffer(msg5);


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

    std::string nickMsg = ":" + oldname + "!user@server-chan NICK :" + newname + "\r\n";
    appendToSendBuffer(nickMsg);
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


User* findUserByFD(int fd)
{
    for (size_t i = 0; i < g_mappa.size(); ++i)
    {
        User* user = g_mappa[i];
        if (user->getFD() == fd) {
            return user;
        }
    }
    return NULL;
}


User* findUserByNickname(const std::string& nick)
{

    for (size_t i = 0; i < g_mappa.size(); ++i)
    {
        User* user = g_mappa[i];
        if (user->getNickname() == nick)
            return user;
    }
    return NULL;
}


void User::HSTopicQuery(Chatroom &chatroom, std::vector<pollfd> &fds)
{
    if (chatroom.hasTopic() == true) {
        std::string topic = chatroom.getTopic();
        std::string setter = chatroom.getLastTopicSetter();
        std::ostringstream oss;
        oss << chatroom.getTopicTime();
        std::string timestamp = oss.str();

        std::string msg332 = ":server-chan 332 " + this->_nickname + " " + chatroom.getName() + " :" + topic + "\r\n";
        appendToSendBuffer(msg332);

        std::string msg333 = ":server-chan 333 " + this->_nickname + " " + chatroom.getName() + " " + setter + " " + timestamp + "\r\n";
        appendToSendBuffer(msg333);
    } else {

        
        std::string msg = ":server-chan 331 " + this->_nickname + " " + chatroom.getName() + " :No topic is set\r\n";
        (void)fds;
        appendToSendBuffer(msg);
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
    _buffer = _buffer.substr(pos + 2);
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

std::string User::getRealname()
{
    return _realname;
}

void User::consumeSendBuffer(size_t bytes)
{
    // std::cout << _sendBuffer << std::endl;
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

    User * us = findUserByFD(client_fd);
    std::string msg1 =  ":" + nickname + "!" + nickname + "@server-chan JOIN :" + channel + "\r\n";
    us->appendToSendBuffer(msg1);

    std::string msg2 = ":server-chan 353 " + nickname + " = " + channel + " :" + nickname + "\r\n";
    us->appendToSendBuffer(msg2);

    std::string msg3 = ":server-chan 366 " + nickname + " " + channel + " :End of /NAMES list." + "\r\n";
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

std::string User::getModeFlags() const {
    std::string out;
    if (this->isInvisible()) out.push_back('i');
    if (this->_isOP)         out.push_back('o');
    return out;
}


User* findUserByNicknameInsensitive(const std::string& nick, User* self)
{
    std::string loweredNick = putAllLowCase(nick);
    for (size_t i = 0; i < g_mappa.size(); ++i)
    {
        User* user = g_mappa[i];
        if (user == self)
            continue;
        if (putAllLowCase(user->getNickname()) == loweredNick)
            return user;
    }
    return NULL;
}