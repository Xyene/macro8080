all: i8080

i8080: core.o keyboard.o terminal.o
	gcc -Wall -o $@ $^

clean:
	rm -rf *.o
	rm -f i8080

%.o: %.c
	gcc -std=gnu99 -Wall -c $<
