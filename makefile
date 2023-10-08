CC = gcc
CFLAGS = -Wall -Iinc
TSTEXESRC = testsExe/
OBJDIR = obj/

TARGET = $(TSTEXESRC)test_data
SRCS = tests/test_data.c src/data.c
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
