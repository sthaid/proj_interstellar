
TARGETS = interstellar

CC = gcc
CFLAGS = -g -O2 -Wall -lm -lrt

all: $(TARGETS)

#
# build rules
#

interstallar: interstallar.c
	$(CC) $(CFLAGS) $< -o $@

#
# clean rule
#

clean:
	rm -f $(TARGETS)

