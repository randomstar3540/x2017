CC=gcc
CFLAGS=-Os -ffunction-sections -fdata-sections -s
LDFLAGS=-Wl,-Map=object.map,--cref,--gc-section

# fill in all your make rules

vm_x2017: fetch_x2017.c vm_x2017.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	objcopy -R .gnu.version -R .comment -R .gnu.hash -R .note.ABI-tag -R .note.gnu.build-id -R .note.gnu.property -R .eh_frame vm_x2017

objdump_x2017: fetch_x2017.c objdump_x2017.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

run_tests:
	bash test.sh

clean:
	rm objdump_x2017
	rm vm_x2017

