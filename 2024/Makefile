NAME = WebServer

CFILES = main.cpp WebServer.cpp  NetworkClient.cpp HttpRequest.cpp

HFILES = WebServer.hpp  NetworkClient.hpp HttpRequest.hpp

CC = c++

FLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(CFILES) $(HFILES)
	$(CC) $(FLAGS) $(CFILES) -o $@

clean:
	rm -f $(NAME)

fclean: clean

re: fclean all