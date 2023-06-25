#include "Parser.hpp"
#include <string>
#include <iostream>

Parser::Parser( std::string Buffer ) : _input(Buffer) {
	ParseMsg();
	return ;
}

Parser::~Parser( void ) {
	return ;
}

void Parser::ParseMsg( void ) {
	if (this->_input.find("\r\n") != std::string::npos) {
		// parsing lol
	}
	return ;
}