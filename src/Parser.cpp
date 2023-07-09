#include "Parser.hpp"
#include <string>
#include <iostream>
#include "Constants.hpp"
#include "Client.hpp"
#include <vector>
#include <sys/socket.h>

Parser::Parser( std::string &buffer, Client client ) {
	size_t pos = buffer.find("\r\n");
	if (pos != std::string::npos ) {
		this->_input = buffer.substr(0, pos);
		parseMsg( client );
		buffer.erase(0, pos + 2);
	}
	else
		std::cerr << "Parsing Error" << std::endl;
	return ;
}


Parser::~Parser( void ) {
	return ;
}

std::string &Parser::getPrefix( void ) {
	return (this->_prefix);
}

std::string &Parser::getTrailing( void ) {
	return (this->_trailing);
}

std::string &Parser::getCMD( void ) {
	return (this->_command);
}

std::vector<std::string> &Parser::getParam( void ) {
	return (this->_parameter);
}


void Parser::isValidCommandLine( Client client ) {
	if (this->getCMD() == "PASS")
		checkPASS( client );
	else if (this->getCMD() == "NICK")
		checkNICK( client );
	else if (this->getCMD() == "USER")
		checkUSER( client );
	else if (this->getCMD() == "QUIT")
		checkQUIT( client );
	else if (this->getCMD() == "JOIN")
		checkJOIN( client );
	else if (this->getCMD() == "MODE")
		checkMODE( client );
	else if (this->getCMD() == "TOPIC")
		checkTOPIC( client );
	else if (this->getCMD() == "INVITE")
		checkINVITE( client );
	else if (this->getCMD() == "KICK")
		checkKICK( client );
	else if (this->getCMD() == "PRIVMSG")
		checkPRIVMSG( client );
	else if (this->getCMD() == "PING")
		checkPING( client );
	else if (this->getCMD() == "PART")
		checkPART( client );
	else if (this->getCMD() == "CAP")
		checkCAP( client );
	return ;
}

void Parser::checkCAP( Client client) {
	if (this->getParam().size() < 1 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::sendError( Client client ) {
	std::string message = SERVER  " " ERR_NEEDMOREPARAMS " " + client.getNickname() + " " + this->getCMD() + " :Not enough parameters";
	send(client.getSocketfd(), message.c_str(), message.length(), 0);
	throw parserErrorException("Invalid Command " + this->getCMD());
	return ;
}

void Parser::checkPASS( Client client ) {
	if (this->getParam().size() != 1 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkNICK( Client client ) {
	if (this->getParam().size() != 1 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkUSER( Client client ) {
	if (this->getParam().size() != 3 || this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkQUIT( Client client ) {
	if (!this->getParam().empty())
		sendError( client );
	return ;
}

void Parser::checkJOIN( Client client ) {
	if (this->getParam().size() < 1 || this->getParam().size() > 2 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkMODE( Client client ) {
	if (this->getParam().size() < 1 || this->getParam().size() > 4 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkTOPIC( Client client ) {
	if (this->getParam().size() != 1)
		sendError( client );
	return ;
}

void Parser::checkINVITE( Client client ) {
	if (this->getParam().size() != 2 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkKICK( Client client ) {
	if (this->getParam().size() != 2)
		sendError( client );
	return ;
}

void Parser::checkPRIVMSG( Client client ) {
	if (this->getParam().size() != 1 || this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkPING( Client client ) {
	if (this->getParam().size() != 1 || !this->getTrailing().empty())
		sendError( client );
	return ;
}

void Parser::checkPART( Client client ) {
	if (this->getParam().size() != 1)
		sendError( client );
	return ;
}

// @todo Have to rework parser, mulitple cmd in one line are ignored
// @todo Have to add more error messages to better replay on erros using the ERR_... codes
void Parser::parseMsg( Client client ) {
	//while loop here
	if (this->_input[0] == ':')
		prefixHandler(this->_input.substr(1, this->_input.find_first_of(' ')));
	commandHandler(this->_input.substr(0, this->_input.find_first_of(' ')));
	if (this->_input[0] != ':')
		paramHandler(this->_input.substr(0, this->_input.find_first_of("\r\n") ));
	else
		trailingHandler(this->_input.substr(1, this->_input.find_first_of("\r\n")));
	isValidCommandLine( client );
	//hier dann auch executen 
	return ;
}

void Parser::prefixHandler( std::string prefix ) {
	this->_prefix = prefix;
	this->_input.erase(0, this->_input.find_first_of(' ') + 1);
}

void Parser::commandHandler( std::string command ) {
	this->_command = command;
	this->_input.erase(0, this->_input.find_first_of(' ') + 1);
}

void Parser::paramHandler( std::string param ) {
	size_t found = 0; 

	while (!param.empty()) {
		if (param == "\r\n")
			return ;
		found = param.find_first_of(": ");
		if (found != std::string::npos && param[found] == ':') {
			trailingHandler(param.substr(1, param.find("\r\n")));
			return ;
		}
		else if (found == std::string::npos) {
			this->_parameter.push_back(param.substr(0, param.find("\r\n")));
			return ;
		}
		else
			this->_parameter.push_back(param.substr(0, found));
		param.erase(0, found + 1);
	}
}

void Parser::trailingHandler( std::string trailing ) {
	this->_trailing = trailing;
	return ;
}

Parser::parserErrorException::~parserErrorException( void ) throw() { return ; }
Parser::parserErrorException::parserErrorException( std::string error ) : _error(error) { return ; }
const char *Parser::parserErrorException::what() const throw() { return (this->_error.c_str()); }