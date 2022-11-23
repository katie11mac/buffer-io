CFLAGS=-Wall -pedantic -g -c -o

testmyio: myio.o testmyio.o
	gcc -o $@ myio.o testmyio.o 

%.o: %.c
	gcc $(CFLAGS) $@ $^

.PHONY: resetfiles
resetfiles: 
	cat testfile > readwritefile
	cat testfile > writereadfile
	cat testfile > writereadseekfile

.PHONY: clean
clean:
	rm -f myio myio.o testmyio.o writefile testSeekWrite
