all: clean
	gcc -O2 -Wall -I. -g -DDEBUG -o test_galil test_galil.c -L/usr/lib -lgclib -lgclibo -lrt
clean:
	rm test_galil
