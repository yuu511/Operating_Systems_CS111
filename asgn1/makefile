CC = cc
CFLAGS = -Wall -g
SRC = argshell.c
SRC_L = shell.l
C_L = lex.yy.c
C_LO = ${C_L:.c=.o}
OBJECTS = ${SRC:.c=.o} ${C_LO}
EXECBIN = argshell

all : ${EXECBIN}

$(C_L): $(SRC_L)
	flex --outfile=$(C_L) $(SRC_L)

%.o: %.c
	$(CC) -c $< $(CFLAGS)  

$(EXECBIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -lfl

clean :
	rm $(OBJECTS) $(EXECBIN) $(C_L)
