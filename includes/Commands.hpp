#ifndef COMMANDS_H
# define COMMANDS_H

# include <exception>
# include <string>
# include <vector>
# include "Channel.hpp"
# include "Client.hpp"

// @todo Commands zu einer richtige static classe machen also auch constructor und destructor in privat
class Commands {

public:

	// Constructor
	Commands(void);

	// Destructor
	~Commands(void);

	//Member functions
	static void	join(Parser &input, Client client, std::vector<Channel> &channels);
	static void	pass(Parser &input, Client client, std::string password);
	static void privmsg(Parser &input, Client client, std::vector<Client> connections, std::vector<Channel> channels);
	static void quit(Parser &input, Client client, std::vector<Channel> &channels);
	static std::vector<Channel>::iterator searchForChannel(std::string channelName, std::vector<Channel> &channels);
	static void forwardMsg(std::string message, std::string channelName, Client client, std::vector<Client> connections);
	static void kick(Parser &input, Client requestor, std::vector<Channel> &channels);
	static void mode(Parser &input, Client client, std::vector<Channel> &channels);
	static void part(Parser &input, Client client, std::vector<Channel> &channels);
	static void topic(Parser &input, Client client, std::vector<Channel> &channels);
	static void ping(Parser &input, Client client);
	static void cap(Parser &input, Client client);
	static void	nick(Parser& input, Client& client, std::vector<Client>& connections);
	static void user(Parser &input, Client &client, std::vector<Client>& connections);
	static void invite(Client& client, Parser& input, std::vector<Client> &connections, std::vector<Channel> &channels);

	//Mode functions
	static void executeInvite(bool sign, Channel &channel, std::string param, Client client);
	static void executeKey(bool sign, Channel &channel, std::string param, Client client);
	static void executeOperator(bool sign, Channel &channel, std::string param, Client client);
	static void executeLimit(bool sign, Channel &channel, std::string param, Client client);
	static void executeTopic(bool sign, Channel &channel, std::string param, Client client);
	// static void mode(Parser &inout, Client client, std::vector<Channel> &channels);

	class commandFailException : std::exception {

	public:
		commandFailException( std::string error );
		~commandFailException( void ) throw();
		virtual const char *what() const throw();

	private:
		std::string _error;

	};

private:

};

#endif
