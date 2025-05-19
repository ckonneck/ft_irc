
#include <sstream>
#include <unistd.h>  // for send()
#include <cstring>   // for strerror()
#include <iostream>

static const std::string SERVER_NAME = "irc.42.fr";

void sendReply(Client& client, const std::string& code, const std::string& msg) {
	std::ostringstream oss;
	oss << ":" << SERVER_NAME << " " << code << " " << client.getNick()
	    << " " << msg << "\r\n";
	std::string output = oss.str();

	if (send(client.getSocket(), output.c_str(), output.length(), 0) == -1)
		std::cerr << "sendReply error: " << strerror(errno) << std::endl;
}

struct ParsedCommand {
	std::string command;
	std::vector <std::string> params;
	std::string message;
};

void sendError(Client& client, const std::string& code, const std::string& msg) {
	std::ostringstream oss;
	oss << ":" << serverName << " " << code << " " << client.getNick() << " " << msg << "\r\n";
	send(client.getSocket(), oss.str().c_str(), oss.str().length(), 0);
}

//void commandParsing(char *messagebuffer)

ParsedCommand parseIrcCommand(const std::string& rawLine) {
	ParsedCommand result;
	std::string line = rawLine;

	// Remove \r\n if present
	if (!line.empty() && line[line.size() - 1] == '\n')
		line = line.substr(0, line.size() - 1);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line = line.substr(0, line.size() - 1);

	// Split on first ":" for trailing
	size_t trailingPos = line.find(" :");
	if (trailingPos != std::string::npos) {
		result.trailing = line.substr(trailingPos + 2);
		line = line.substr(0, trailingPos);
	}

	std::istringstream iss(line);
	std::string word;
	bool first = true;

	while (iss >> word) {
		if (first) {
			result.command = word;
			first = false;
		} else {
			result.params.push_back(word);
		}
	}
	return result;
}


//test 
// ParsedCommand cmd = parseIrcCommand("PRIVMSG #general :hello world");

// std::cout << "Command: " << cmd.command << std::endl;
// for (size_t i = 0; i < cmd.params.size(); ++i)
// 	std::cout << "Param " << i << ": " << cmd.params[i] << std::endl;
// std::cout << "Trailing: " << cmd.trailing << std::endl;


void handleCommand(const ParsedCommand& cmd, Client& client) {
	std::string c = cmd.command;
	std::transform(c.begin(), c.end(), c.begin(), ::toupper); // Case-insensitive command

	if (c == "PASS") {
		if (cmd.params.empty()) {
			sendError(client, "461", "PASS :Not enough parameters");
			return;
		}
		std::cout << "[Handle] PASS\n";
		// TODO: Check password and mark client as authenticated
	}
	else if (c == "NICK") {
		if (cmd.params.empty()) {
			sendError(client, "431", ":No nickname given");
			return;
		}
		std::cout << "[Handle] NICK\n";
		// TODO: Set nickname, check for conflicts
	}
	else if (c == "USER") {
		if (cmd.params.size() < 4) {
			sendError(client, "461", "USER :Not enough parameters");
			return;
		}
		std::cout << "[Handle] USER\n";
		// TODO: Set username and real name
	}
	else if (c == "JOIN") {
		if (cmd.params.empty()) {
			sendError(client, "461", "JOIN :Not enough parameters");
			return;
		}
		std::cout << "[Handle] JOIN\n";
		// TODO: Add client to channel
	}
	else if (c == "PART") {
		if (cmd.params.empty()) {
			sendError(client, "461", "PART :Not enough parameters");
			return;
		}
		std::cout << "[Handle] PART\n";
		// TODO: Remove client from channel
	}
	else if (c == "PRIVMSG") {
		if (cmd.params.empty()) {
			sendError(client, "411", ":No recipient given (PRIVMSG)");
			return;
		}
		if (cmd.trailing.empty()) {
			sendError(client, "412", ":No text to send");
			return;
		}
		std::cout << "[Handle] PRIVMSG\n";
		// TODO: Send message to user or channel
	}
	else if (c == "QUIT") {
		std::cout << "[Handle] QUIT\n";
		// TODO: Close connection and notify others
	}
	else if (c == "PING") {
		if (cmd.trailing.empty()) {
			sendError(client, "409", ":No origin specified (PING)");
			return;
		}
		std::cout << "[Handle] PING\n";
		// TODO: Respond with PONG
	}
	else if (c == "PONG") {
		std::cout << "[Handle] PONG (ignored)\n";
		// Usually ignored unless tracking ping times
	}
	else if (c == "TOPIC") {
		if (cmd.params.empty()) {
			sendError(client, "461", "TOPIC :Not enough parameters");
			return;
		}
		std::cout << "[Handle] TOPIC\n";
		// TODO: View or set topic
	}
	else if (c == "KICK") {
		if (cmd.params.size() < 2) {
			sendError(client, "461", "KICK :Not enough parameters");
			return;
		}
		std::cout << "[Handle] KICK\n";
		// TODO: Kick user from channel
	}
	else if (c == "INVITE") {
		if (cmd.params.size() < 2) {
			sendError(client, "461", "INVITE :Not enough parameters");
			return;
		}
		std::cout << "[Handle] INVITE\n";
		// TODO: Invite user to channel
	}
	else if (c == "MODE") {
		if (cmd.params.empty()) {
			sendError(client, "461", "MODE :Not enough parameters");
			return;
		}
		std::cout << "[Handle] MODE\n";
		// TODO: Set channel/user modes
	}
	else {
		std::cout << "[Warning] Unknown command: " << cmd.command << std::endl;
		sendError(client, "421", cmd.command + " :Unknown command"); // ERR_UNKNOWNCOMMAND
	}
}