OBJS = Host.o Server.o Packet.o Route.o RoutePacket.o
OBJS2= Client.o Packet.o
CC = clang++
CFLAGS= -Wall -g -std=c++11 -pthread

all: Server Client

# $@ reps the target of the rule
Server: $(OBJS)  
	$(CC) $(CFLAGS) $(OBJS) -o $@
Client: $(OBJS2)  
	$(CC) $(CFLAGS) $(OBJS2) -o $@
# $< reps the name of the 1st dependency of our rule.
# $^ if used would rep the dependcy list of the rule.
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< 

#Removal of files that have been built

clean:
	rm -f *.o *~

clean-all: clean
	rm -f Server
	rm -f Client
