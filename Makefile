myio: myio.c
	gcc -Wall -pedantic -o myio myio.c

.PHONY: clean

clean:
	rm -f myio
