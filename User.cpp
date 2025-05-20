#include "Server.hpp"

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
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd >= 0)
    {   //need a map of clients thats stored somewhere. and correctly free the memory when the user leaves.
        User *bob = new User("needparsing", "needpassword" );//return the actual one from parsing
        bob->_FD = client_fd;
        g_mappa.push_back(bob);
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        pollfd client_pollfd = { client_fd, POLLIN, 0 };
        fds.push_back(client_pollfd);
        std::cout << "New client connected: FD nr " << client_fd << "\n";
        bob->HSwelcome(client_fd);
    }
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

