CFLAGS = -Wall -I inc
CC = gcc
SRC = src/
SRCTST = tests/
INCLUDE = inc/
OBJ = obj/

OBJECTS = $(OBJ)data.o $(OBJ)entry.o $(OBJ)list.o $(OBJ)test_data.o $(OBJ)test_entry.o $(OBJ)test_list.o $(OBJ)test_serialization.o $(OBJ)test_table.o

all: out

out: data entry list test_data test_entry test_list test_serialization test_table

data: $(OBJ)data.o

$(OBJ)data.o: $(SRC)data.c $(INCLUDE)data.h
	$(CC) -c $(CFLAGS) $< -o $@

entry: $(OBJ)entry.o

$(OBJ)entry.o: $(SRC)entry.c $(INCLUDE)data.h $(INCLUDE)entry.h
	$(CC) -c $(CFLAGS) $< -o $@

list: $(OBJ)list.o

$(OBJ)list.o: $(SRC)list.c $(INCLUDE)data.h $(INCLUDE)entry.h $(INCLUDE)list.h
	$(CC) -c $(CFLAGS) $< -o $@

test_data: $(OBJ)test_data.o $(OBJ)data.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ)test_data.o: $(SRCTST)test_data.c $(INCLUDE)data.h
	$(CC) -c $(CFLAGS) $< -o $@

test_entry: $(OBJ)test_entry.o $(OBJ)data.o $(OBJ)entry.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ)test_entry.o: $(SRCTST)test_entry.c
	$(CC) -c $(CFLAGS) $< -o $@

test_list: $(OBJ)test_list.o $(OBJ)data.o $(OBJ)list.o $(OBJ)entry.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ)test_list.o: $(SRCTST)test_list.c  $(INCLUDE)data.h $(INCLUDE)entry.h $(INCLUDE)list.h
	$(CC) -c $(CFLAGS) $< -o $@

test_serialization: $(OBJ)test_serialization.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ)test_serialization.o: $(SRCTST)test_serialization.c $(INCLUDE)data.h $(INCLUDE)entry.h $(INCLUDE)serialization.h
	$(CC) -c $(CFLAGS) $< -o $@

test_table: $(OBJ)test_table.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ)test_table.o: $(SRCTST)test_table.c $(INCLUDE)data.h $(INCLUDE)entry.h $(INCLUDE)table.h
	$(CC) -c $(CFLAGS) $< -o $@

correr_test_data:
	$(CC) -g $(SRCTST)test_data.c $(OBJ)data.o -o test_data

clean:
	rm -rf $(OBJ) *.o test_data
