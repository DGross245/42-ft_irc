#ifndef COMMANDS_H
# define COMMANDS_H

# include "Channel.hpp"
# include "Client.hpp"
# include <string>
# include <vector>

class Commands {

public:

	//Member functions
	static void	join(Parser &input, Client client, std::vector<Channel> &channels);
	static void	pass(Parser &input, Client &client, std::string password);
	static void privmsg(Parser &input, Client client, std::vector<Client> connections, std::vector<Channel> channels);
	static void quit(Parser &input, Client client, std::vector<Channel> &channels, std::vector<Client>& connections);
	static std::vector<Channel>::iterator searchForChannel(std::string channelName, std::vector<Channel> &channels);
	static void forwardMsg(std::string message, Client target, std::vector<Client> clients, bool shouldInclude);
	static void kick(Parser &input, Client requestor, std::vector<Channel> &channels);
	static void mode(Parser &input, Client client, std::vector<Channel> &channels);
	static void part(Parser &input, Client client, std::vector<Channel> &channels);
	static void topic(Parser &input, Client client, std::vector<Channel> &channels);
	static void ping(Parser &input, Client client);
	static void cap(Parser &input, Client client);
	static void	nick(Parser& input, Client& client, std::vector<Client>& connections);
	static void user(Parser &input, Client &client);
	static void invite(Client& client, Parser& input, std::vector<Client> &connections, std::vector<Channel> &channels);
	static void sendWelcomeMessage(Client client, std::vector<Channel>::iterator channelIt);
	static void splitByComma( Parser &input );

	//Mode functions
	static void executeInvite(bool sign, Channel &channel, std::string param, Client client);
	static void executeKey(bool sign, Channel &channel, std::string key, Client client);
	static void executeOperator(bool sign, Channel &channel, std::string targetName, Client client);
	static void executeLimit(bool sign, Channel &channel, std::string channelLimit, Client client);
	static void executeTopic(bool sign, Channel &channel, std::string param, Client client);

private:
	// Constructor
	Commands(void);

	// Destructor
	~Commands(void);

};

#endif
