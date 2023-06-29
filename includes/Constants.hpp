#ifndef CONSTANTS_H
# define CONSTANTS_H

# define SERVER ":IRCSERV"

// SUCCESSFUL CONSTANTS MESSAGES
# define RPL_WELCOME "001"
# define RPL_NOTOPIC "331"
# define RPL_TOPIC "332"
# define RPL_CHANNELMODEIS "324"


// ERROR CONSTANTS MESSAGES
# define ERR_PASSWDMISMATCH "464"
# define ERR_NEEDMOREPARAMS "461"
# define ERR_NICKNAMEINUSE "433"
# define ERR_ALREADYREGISTRED "462"
# define ERR_INVITEONLYCHAN "473"
# define ERR_NOSUCHCHANNEL "403"
# define ERR_NOTONCHANNEL "442"
# define ERR_CHANOPRIVSNEEDED "482"
# define ERR_UNKNOWNMODE "472"
# define ERR_NOSUCHNICK "401"
# define ERR_USERONCHANNEL "443"
# define ERR_NORECIPIENT "411"
# define ERR_NOTEXTTOSEND "412"
# define ERR_NOSUCHSERVER "402"

#endif