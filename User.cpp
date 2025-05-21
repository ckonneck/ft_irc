#include "User.hpp"
#include "Server.hpp"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

std::vector<User*> g_mappa;

User::User(const std::string& nickname, const std::string& password)
  : hasPass(false)
  , hasNick(false)
  , hasUser(false)
  , _FD(-1)
  , _nickname(nickname)
  , _password(password)
  , _realName("")
  , _isOp(false)
  , _registered(false)
{
    std::cout << "User " << _nickname << " has been created" << std::endl;
}

User::~User() {
    std::cout << "User " << _nickname << " disconnected" << std::endl;
}

bool User::isRegistered() const       { return _registered; }
void User::setRegistered(bool r)      { _registered = r; }

int  User::getFD() const              { return _FD; }
void User::setFD(int fd)              { _FD = fd; }

const std::string& User::getNickname() const { return _nickname; }
const std::string& User::getPassword() const { return _password; }
const std::string& User::getRealName() const { return _realName; }
void User::setRealName(const std::string& rn) { _realName = rn; }

bool User::isOp() const               { return _isOp; }
void User::setOp(bool op)             { _isOp = op; }

void User::Mode(char &modeChar) {
    // implement mode changes
    (void)modeChar;  // avoid unused-parameter warning
}

void User::newclient(int server_fd, std::vector<pollfd>& fds) {
    sockaddr_in client_addr;
    socklen_t   addrlen = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addrlen);
    if (client_fd < 0) {
        std::cerr << "accept() failed: " << strerror(errno) << '\n';
        return;
    }

    // Make it non-blocking
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    // Add to poll fds
    pollfd pfd;
    pfd.fd      = client_fd;
    pfd.events  = POLLIN;
    pfd.revents = 0;
    fds.push_back(pfd);

    // Create a new unregistered User placeholder
    User* user = new User("", "");   // empty nick/password for now
    user->setFD(client_fd);
    g_mappa.push_back(user);

    std::cout << "New unregistered client on FD " << client_fd << std::endl;
}

void User::HSwelcome() {
    // TODO: implement welcome numeric replies
}

void User::HSNick(const std::string& newnick) {
    (void)newnick;
}

void User::HSKick(const std::string& target) {
    (void)target;
}

void User::HSInvite(const std::string& whom) {
    (void)whom;
}

void User::HSTopicQuery(Chatroom& room) {
    (void)room;
}

User* User::findUserByFD(int fd) {
    for (size_t i = 0; i < g_mappa.size(); ++i) {
        if (g_mappa[i]->_FD == fd)
            return g_mappa[i];
    }
    return NULL;
}

User* User::findUserByNickname(const std::string& nick) {
    for (size_t i = 0; i < g_mappa.size(); ++i) {
        if (g_mappa[i]->_nickname == nick)
            return g_mappa[i];
    }
    return NULL;
}
