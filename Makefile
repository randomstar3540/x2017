CC=gcc
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -g -std=gnu11 -lasan

# fill in all your make rules

vm_x2017: fetch_x2017.c vm_x2017.c
	$(CC) $(CFLAGS) $^ -o $@

objdump_x2017: fetch_x2017.c objdump_x2017.c
	$(CC) $(CFLAGS) $^ -o $@

tests:
	echo "tests"

run_tests:
	echo "run_tests"

clean:
	echo "clean"

