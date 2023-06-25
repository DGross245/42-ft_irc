# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dgross <dgross@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/31 08:49:39 by dgross            #+#    #+#              #
#    Updated: 2023/06/25 12:39:49 by dgross           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= ircserv

VPATH			= includes: src:

SRC				= main.cpp\
				  server.cpp\
				  client.cpp \
				  channel.cpp \
				  parser.cpp

HEADER			= Channel.hpp \
				  Client.hpp \
				  Server.hpp \
				  Parser.hpp

OBJ_DIR			= ./obj/

OBJ				= $(addprefix $(OBJ_DIR),$(SRC:.cpp=.o))

CC				= c++

CFLAGS			= -Wall -Wextra -Werror -std=c++98 -g

all: $(NAME)

obj:
	@mkdir -p $(OBJ_DIR)

obj/%.o: %.cpp
	@$(CC) $(CFLAGS) -I./includes -c $< -o $@

$(NAME): obj $(OBJ)
	@$(CC) $(OBJ) $(CFLAGS) -o $(NAME)

clean:
	@$(RM) -rf obj

fclean: clean
	@$(RM) -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re