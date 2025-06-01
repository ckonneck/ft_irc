#include "Server.hpp"

void validatePort(char *argv)
{
    if(isDigit(argv) == true)
    {
        int len = strlen(argv);
        if(len >= 1 && len <= 5)
        {
            std::cout << "you chose port " << argv << std::endl;
        }
        else
        {
            std::cout << "invalid port, choose one between 1 and 65535." << std::endl;
            std::cout << "btw, 6667 works best." << std::endl;
            exit(1);
        }
    }
    else
    {
        std::cout << "invalid port, choose one between 1 and 65535." << std::endl;
        std::cout << "btw, 6667 works best." << std::endl;
        exit(1);
    }
}

bool isDigit(char *strnum)
{
	int i = 0;
	while (strnum[i])
	{
		if (strnum[i] > '9' || strnum[i] < '0' )
			return false;
		i++;
	}
	return true;
}
std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Utility function to trim trailing \r and \n
std::string sanitize(const std::string& str) {
    size_t end = str.find_last_not_of("\r\n");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
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
