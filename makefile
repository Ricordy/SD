CC = gcc
CFLAGS = -Wall -Iinc
TSTEXESRC = testsExe/
OBJDIR = obj/

TARGET = $(TSTEXESRC)test_data $(TSTEXESRC)test_list $(TSTEXESRC)test_entry
SRCS = tests/test_data.c tests/test_list.c tests/test_entry.c src/data.c src/list.c src/entry.c
OBJS = $(patsubst %.c,$(OBJDIR)%.o,$(SRCS))

all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJDIR)%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET)
	rm -rf $(OBJDIR)
