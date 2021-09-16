CC=gcc
CFLAGS=-fsanitize=address -Wall -Werror -std=gnu11 -g -lm
LDFLAGS=-Wl,--gc-section

# fill in all your make rules

vm_x2017: fetch_x2017.c vm_x2017.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

objdump_x2017: fetch_x2017.c objdump_x2017.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

run_tests:
	bash test.sh

clean:
	rm objdump_x2017
	rm vm_x2017

