#include "Chatroom.hpp"

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

bool uniqueNick(User* user)
{
    const std::string& currentNick = user->getNickname();
    if (currentNick.empty())
        return false;

    std::string nick = currentNick;
    if (nick.size() > MAX_NICK_LEN) {
        std::string truncated = nick.substr(0, MAX_NICK_LEN);
        user->setNickname(truncated);

        // Notify about truncation
        std::ostringstream notice;
        notice << ":" << servername
               << " NOTICE " << truncated
               << " :Your nickname has been truncated to " << truncated
               << "\r\n";
        user->appendToSendBuffer(notice.str());

        nick = truncated;
    }

    for (std::vector<User*>::const_iterator it = g_mappa.begin();
         it != g_mappa.end(); ++it)
    {
        User* other = *it;
        if (other == user) 
            continue;
        if (other->getNickname() == nick) {
            std::ostringstream err;
            err << ":" << servername
                << " 433 " << user->getNickname()
                << " "   << nick
                << " :Nickname is already in use"
                << "\r\n";
            user->appendToSendBuffer(err.str());
            return false;
        }
    }

    // 4) All good
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
    std::vector<User*>::iterator it =
        std::find(operators_of_room.begin(),
                  operators_of_room.end(),
                  u);

    if (it != operators_of_room.end()) {
        operators_of_room.erase(it);
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

void Chatroom::passOperatorOn(User *partinguser, std::vector<pollfd>& fds)
{
    if (this->isOperator(partinguser) == false)
        return;
    for (std::vector<User*>::iterator it = operators_of_room.begin(); it != operators_of_room.end(); ++it)
    {
        if (*it == partinguser)
        {
            removeOperator(partinguser);
            break;
        }
    }
    if (operators_of_room.empty())
    {
        // If no operators left, assign operator to first member of the room if any
        if (!members_in_room.empty())
        {

            User* newOp = members_in_room[0];
            this->addOperator(newOp);
            std::ostringstream msg;
            msg << ":" << servername << " NOTICE " << this->getName()
                << " :" << newOp->getNickname()
                << " is now a channel operator (set by "
                << partinguser->getNickname() << ")\r\n";
            this->broadcast(msg.str(), NULL, fds);
            std::ostringstream modeMsg;
            modeMsg << ":" << servername << " MODE " << this->getName() << " +o " << newOp->getNickname() << "\r\n";
            this->broadcast(modeMsg.str(), NULL, fds);

        }
    }
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
            std::string err = ":" + servername
                            + " 442 "
                            + sender->getNickname()
                            + " " + this->_channelname
                            + " :You're not on that channel\r\n";
            sender->appendToSendBuffer(err);

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
        //std::cout << "Broadcast to " << this->_channelname << ": " << msg;
        std::cout << random_ascii_kitty() << std::endl; // UwU KITTYYYY~!! 🐱💕
        for (size_t i = 0; i < members_in_room.size(); ++i)
    {std::cout << members_in_room[i]->getNickname() <<"   "<< members_in_room[i]->getSendBuffer() << std::endl;}
}


void Chatroom::broadcastonce(const std::string &msg, User *sender, std::vector<pollfd> &fds, std::set<int>& alreadyNotifiedFDs)
{
    //just debug message, dont forget to comment out for eval
    // for (size_t i = 0; i < members_in_room.size(); ++i)
    // {std::cout << members_in_room[i]->getNickname() << std::endl;}


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
        //std::cout << "Broadcast to " << this->_channelname << ": " << msg;
        std::cout << random_ascii_kitty() << std::endl; // UwU KITTYYYY~!! 🐱💕
            //just debug message, dont forget to comment out for eval
    //     for (size_t i = 0; i < members_in_room.size(); ++i)
    // {std::cout << members_in_room[i]->getNickname() <<"   "<< members_in_room[i]->getSendBuffer() << std::endl;}
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

void Chatroom::checkIfEmpty()
{
    for (std::map<std::string, Chatroom*>::iterator it = g_chatrooms.begin(); it != g_chatrooms.end(); )
    {
        Chatroom* room = it->second;
        if (room->isEmpty())
        {
            std::cout << room->getName() << " is empty, deleting it." << std::endl;
            delete room;
            g_chatrooms.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

bool Chatroom::isEmpty() const
{
    return members_in_room.empty();
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

// private chatroom (DM)

static std::string makeDMName(User* x, User* y) {
    std::string n1 = x->getNickname();
    std::string n2 = y->getNickname();
    if (n1 < n2) return "#dm_" + n1 + "_" + n2;
    return "#dm_" + n2 + "_" + n1;
}

PrivateChatroom::PrivateChatroom(User* a, User* b)
  : Chatroom(makeDMName(a,b))
{
    addUser(a);
    addUser(b);
    addOperator(a);
    addOperator(b);

    Chatroom::setLimit(2);
}

PrivateChatroom::~PrivateChatroom() {}

void PrivateChatroom::setLimit(int)           { /* no-op */ }
void PrivateChatroom::unsetLimit()            { /* no-op */ }
void PrivateChatroom::setKey(const std::string&){ /* no-op */ }
void PrivateChatroom::unsetKey()              { /* no-op */ }
void PrivateChatroom::setInviteOnly(bool)     { /* no-op */ }
void PrivateChatroom::setTopicOnlyOps(bool)   { /* no-op */ }