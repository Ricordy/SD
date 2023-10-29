CFLAGS = -Wall -I include -g
CC = gcc
SRC = source
INCLUDE = include
OBJ = object
BIN = binary

EXECUTABLES = table_client table_server

out: sdmessage message.o client_stub.o network_client.o table_client.o sdmessage.o network_server.o table_skel.o table_server.o $(EXECUTABLES)


table_client: $(OBJ)/table_client.o $(OBJ)/message.o $(OBJ)/client_stub.o $(OBJ)/network_client.o $(OBJ)/data.o $(OBJ)/entry.o $(OBJ)/list.o $(OBJ)/table.o $(OBJ)/sdmessage.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ -lprotobuf-c

table_server: $(OBJ)/table_server.o $(OBJ)/message.o $(OBJ)/network_server.o $(OBJ)/data.o $(OBJ)/entry.o $(OBJ)/list.o $(OBJ)/table.o $(OBJ)/sdmessage.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ -lprotobuf-c

data.o: $(SRC)/data.c $(INCLUDE)/data.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

entry.o: $(SRC)/entry.c $(INCLUDE)/data.h $(INCLUDE)/entry.h
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

table_server.o: $(SRC)/table_server.c $(INCLUDE)/table_skel.h  $(INCLUDE)/network_server.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

table_skel.o: $(SRC)/table_skel.c $(INCLUDE)/table_skel.h  $(INCLUDE)/sdmessage.pb-c.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

network_client.o: $(SRC)/network_client.c $(INCLUDE)/network_client.h 
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

network_server.o: $(SRC)/network_server.c $(INCLUDE)/network_server.h $(INCLUDE)/message-private.h $(INCLUDE)/sdmessage.pb-c.h
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

message.o: $(SRC)/message.c $(INCLUDE)/message-private.h 
	$(CC) -c $(CFLAGS) $< -o $(OBJ)/$@

clean:
	rm -f $(OBJ)/*
	rm -f $(SRC)/sdmessage.pb-c.c
	rm -f $(INCLUDE)/sdmessage.pb-c.h
	
