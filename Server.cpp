#include "Server.hpp"
void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd)
{
    for (size_t i = 0; i < fds.size(); i++) {
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
                continue;
            }
            char buffer[1024];
            ssize_t n = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0) {
                std::cout << "Client disconnected: FD " << fds[i].fd << std::endl;
                close(fds[i].fd);
                fds.erase(fds.begin() + i);
                User *old = findUserByFD(server_fd);
                std::cout << "deleting old" << std::endl;
                removeUser(old);
                --i;
                continue;
            }
            buffer[n] = '\0';
            std::string msg(buffer);
            std::cout << "Received from " << fds[i].fd << ": " << msg;
            // New connection
            // Data from existing client
            
            commandParsing(buffer, fds, i);
        }
    }
}

void removeUser(User* target) {
    // Step 1: move all non-target pointers to the front, get new logical end
    std::vector<User*>::iterator newEnd =
        std::remove(g_mappa.begin(), g_mappa.end(), target);

    // Step 2: delete the removed User* objects
    for (std::vector<User*>::iterator it = newEnd; it != g_mappa.end(); ++it) {
        delete *it;
    }

    // Step 3: actually erase them from the vector
    g_mappa.erase(newEnd, g_mappa.end());
}

void welcomemessage()
{
    std::cout << "server-chan has been started UwU" << std::endl;
    std::cout << "type exit to exit UwU" << std::endl;
    std::cout << "if you wanna connect to this, the easiest way is to " << std::endl;
    std::cout << "open another cmdwindow and type: telnet localhost 6667" << std::endl;
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


void messagehandling(std::vector<pollfd> &fds, size_t i)
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

void commandParsing(char *messagebuffer, std::vector<pollfd> &fds, size_t i)
{
    std::string mBuf(messagebuffer);
    std::cout << "the command is " << mBuf << std::endl;
    User *curr = findUserByFD(fds[i].fd);
    std::cout << "this is tha nicknamuin commando: "<< curr->getNickname() << std::endl;
    std::vector<std::string> mVec = split(mBuf, ' ');
    if (mBuf.find("PING") == 0)
    {
        std::string response = "PONG :localhost\r\n";
        send(fds[i].fd, response.c_str(), response.length(), 0);
    }
    if (mBuf.find("NICK") == 0 && mVec.size() > 1)
    {
        std::cout << "found /NICK on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        std::string newnick = parseNick(mBuf);
        curr->setNickname(newnick);
        curr->HSNick(newnick);
        std::cout << "this is tha nicknamuin commando2222: "<< curr->getNickname() << std::endl;

    }
    if (mBuf.find("KICK") == 0 && mVec.size() > 1)
    {
        std::cout << "found /KICK on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
    }
    if (mBuf.find("JOIN") == 0 && mVec.size() > 1)
    {
        std::cout << "found /JOIN on position 0" << std::endl;
        
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        std::string chanName = mVec[1];
        if (chanName[0] != '#')
        {
            std::cout <<"  Invalid channel name " << std::endl;
            return;
        }
        Chatroom* chan = NULL;
        if(g_chatrooms.find(chanName) == g_chatrooms.end())
        {
            chan = new Chatroom(chanName);
            g_chatrooms[chanName] = chan;
        }
        else
        {
            chan = g_chatrooms[chanName];
        }

        chan->addUser(curr);
        std::string msg = ":" + curr->getNickname() + " JOIN :" + chanName + "\r\n";
        chan->broadcast(msg, NULL); // NULL = broadcast to all
        join_channel(fds[i].fd, curr->getNickname(), chan->getName());
    }
    if (mBuf.find("PRIVMSG") == 0)
    {
        if (mVec.size() < 3) return;
        std::string target = mVec[1];
        size_t msg_start = mBuf.find(" :", 0);
        std::string actual_message = (msg_start != std::string::npos)
        ? mBuf.substr(msg_start + 2) : "";
        if (target[0] == '#')
        {
            // Channel message
            if (g_chatrooms.find(target) == g_chatrooms.end()) {
                send_to_client(fds[i].fd, "403 " + target + " :No such channel\r\n");
                return;
            }
        Chatroom* chan = g_chatrooms[target];
        std::string fullMsg = ":" + curr->getNickname() + " PRIVMSG " + target + " :" + actual_message + "\r\n";
        chan->broadcast(fullMsg, curr);
    } else {
        // Private message to a user
    }
    }
    if (mBuf.find("INVITE") == 0 && mVec.size() > 1)
    {
        std::cout << "found /INVITE on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
    }
    if (mBuf.find("TOPIC") == 0 && mVec.size() > 1)
    {
        std::cout << "found /TOPIC on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        //to pass full topic, should use the full vector minus the first word
        //which will be /TOPIC
    }
    // if (mBuf.find("NICK") == 0 && mVec.size() > 1)
    // {
    //     std::cout << "found /NICK on position 0" << std::endl;
    //     std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
    //     //let jan handle parsing
    // }
    
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