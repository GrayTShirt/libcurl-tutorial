.PHONY: clean
.SUFFIXES: .c .o

CFLAGS :=
CFLAGS += -Wall -Wextra
CFLAGS += -std=c99 -pedantic -g
CLFAGS += -pipe

LIBS=-lcurl

.c:
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

TARGET :=
TARGET += simple

all: $(TARGET)

clean:
	rm -f $(TARGET)
