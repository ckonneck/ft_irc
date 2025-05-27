#include "Server.hpp"

std::string servername = "server-chan";
std::string g_serverPassword = "";

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
        // std::cout << i << "sanity" << std::endl;
        User *user = findUserByFD(fds[i].fd);
        if (fds[i].revents & POLLIN) 
        {
			 // Handle input from server terminal
            if (fds[i].fd == STDIN_FILENO)
            {
                if(serverexit() == true)
                {
                    running = false;
                    break;
                }
                else
                    continue;
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
                }else {
                    std::cout << "No matching User* for FD " << disc_fd << std::endl;
                    }
                --i;
                continue;
            }
            buffer[n] = '\0';
            std::string msg(buffer);
            std::cout << "Received from " << fds[i].fd << ": " << msg;
            
            if (!user)
                continue;
            user->appendToBuffer(std::string(buffer));
            while (user->hasCompleteLine())
            {
                std::string msg = user->extractLine();
                //std::cout << "Parsed line from " << fds[i].fd << ": " << msg << std::endl;
                if (user->isRegis()== false)
                {
                    registrationParsing(user, msg);
                }
                continue;
            }
            commandParsing(buffer, fds, i);
                        // ⭐ Optional: If user has new data to send, set POLLOUT
                    }
        if (user && user->hasDataToSend())
        {
            fds[i].events |= POLLOUT;
            // user->set_rdyToWrite(false);
        }
        if (fds[i].revents & POLLOUT)
            {
                std::cout << "[DEBUG] POLLOUT ready for fd=" << fds[i].fd << "\n";
                if (!user || !user->hasDataToSend()) continue;
                std::cout << "[DEBUG] POLLOUT after the !user->hasdatatosend() check" << fds[i].fd << "\n";
                const std::string& msg = user->getSendBuffer();
                ssize_t sent = send(fds[i].fd, msg.c_str(), msg.size(), 0);
                std::cout << "[DEBUG] send(fd="<<fds[i].fd<<", bytes="<<msg.size()
                    <<") returned "<<sent<<"\n";
                if (sent > 0)
                {
                    user->consumeSendBuffer(sent);
                    if (!user->hasDataToSend())
                    {
                        std::cout << "no more data to send " << user->getFD() << std::endl;
                        // No more to send — stop polling for write
                        fds[i].events &= ~POLLOUT;
                    }
                }
                else if (sent == -1 && errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    std::cerr << "Error sending to fd " << fds[i].fd << ": " << strerror(errno) << std::endl;
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    if (user) cleanupUser(user);
                    --i;
                    continue;
                }
            }


    }
    
}

void registrationParsing(User *user, std::string msg)
{
    // if (msg.rfind("PASS ", 0) == 0) {
    //     if (user->isRegis()) {
    //         user->appendToSendBuffer(
    //             ":" + servername + " 462 * :You may not re-register\r\n"
    //         );
    //         return;
    //     }
    //     std::vector<std::string> tok = split(msg, ' ');
    //     if (tok.size() < 2 || tok[1] != PasswordManager::getPassword()) {
    //         user->appendToSendBuffer(
    //             ":" + servername + " 464 * :Password incorrect\r\n"
    //         );
    //         removeUser(user); //raus mit dem falschpasswortler aber macht alles kaputt
    //         return;
    //     }
    //     user->setPassValid(true);
    //     std::cout << "User entered Correct Password UwU" << std::endl;
    //     return;
    // }
    // if (!user->isPassValid()) {
    //     user->appendToSendBuffer(
    //         ":" + servername + " 451 * :You have not registered\r\n"
    //     );
    //     return;
    // }
    std::cout << "WE NEW USER UP IN HERE" << std::endl;
    std::string nick = parseNick(msg);
    std::string host = parseHost(msg);
    std::string user_str = parseUser(msg);
    if (!nick.empty())
        user->setNickname(nick);
    if (!host.empty())
        user->setHostname(host);
    if (!user_str.empty())
        user->setUser(user_str);
    if(user->getNickname() != "" && user->getHostname() != "" && user->getUsername() != "")//fixed hiccups hopefully
    {
        user->setRegis(true);
        std::cout << "USER " << nick << " HAS BEEN ABSOLUTELY VERIFIED FOR SURE" << std::endl;
        user->HSwelcome();
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
    if (uit != g_mappa.end()) {
        delete *uit;
        g_mappa.erase(uit);
    }
}


void welcomemessage()
{
    std::cout << "server-chan has been started UwU" << std::endl;
    std::cout << "type exit to exit UwU" << std::endl;
    std::cout << "have fun and don't be a mean cookie UwU" << std::endl;
    std::cout << std::endl;
    std::cout << "   ⠀⠀⠀⠀⡴⣦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⠀⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠀⠚⠁⠀⠳⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠴⠋⠈⢧⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠀⢠⡆⠀⠀⠀⠀⢠⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⢀⡀⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠀⢸⣧⠀⠀⠀⠀⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡄⠀⠀⠀⠀⣸⠃⠀⠀⠀⠀" << std::endl;
    std::cout << "⠒⠒⠂⠀⢿⡆⢀⣀⣴⠏⠀⠀⢾⡀⠀⣧⠀⠀⡇⠀⠈⣷⣄⠀⢀⣰⠃⠀⠈⠉⠉⠁" << std::endl;
    std::cout << "⡤⠖⠀⠀⠈⠙⠛⠋⠁⠀⠀⠀⠈⠷⠿⠙⠳⠞⠃⠀⠀⠈⠻⠷⠛⠁⠀⠀⡈⠑⠢⡄" << std::endl;
    std::cout << "⠀⢠⠖⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⣆⠀⠀" << std::endl;
    std::cout << "⠘⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠃⠀" << std::endl;
    std::cout << std::endl;
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

// void send_to_client(int client_fd, const std::string& message)
// {
//     std::string msg = message + "\r\n";
//     send(client_fd, msg.c_str(), msg.length(), 0);
// }

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

std::string PasswordManager::_password;

void PasswordManager::setPassword(const std::string& pw) {
    _password = pw;
}

const std::string& PasswordManager::getPassword() {
    return _password;
}
// void messagehandling(std::vector<pollfd> &fds, size_t i)//obsolete
// {
//     char messagebuffer[2024];
//     int n = recv(fds[i].fd, messagebuffer, sizeof(messagebuffer) - 1, 0);
//     if (n <= 0)
//     {
//         std::cout << "Client disconnected: FD " << fds[i].fd << "\n";
//         close(fds[i].fd);
//         fds.erase(fds.begin() + i);
//         --i;
//     }
//     else
//     {
//         commandParsing(messagebuffer, fds, i);

//         //insert command parsing
//         messagebuffer[n] = '\0';
//         std::cout << "Client " << fds[i].fd << ": " << messagebuffer;
//     }
// }



