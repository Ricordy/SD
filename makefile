CFLAGS = -Wall -I include -g -no-pie 
CC = gcc
SRC = source
INCLUDE = include
OBJ = object/general
OBJS = object/professor
BIN = binary
LIB = lib

EXECUTABLES = table_client table_server

LIBTABLE_OBJS = $(OBJS)/data.o $(OBJS)/entry.o $(OBJS)/list.o $(OBJS)/table.o


out: sdmessage libtable message.o private_functions.o server_redundancy.o stats.o client_stub.o network_client.o table_client.o sdmessage.o network_server.o table_server.o table_skel.o $(EXECUTABLES)


table_client: $(OBJ)/table_client.o $(OBJ)/message.o $(OBJ)/client_stub.o $(OBJ)/network_client.o $(OBJS)/data.o $(OBJS)/entry.o $(OBJS)/list.o $(OBJS)/table.o $(OBJ)/sdmessage.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ -lprotobuf-c -lzookeeper_mt -D THREADED

table_server:  $(OBJ)/network_server.o $(OBJ)/table_server.o $(OBJ)/message.o $(OBJ)/stats.o $(OBJS)/data.o $(OBJS)/entry.o $(OBJS)/list.o $(OBJS)/table.o $(OBJ)/sdmessage.o $(OBJ)/table_skel.o $(OBJ)/server_redundancy.o $(OBJ)/client_stub.o $(OBJ)/network_client.o $(OBJ)/private_functions.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ -lprotobuf-c -lzookeeper_mt -D THREADED

libtable: $(LIB)/libtable.a

$(LIB)/libtable.a: $(LIBTABLE_OBJS)
	ar rcs $@ $^

data.o: $(SRC)/data.c $(INCLUDE)/data.h
	$(CC) -c $(CFLAGS) $< -o $(OBJS)/$@

entry.o: $(SRC)/entry.c $(INCLUDE)/data.h $(INCLUDE)/entry.h
	$(CC) -c $(CFLAGS) $< -o $(OBJS)/$@

list.o: $(SRC)/list.c $(INCLUDE)/data.h $(INCLUDE)/entry.h $(INCLUDE)/list.h
	$(CC) -c $(CFLAGS) $< -o $(OBJS)/$@

table.o: $(SRC)/table.c $(INCLUDE)/data.h $(INCLUDE)/entry.h $(INCLUDE)/table.h
	$(CC) -c $(CFLAGS) $< -o $(OBJS)/$@

stats.o: $(SRC)/stats.c $(INCLUDE)/stats.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

sdmessage.o: $(SRC)/sdmessage.pb-c.c $(INCLUDE)/sdmessage.pb-c.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

sdmessage: 
	protoc --c_out=. sdmessage.proto
	mv sdmessage.pb-c.c $(SRC)/
	mv sdmessage.pb-c.h $(INCLUDE)/

client_stub.o: $(SRC)/client_stub.c $(INCLUDE)/client_stub.h  $(INCLUDE)/data.h $(INCLUDE)/entry.h $(INCLUDE)/network_client.h  $(INCLUDE)/message-private.h  $(INCLUDE)/client_stub-private.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

table_client.o: $(SRC)/table_client.c $(INCLUDE)/client_stub.h  $(INCLUDE)/network_client.h   $(INCLUDE)/client_stub-private.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

table_server.o: $(SRC)/table_server.c $(SRC)/stats.c $(INCLUDE)/table_skel.h  $(INCLUDE)/network_server.h $(INCLUDE)/message-private.h $(INCLUDE)/sdmessage.pb-c.h $(INCLUDE)/server_redundancy.h $(INCLUDE)/private_functions.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

table_skel.o: $(SRC)/table_skel.c $(SRC)/stats.c $(INCLUDE)/table_skel.h  $(INCLUDE)/sdmessage.pb-c.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

network_client.o: $(SRC)/network_client.c $(INCLUDE)/network_client.h 
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

network_server.o: $(SRC)/network_server.c $(SRC)/stats.c $(SRC)/table_server.c $(INCLUDE)/network_server.h $(INCLUDE)/message-private.h $(INCLUDE)/sdmessage.pb-c.h $(INCLUDE)/server_redundancy.h $(INCLUDE)/client_stub.h $(INCLUDE)/client_stub-private.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

message.o: $(SRC)/message.c $(INCLUDE)/message-private.h $(INCLUDE)/sdmessage.pb-c.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

server_redundancy.o: $(SRC)/server_redundancy.c $(INCLUDE)/server_redundancy.h 
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

private_functions.o: $(SRC)/private_functions.c $(INCLUDE)/private_functions.h 
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

clean:
	rm -f $(OBJ)/*
	rm -f $(SRC)/sdmessage.pb-c.c
	rm -f $(INCLUDE)/sdmessage.pb-c.h
	rm -f $(BIN)/*
	rm -f $(LIB)/*
	
