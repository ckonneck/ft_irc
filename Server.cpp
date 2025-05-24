#include "Server.hpp"

std::string servername = "server-chan";

void cleanupUser(User* u) {
    std::map<std::string,Chatroom*>::iterator it;
    for (it = g_chatrooms.begin(); it != g_chatrooms.end(); ++it) {
        Chatroom* chan = it->second;
        chan->removeUser(u);
        chan->removeOperator(u);
    }
    removeUser(u);
}

void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd)
{
    for (size_t i = 0; i < fds.size(); i++)
    {
        if (fds[i].revents & POLLIN) {
			 // Handle input from server terminal
            if (fds[i].fd == STDIN_FILENO)
            {
                if(serverexit() == true)
                {
                    running = false;
                    break;
                }
            }
            if (fds[i].fd == server_fd) {
                
                User::newclient(server_fd, fds);
                std::cout << "found new client" << std::endl;
                i++;
                continue;
            }
            char buffer[1024];
            ssize_t n = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
if (n <= 0) {
    int disc_fd = fds[i].fd;
    std::cout << "Client disconnected: FD " << disc_fd << std::endl;
    close(disc_fd);

    // remove this fd from the poll list
    fds.erase(fds.begin() + i);
    User* old = findUserByFD(disc_fd);
    if (old) {
        cleanupUser(old);
        std::cout << "Cleaned up User* for FD " << disc_fd << std::endl;
    } else {
        std::cout << "No matching User* for FD " << disc_fd << std::endl;
    }
    --i;
    continue;
}


            buffer[n] = '\0';
            std::string msg(buffer);
            std::cout << "Received from " << fds[i].fd << ": " << msg;
            User *user = findUserByFD(fds[i].fd);
            if (user->isRegis()== false)
            {
                std::cout << "WE NEW USER UP IN HERE" << std::endl;
                std::string nick = parseNick(msg);
                std::string host = parseHost(msg);
                std::string user_str = parseUser(msg);
                user->setNickname(nick);
                user->setHostname(host);
                user->setUser(user_str);
                if(user->getNickname() != "" && user->getHostname() != "" && user->getUsername() != "")//this sometimes has hiccups
                {
                    user->setRegis(true);
                    std::cout << "USER " << nick << " HAS BEEN ABSOLUTELY VERIFIED FOR SURE" << std::endl;
                    user->HSwelcome(fds[i].fd);
                }
                continue;
            }
            commandParsing(buffer, fds, i);
        }
    }
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
    for (std::vector<User*>::iterator it = g_mappa.begin();
         it != g_mappa.end(); ++it)
    {
        if (*it == target) {
            delete *it;
            g_mappa.erase(it);
            return;
        }
    }
}


void welcomemessage()
{
    std::cout << "server-chan has been started UwU" << std::endl;
    std::cout << "type exit to exit UwU" << std::endl;
    std::cout << "have fun and don't be a mean cookie UwU" << std::endl;
}
bool serverexit()
{
    std::string input;
    std::getline(std::cin, input);
    if (input == "exit")
    {
        std::cout << "Server shutting down.\n";
        return true;
    }
    else
        return false;
}

void send_to_client(int client_fd, const std::string& message)
{
    std::string msg = message + "\r\n";
    send(client_fd, msg.c_str(), msg.length(), 0);
}

void join_channel(int client_fd, const std::string& nickname, const std::string& channel) {
    // Simulate JOIN message
    send_to_client(client_fd, ":" + nickname + "!" + nickname + "@localhost JOIN :" + channel);

    // RPL_NAMREPLY (353): list of users in channel
    send_to_client(client_fd, ":localhost 353 " + nickname + " = " + channel + " :" + nickname);

    // RPL_ENDOFNAMES (366): end of names list
    send_to_client(client_fd, ":localhost 366 " + nickname + " " + channel + " :End of /NAMES list.");
}


void messagehandling(std::vector<pollfd> &fds, size_t i)//obsolete
{
    char messagebuffer[2024];
    int n = recv(fds[i].fd, messagebuffer, sizeof(messagebuffer) - 1, 0);
    if (n <= 0)
    {
        std::cout << "Client disconnected: FD " << fds[i].fd << "\n";
        close(fds[i].fd);
        fds.erase(fds.begin() + i);
        --i;
    }
    else
    {
        commandParsing(messagebuffer, fds, i);

        //insert command parsing
        messagebuffer[n] = '\0';
        std::cout << "Client " << fds[i].fd << ": " << messagebuffer;
    }
}




std::vector<std::string> split(const std::string &input, char delimiter) {
    std::vector<std::string> result;
    std::istringstream ss(input);
    std::string item;

    while (std::getline(ss, item, delimiter))
    {
        result.push_back(item);
    }
    return result;
}