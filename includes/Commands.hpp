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
	static void nick(Parser& input, Client& client, std::vector<Client>& connections);
	static void topic(Client& client);

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
