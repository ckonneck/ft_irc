#include "Server.hpp"
int main (int argc, char **argv)
{	
    if (argc == 3)
    {
        validatePort(argv[1]);
    }
    else
    {
        std::cout << "invalid number of arguments." << std::endl;
        std::cout << "Usage: /ircserv <port> <password>" << std::endl;//password is currently doing nothing yet.
        exit(1);
    }
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    
    addr.sin_port = htons(std::atoi(argv[1])); //port of our choosing


    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
	std::vector<pollfd> fds;
    pollfd server_pollfd = { server_fd, POLLIN, 0 };
    pollfd stdin_pollfd = { STDIN_FILENO, POLLIN, 0 };

    fds.push_back(stdin_pollfd);  // for server console input
    fds.push_back(server_pollfd); // for incoming client connections

    bool running = true;
    
    welcomemessage();
    while (running)
    {
        int activity = poll(fds.data(), fds.size(), -1);
        if (activity < 0)
        {
            continue;
        }
        serverloop(fds, running, server_fd);

    }
    cleanup(fds);

    

    return 0;
}

void cleanup(std::vector<pollfd> &fds)
{
    for (size_t i = 0; i < fds.size(); ++i)
    {
        close(fds[i].fd);
    }
}

