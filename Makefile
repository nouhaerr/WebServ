CPP = c++

FLAGS = -Wall -Wextra -Werror -fsanitize=address -g3 #-std=c++98 

SRC =	src/main.cpp \
		src/parsing/Config.cpp src/parsing/ParseFile.cpp\
		src/parsing/ConfigServer.cpp src/parsing/utils.cpp\
		src/parsing/ConfigLocation.cpp \

OBJS = $(SRC:.cpp=.o)

NAME = webserv

$(NAME) : $(OBJS)
	@echo "\033[0;93m- Waiting ...\033[0m"
	@$(CPP) $(FLAGS) $(OBJS) -o $(NAME)
	@echo "\033[0;32m- Webserver is ready ^^ \033[0m"

all : $(NAME) $(SRC)

clean :
	@rm -rf $(OBJS)
	@echo "\033[1;31m- Object files removed!\033[0m"

fclean : clean
	@rm -rf $(NAME)
	@echo "\033[1;31m- All cleaned.\033[0;0m"

re : clean all

.SILENT : $(OBJS)

.PHONY: all clean fclean re