all: linux
linux: clean
	gcc -O2 -Wall -I. -g -DDEBUG -o test_galil test_galil.c -L/usr/lib -lgclib -lgclibo -lrt
mac: clean
	gcc -O2 -Wall -I/Applications/gclib/include -I. -g -DDEBUG -o test_galil test_galil.c /Applications/gclib/dylib/gclib.0.dylib /Applications/gclib/dylib/gclibo.0.dylib
	[ -d test_galil.dSYM ] && rm -rf test_galil.dSYM || echo ""
clean:
	[ -f test_galil ] && rm test_galil || echo ""
	[ -d test_galil.dSYM ] && rm -rf test_galil.dSYM || echo ""
