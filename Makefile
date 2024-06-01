CPP = c++

FLAGS = -Wall -Wextra -Werror -fsanitize=address -g3 -std=c++98

SRC = src/main.cpp\
	src/parsing/Config.cpp src/parsing/ParseFile.cpp\
	src/parsing/ConfigServer.cpp src/parsing/utils.cpp\
	src/parsing/ConfigLocation.cpp\
	src/networking/WebServer.cpp src/networking/NetworkClient.cpp \
	src/networking/HttpRequest.cpp \
	src/networking/requestBody.cpp \
	src/networking/HttpRequestParser.cpp\
	src/response/HttpResponse.cpp\
	src/response/GET.cpp\

OBJS = $(SRC:.cpp=.o)

NAME = webserv

$(NAME) : $(OBJS)
	@$(CPP) $(FLAGS) $(OBJS) -o $(NAME)
	@echo "\033[0;32m- Webserver is ready ^^ \033[0m"

%.o: %.cpp
	@printf "\033[0;93m- Waiting...\033[0m \n"
	@$(CPP) $(FLAGS) -c $< -o $@

all : $(NAME) $(SRC)

clean :
	@rm -rf $(OBJS)
	@echo "\033[1;31m- Object files removed !\033[0m"

fclean : clean
	@rm -rf $(NAME)
	@echo "\033[1;31m- Binaries removed !\033[0;0m"

re : clean all

.SILENT : $(OBJS)

.PHONY: all clean fclean re
