#include "Client.hpp"

void newclient(int &server_fd, std::vector<pollfd> &fds)
{
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd >= 0)
    {
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        pollfd client_pollfd = { client_fd, POLLIN, 0 };
        fds.push_back(client_pollfd);
        std::cout << "New client connected: FD nr " << client_fd << "\n";
    }
}

void messagehandling(std::vector<pollfd> &fds, size_t &i)
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
        //insert command parsing
        messagebuffer[n] = '\0';
        std::cout << "Client " << fds[i].fd << ": " << messagebuffer;
    }
}

