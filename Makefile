all : pseudo-shell

pseudo-shell : main.o string_parser.o cli.o command.o
	cd src && \
	gcc -g -o ../pseudo-shell main.o string_parser.o cli.o command.o


main.o : src/main.c 
	cd src && \
	gcc -c main.c


cli.o : src/cli.c src/cli.h
	cd src && \
	gcc -c cli.c

command.o : src/command.c src/command.h
	cd src && \
	gcc -g -c command.c

string_parser.o : src/string_parser.c src/string_parser.h
	cd src && \
	gcc -c string_parser.c


clean:
	rm -f core src/*.o pseudo-shell 