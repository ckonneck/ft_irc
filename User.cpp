#include "Server.hpp"
#include <cerrno>

std::vector<User*> g_mappa;
User::User(const std::string &nickname,const std::string &password) : _nickname(nickname), _password(password)
{
    this->_isOP = false;
    std::cout << "User "<< this->_nickname <<" has been created" <<std::endl;
}

User::~User()
{
    std::cout << "User "<< this->_nickname <<" fucked off to somewhere else" <<std::endl;
}

void User::newclient(int &server_fd, std::vector<pollfd> &fds)
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd < 0) {
        std::cerr << "[newclient] accept error: " << strerror(errno) << std::endl;
        return;
    }
    // Set non-blocking
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    pollfd pfd;
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    fds.push_back(pfd);

	char buffer[1024];
            ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
			(void)n;
            // if (n <= 0) {
            //     std::cout << "Client disconnected: FD " << client_fd << std::endl;
            //     close(client_fd);
            //     fds.erase(fds.begin());
            // }
			std::cout << "to parse" << std ::endl << buffer << std::endl;
	//User *newUser =  new User("non", "init");

    std::cout << "New client connected: FD " << client_fd << std::endl;
}
void User::HSwelcome(int &client_fd)
{
    std::string nick = "needparsing"; // from client //get ALL THE NECESSARY DETAILS FROM THIS PARSING
            //including stuff like password and whatnot, then actually create the User();
    std::string msg1 = ":localhost 001 " + nick + " :Welcome to UWURC server " + nick + "\r\n";
    send(client_fd, msg1.c_str(), msg1.length(), 0);

    std::string msg2 = ":localhost 002 " + nick + " :Your host is UWUCHAN running version 1.0\r\n";
    send(client_fd, msg2.c_str(), msg2.length(), 0);

    std::string msg3 = ":localhost 003 " + nick + " :This server was created IMA DA NYA\r\n";
    send(client_fd, msg3.c_str(), msg3.length(), 0);

    std::string msg4 = ":localhost 004 " + nick + " owo please don't be mean\r\n";
    send(client_fd, msg4.c_str(), msg4.length(), 0);
}

void User::HSNick(const std::string &newname)//not yet called anywhere
{
    std::string oldnick = this->getNickname();
    std::string newNick = newname;//parsing job, get new nickname
    std::string nickMsg = ":" + oldnick + "!user@localhost NICK :" + newNick + "\r\n";
    send(this->_FD, nickMsg.c_str(), nickMsg.length(), 0);

}
User* findUserByFD(int fd)
{
    std::cout << "Starting search for user with FD: " << fd << std::endl;
    for (size_t i = 0; i < g_mappa.size(); ++i)
    {
        User* user = g_mappa[i];

        // Debug print of nickname and FD being checked
        std::cout << "Checking user: " << user->getNickname()
                  << " (FD: " << user->getFD() << ")" << std::endl;

        if (user->getFD() == fd) {
            std::cout << "Match found: " << user->getNickname() << std::endl;
            return user;
        }
    }
    std::cout << "No user found for FD: " << fd << std::endl;
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

