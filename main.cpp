#include "server.hpp"

int main (void)
{
	
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(6667); //port of our choosing

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
	std::vector<pollfd> fds;
	pollfd server_pollfd = {server_fd, POLLIN, 0};
	fds.push_back(server_pollfd);

	char messagebuffer[2024];
	    while (true)
		{
        int activity = poll(&fds[0], fds.size(), -1);
        if (activity < 0) continue;

        for (size_t i = 0; i < fds.size(); ++i)
		{
            if (fds[i].revents & POLLIN)
			{
                if (fds[i].fd == server_fd)
				{
                    int client_fd = accept(server_fd, NULL, NULL);
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    pollfd client_pollfd = { client_fd, POLLIN, 0 };
                    fds.push_back(client_pollfd);
                }
				else
				{

                    int n = recv(fds[i].fd, messagebuffer, sizeof(messagebuffer) - 1, 0);
                    if (n <= 0) {
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                    }
					else
					{
                        messagebuffer[n] = '\0';
                        std::cout << "Client: " << messagebuffer;
                        // You can add command handling here
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}