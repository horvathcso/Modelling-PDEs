CC = gcc
LD = gcc
CFLAGS = 
LDFLAGS = -Ofast
RM = /bin/rm -f
OBJS = cachetest.o
EXECUTABLE = cachetest

all:$(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(EXECUTABLE)

cachetest.o: cachetest.c
	$(CC) $(CFLAGS) -c cachetest.c

clean:
	$(RM) $(EXECUTABLE) $(OBJS)
