#Makefile
CC=g++ -Wall -g -pg

DFLAGS= -lpthread -I/usr/local/include -I/usr/local/include/cppconn -L/usr/local/lib  -lmysqlcppconn -lgsl -lgslcblas -lm

LIB=error.o sock.o util.o config.o
BUFMANAGER=bufManager.o bufManagerByLRU.o bufManagerByLFU.o bufManagerByDW.o bufManagerByLFRU.o bufManagerByLRFU.o 
#bufManagerByILRFU.o bufManagerByPLFU.o
CLIENT=client.o clientManager.o clientEntrance.o 
SERVER=server.o serverEntrance.o MYSQLPool.o

target= client server testlognormal fileStatistics
all:$(target)

fileStatistics:$(LIB) fileStatistics.o
	$(CC) $^ -o $@ $(DFLAGS)
testlognormal:$(LIB) testlognormal.o
	$(CC) $^ -o $@ $(DFLAGS)
client: $(LIB) $(CLIENT) $(BUFMANAGER)
	$(CC) $^ -o $@ $(DFLAGS)
server: $(LIB) $(SERVER)
	$(CC) $^ -o $@ $(DFLAGS)

%.o:lib/%.cpp
	$(CC) -c $^
%.o:src/%.cpp
	$(CC) -c $^
%.o:test/%.cpp
	$(CC) -c $^

clean:
	rm -rf *.o
	rm -rf $(target)

#end
