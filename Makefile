#MAKEFLAGS += --silent

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -g
CDEBUGFLAGS = -g
INC_DIR = inc/
INC = ft_ping.h

EXE = ft_ping

SRC_DIR = src/
SRC =	main.c\
		ft_ping.c\
		signal.c\
		stats.c

OBJ_DIR = .obj/
OBJ = $(SRC:%.c=$(OBJ_DIR)%.o)

.PHONY : all clean fclean re $(EXE)

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXE) -lm
	@echo "------------\n"

$(OBJ_DIR)%.o: $(SRC_DIR)%.c $(addprefix $(INC_DIR),$(INC))
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR)

run_container:
	docker build . -t ft_ping
	docker run --cap-add=NET_ADMIN -it -v ./:/shared --name ft_ping ft_ping /bin/bash

shell:
	docker exec -it ft_ping /bin/bash

rm_container:
	docker container prune

clean:
	@/bin/rm -rf $(OBJ_DIR)

fclean: clean
	@/bin/rm -f $(EXE)

re: 
	$(MAKE) fclean
	$(MAKE) all
