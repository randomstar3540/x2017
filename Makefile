CC=gcc-10
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -g -std=gnu11 -lasan

# fill in all your make rules

vm_x2017: 
	echo "vm_x2017"

objdump_x2017: objdump_x2017.c
	$(CC) $(CFLAGS) $^ -o $@

tests:
	echo "tests"

run_tests:
	echo "run_tests"

clean:
	echo "clean"

