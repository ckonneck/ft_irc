#include "Server.hpp"

void serverloop(std::vector<pollfd> &fds, bool &running, int &server_fd)
{
    for (size_t i = 0; i < fds.size(); ++i) {
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
            // New connection
            if (fds[i].fd == server_fd) {
				
                User::newclient(server_fd, fds);
				std::cout << "found new client" << std::endl;
                continue;
            }
            // Data from existing client
            char buffer[1024];
            ssize_t n = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0) {
                std::cout << "Client disconnected: FD " << fds[i].fd << std::endl;
                close(fds[i].fd);
                fds.erase(fds.begin() + i);
                --i;
                continue;
            }
            buffer[n] = '\0';
            std::string msg(buffer);
            std::cout << "Received from " << fds[i].fd << ": " << msg;
            commandParsing(buffer, fds, i);
        }
    }
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
    std::vector<std::string> mVec = split(mBuf, ' ');
    if (mBuf.find("/KICK") == 0 && mVec.size() > 1)
    {
        std::cout << "found /KICK on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
    }
    if (mBuf.find("/JOIN") == 0 && mVec.size() > 1)
    {
        std::cout << "found /JOIN on position 0" << std::endl;

        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        join_channel(fds[i].fd, NULL, NULL);
    }
    if (mBuf.find("/INVITE") == 0 && mVec.size() > 1)
    {
        std::cout << "found /INVITE on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
    }
    if (mBuf.find("/TOPIC") == 0 && mVec.size() > 1)
    {
        std::cout << "found /TOPIC on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        //to pass full topic, should use the full vector minus the first word
        //which will be /TOPIC
    }
    if (mBuf.find("/NICK") == 0 && mVec.size() > 1)
    {
        std::cout << "found /NICK on position 0" << std::endl;
        std::cout << "found "<< mVec[1] <<" on position 1" << std::endl;
        //let jan handle parsing
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