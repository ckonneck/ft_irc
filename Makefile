NAME = ircserv
CC = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g
SRCS =  main.cpp utils.cpp User.cpp Server.cpp Chatroom.cpp Parsing.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o $(NAME)
	
%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

sanitize:
	$(MAKE) CFLAGS="$(CFLAGS) -fsanitize=address -fsanitize=undefined" all

# valgrind rule
valgrind: $(NAME)
		valgrind --leak-check=full --track-origins=yes ./$(NAME) $(ARGS)
		
# extra options: --verbose --show-leak-kinds=all --log-file=valgrind-out.txt
#norminette rule
#norminette: $(SRSC)
#		norminette $(SRSC)

# Declare phony targets
.PHONY: all clean fclean re