#include "Parser.hpp"
#include <string>
#include <iostream>

// JOIN #Channel/r/n
// gibt vllt noch ein problem mit blocking

Parser::Parser( std::string Buffer ) : _input(Buffer) {
	ParseMsg();
	return ;
}

Parser::~Parser( void ) {
	return ;
}

void Parser::ParseMsg( void ) {
	if (this->_input.find("\r\n") != std::string::npos) {
		this->_command = this->_input.substr(0, this->_input.find_first_of(' '));
		this->_parameter = this->_input.substr(this->_input.find_first_of(' ') + 1, this->_input.find_first_of('\n'));
	}
	return ;
}