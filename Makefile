CC=gcc
CFLAGS=-Os -ffunction-sections -fdata-sections -s
LDFLAGS=-Wl,-Map=object.map,--cref,--gc-section

# fill in all your make rules

vm_x2017: fetch_x2017.c vm_x2017.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	gzexe $@

objdump_x2017: fetch_x2017.c objdump_x2017.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	gzexe $@

tests:
	make vm_x2017
	make objdump_x2017

run_tests:
	./tests.sh

clean:
	rm objdump_x2017
	rm vm_x2017

