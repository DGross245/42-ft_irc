#ifndef COMMANDS_H
# define COMMANDS_H

# include <exception>
# include <string>
# include <vector>

class Commands {

public:

	// Constructor
	Commands(void);

	// Destructor
	~Commands(void);

	//Member functions
	static void	join(Parser &input, Client client, std::vector<Channel> channels);
	static void	pass(Parser &input, Client client, std::string password);
	static void privmsg(Parser &input, Client client, std::vector<Client> connections, std::vector<Channel> channels);
	static void quit(Parser &input, Client client, std::vector<Channel> channels);
	static int searchForChannel(std::string channelName, std::vector<Channel> channels);
	static int searchForUser(std::string nickname, std::vector<Client> clients);
	static void joinChannel(std::string channelName, Client user, std::vector<Channel> &channels);
	static void forwardMsg(std::string message, std::vector<Client> connections);
	static void kick(Parser &input, Client client, std::vector<Channel> channels);

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
