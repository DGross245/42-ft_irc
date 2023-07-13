#ifndef CONSTANTS_H
# define CONSTANTS_H

# define SERVER	":IRCSERV"

// SUCCESSFUL CONSTANTS MESSAGES
# define RPL_WELCOME		"001"
# define RPL_CHANNELMODEIS	"324"
# define RPL_NOTOPIC		"331"
# define RPL_TOPIC			"332"

// ERROR CONSTANTS MESSAGES
# define ERR_NOSUCHNICK 		"401"
# define ERR_NOSUCHSERVER		"402"
# define ERR_NOSUCHCHANNEL 		"403"
# define ERR_CANNOTSENDTOCHAN   "404"
# define ERR_NORECIPIEN			"411"
# define ERR_NOTEXTTOSEND		"412"
# define ERR_ERRONEUSNICKNAME	"432"
# define ERR_NICKNAMEINUSE		"433"
# define ERR_NOTONCHANNEL 		"442"
# define ERR_USERONCHANNEL 		"443"
# define ERR_NEEDMOREPARAMS 	"461"
# define ERR_ALREADYREGISTRED	"462"
# define ERR_PASSWDMISMATCH 	"464"
# define ERR_CHANNELISFULL		"471"
# define ERR_UNKNOWNMODE 		"472"
# define ERR_INVITEONLYCHAN 	"473"
# define ERR_BADCHANNELKEY		"475"
# define ERR_NOPRIVLIEGES		"481"
# define ERR_CHANOPRIVSNEEDED 	"482"
# define ERR_NOTREGISTERED		"451"

//COLOURS
# define RESET   "\033[0m"
# define RED     "\033[31m"
# define GREEN   "\033[32m"
# define YELLOW  "\033[33m"
# define BLUE    "\033[34m"
# define MAGENTA "\033[35m"
# define CYAN    "\033[36m"
# define WHITE   "\033[37m"
# define BLACK   "\033[30m"
# define BOLD    "\033[1m"
# define UNDERLINE "\033[4m"
# define ORANGE  "\033[38;2;255;165;0m"

#endif
