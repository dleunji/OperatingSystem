testlib : list.o bitmap.o hash.o hex_dump.o main.o
	gcc -o testlib list.o bitmap.o hash.o hex_dump.o main.o

list.o : list.c
	gcc -c -o list.o list.c

bitmap.o : bitmap.c
	gcc -c -o bitmap.o bitmap.c

hash.o : hash.c
	gcc -c -o hash.o hash.c

hex_dump : hex_dump.c
	gcc -c -o hex_dump.o hex_dump.c

main.o : main.c
	gcc -c -o main.o main.c

clean:
	rm *.o testlib
	rm ../os_hw1_tester/*.output
	rm ../os_hw1_tester/*.result
	rm Score.txt
