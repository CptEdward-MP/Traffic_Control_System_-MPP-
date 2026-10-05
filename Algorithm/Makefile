CC = gcc
CFLAGS =  -O0 -g -Wall -Wextra -pedantic -Wshadow -Wuninitialized -Wno-unused-parameter
SOURCES = traffic_manager.c log.c
LIBS =  -lm
OUTPUT = qame

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CC)  $(CFLAGS) $(SOURCES) $(LIBS) -o $(OUTPUT)

err:
	$(CC) $(CFLAGS) $(SOURCES) $(LIBS) -o $(OUTPUT) >> err.txt 2>&1

clean:
	rm -rf $(OUTPUT) err.txt
