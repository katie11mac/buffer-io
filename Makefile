testmyio: myio.o testmyio.o
	gcc -o testmyio myio.o testmyio.o 

myio.o: myio.c
	gcc -Wall -pedantic -g -c -o myio.o myio.c

testmyio.o: testmyio.c 
	gcc -Wall -pedantic -g -c -o testmyio.o testmyio.c


.PHONY: clean
clean:
	rm -f myio myio.o testmyio.o writefile testSeekWrite
