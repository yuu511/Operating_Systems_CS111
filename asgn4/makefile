CC = cc
CFLAGS = -Wall -g
CFLAGS_F = `pkgconf fuse --cflags --libs`
SRC = mk_disk.c
SRC_F = fuse_routines.c
OBJECTS = ${SRC:.c=.o} ${C_LO} 
OBJECTS_F = ${SRC_F:.c=.o} ${C_LO} 
EXECBIN = mk_disk
EXECBIN_F = fuse_routines
DISK =  NEWFS

all : ${EXECBIN} ${EXECBIN_F}

mk_disk.o:
	$(CC) -c $< $(CFLAGS)  

$(EXECBIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(EXECBIN_F):
	$(CC) $(SRC_F) -o $@ $(CFLAGS_F)

clean :
	rm -f $(OBJECTS) $(EXECBIN) $(OBJECTS_F) $(EXECBIN_F) $(DISK) 
