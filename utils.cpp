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

#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>

void printStringHex(const std::string& input)
{
    for (std::string::size_type i = 0; i < input.size(); ++i) {
        char c = input[i];
        std::cout << "[";
        switch (c) {
            case '\n': std::cout << "\\n"; break;
            case '\r': std::cout << "\\r"; break;
            case '\t': std::cout << "\\t"; break;
            case '\v': std::cout << "\\v"; break;
            case '\f': std::cout << "\\f"; break;
            case '\a': std::cout << "\\a"; break;
            case '\b': std::cout << "\\b"; break;
            case '\\': std::cout << "\\\\"; break;
            default:
                if (c >= 32 && c <= 126) {
                    std::cout << c;
                } else {
                    // Non-printable, show as hex escape
                    std::cout << "\\x";
                    static const char* hex = "0123456789ABCDEF";
                    unsigned char uc = static_cast<unsigned char>(c);
                    std::cout << hex[(uc >> 4) & 0xF] << hex[uc & 0xF];
                }
        }
        std::cout << "]" << std::endl;
    }
}

std::string extractAfterHashBlock(const std::vector<std::string>& words) {
    std::string result;
    bool foundHash = false;
    bool skippingHash = false;

    for (std::vector<std::string>::size_type i = 0; i < words.size(); ++i) {
        const std::string& word = words[i];

        if (!foundHash) {
            if (!word.empty() && word[0] == '#') {
                foundHash = true;
                skippingHash = true;
            }
        } else if (skippingHash) {
            if (!word.empty() && word[0] == '#') {
                continue; // still skipping
            } else {
                skippingHash = false;
                result += word;
            }
        } else {
            result += " " + word;
        }
    }

    return result;
}

#include <cstdio>
static std::set<std::string> generatedNicks;

std::string uwuTasticNick()
{
    static const char* syllables[] = {
        "senpai","baka","chan","buta", "senpai", "uwu", "nya", "me", "ne", "mo", "ki", "bu", "po", "lo", "chu", "rin", "ko", "mi", "lu", "to"
    };
    const int count = sizeof(syllables) / sizeof(syllables[0]);

    static bool seeded = false;
    if (!seeded) {
        std::srand(std::time(NULL));
        seeded = true;
    }

    for (int attempts = 0; attempts < 1000; ++attempts) {
        const char* first = syllables[std::rand() % count];
        const char* second = syllables[std::rand() % count];

        int number = std::rand() % 100;  // 0–99
        char numBuf[4]; 
        std::snprintf(numBuf, sizeof(numBuf), "%d", number);

        std::string nick = std::string(first) + second + numBuf;

        if (nick.length() <= 9 && generatedNicks.find(nick) == generatedNicks.end()) {
            generatedNicks.insert(nick);
            return nick;
        }
    }

    return "uwu0";
}