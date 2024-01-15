all:
	gcc main.c -o ft_ping -Wall -Wextra -Werror -pedantic -std=c99

clean:
	rm -f ft_ping

re: clean all