#include "Server.hpp"


User::User(std::string &nickname, std::string &password) : OP(nickname, password)
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
    {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        pollfd client_pollfd = { client_fd, POLLIN, 0 };
        fds.push_back(client_pollfd);
        std::cout << "New client connected: FD nr " << client_fd << "\n";
        std::string nick = "needparsing"; // from client //get ALL THE NECESSARY DETAILS FROM THIS PARSING

        std::string msg1 = ":localhost 001 " + nick + " :Welcome to UWURC server " + nick + "\r\n";
        send(client_fd, msg1.c_str(), msg1.length(), 0);

        std::string msg2 = ":localhost 002 " + nick + " :Your host is UWUCHAN running version 1.0\r\n";
        send(client_fd, msg2.c_str(), msg2.length(), 0);

        std::string msg3 = ":localhost 003 " + nick + " :This server was created IMA DA NYA\r\n";
        send(client_fd, msg3.c_str(), msg3.length(), 0);

        std::string msg4 = ":localhost 004 " + nick + " owo please don't be mean\r\n";
        send(client_fd, msg4.c_str(), msg4.length(), 0);
    }
}