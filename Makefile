# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pvivian <pvivian@student.21-school.ru>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/05/05 16:47:32 by pvivian           #+#    #+#              #
#    Updated: 2021/06/13 12:06:47 by pvivian          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME =		webserv
CC =		clang++

FLAGS =		-Wall -Werror -Wextra -g -Wunreachable-code -MMD  -I ./sources -std=c++98
# -fsanitize=address
# -std=c++98

MAIN = $(SRCS_DIR)main.cpp

SRCS =		main.cpp \
			Cluster.cpp \
			Config.cpp \
			Server.cpp \
			Handler.cpp \
			Session.cpp \
			ParseRequest.cpp\


SRCS_DIR = ./sources/

OBJS =	$(SRCS:.cpp=.o)
OBJS_DIR = ./objects/
OBJ = $(addprefix $(OBJS_DIR), $(OBJS))
DEP = $(OBJ:.o=.d)

LOGFILE = *_log


all: $(NAME)

$(NAME): $(OBJ) $(MAIN)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

-include $(DEP)

$(OBJS_DIR)%.o: $(SRCS_DIR)%.cpp
	test -d $(OBJS_DIR) || mkdir $(OBJS_DIR)
	$(CC) $(FLAGS) -c $< -o $@


clean:
	rm -rf $(OBJS_DIR)

fclean:
	rm -rf $(OBJS_DIR) $(NAME) $(LOGFILE)
	rm -rf ./cgi/temp
	./clear.sh
	
re: fclean all

.PHONY: all clean fclean re
