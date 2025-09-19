#///////////////////////////////////////////////////////////////////////////////
#//
#//  bokGalil Makefile
#//
#///////////////////////////////////////////////////////////////////////////////


#///////////////////////////////////////////////////////////////////////////////
#// primary dependencies
#///////////////////////////////////////////////////////////////////////////////
ATARGETS  = bok_read_filters bok_read_guider_filters bok_read_instrument_filters bok_read_nominal_plane Galil_DMC_22x0_CLI
DTARGETS  = bokGalilIndiDriver
PTARGETS  = Galil_DMC_22x0_NgClient.py  Galil_DMC_22x0_NgClient_Requests.py  Galil_DMC_22x0_Read_Telemetry.py  Galil_DMC_22x0_TCP_Read.py  Galil_DMC_22x0_UDP_Read.py
STARGETS  = Galil_DMC_22x0_NgClient Galil_DMC_22x0_NgServer
TTARGETS  = Galil_DMC_22x0_TCP_Write Galil_DMC_22x0_TCP_Read
UTARGETS  = Galil_DMC_22x0_UDP_Write Galil_DMC_22x0_UDP_Read Galil_DMC_22x0_Write_Memory
LIBRARIES = libbokGalil.so libbokGalil.a

setup := $(shell /usr/bin/bash -c "cd ../ && . etc/bokGalil.sh && env")
export setup
BOK_GALIL_HOME := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_HOME | cut -d'=' -f2)
BOK_GALIL_BIN  := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_BIN  | cut -d'=' -f2)
BOK_GALIL_DOCS := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_DOCS | cut -d'=' -f2)
BOK_GALIL_ETC  := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_ETC  | cut -d'=' -f2)
BOK_GALIL_LIB  := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_LIB  | cut -d'=' -f2)
BOK_GALIL_LOG  := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_LOG  | cut -d'=' -f2)
BOK_GALIL_SRC  := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_SRC  | cut -d'=' -f2)
BOK_GALIL_TCL  := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_TCL  | cut -d'=' -f2)
BOK_GALIL_TEX  := $(shell echo $$setup | tr " " "\n" | grep BOK_GALIL_TEX  | cut -d'=' -f2)

LD_LIBRARY_PATH := $(shell echo $$setup | tr " " "\n" | grep LD_LIBRARY_PATH  | cut -d'=' -f2)
PYTHON_PATH     := $(shell echo $$setup | tr " " "\n" | grep PYTHON_PATH  | cut -d'=' -f2)
export LD_LIBRARY_PATH 
export PYTHON_PATH     

HERE     := $(shell pwd)
BOK_HOME := $(BOK_GALIL_HOME)
BOK_BIN  := $(BOK_GALIL_BIN)
BOK_ETC  := $(BOK_GALIL_ETC)
BOK_LIB  := $(BOK_GALIL_LIB)
BOK_LOG  := $(BOK_GALIL_LOG)
BOK_SRC  := $(BOK_GALIL_SRC)
BOK_TCL  := $(BOK_GALIL_TCL)


#///////////////////////////////////////////////////////////////////////////////
#// includes and flags
#///////////////////////////////////////////////////////////////////////////////
CFLAGS = -O2 -Wall -I$(BOK_SRC) -g -DDEBUG # optimize level 2, warn all, include directory, produce debugging info, predefine macro `DEBUG 1`
GLIBS = -L/usr/lib -lgclib -lgclibo -lrt
DLIBS = -lindidriver -lm -lcurl
LLIBS = -L$(BOK_LIB) -lbokGalil
SLIBS = -lm -lpthread


#///////////////////////////////////////////////////////////////////////////////
#// rules
#///////////////////////////////////////////////////////////////////////////////
help:
	clear
	@echo ""
	@echo "Main targets are available:"
	@echo ""
	@echo "   all clean everything help install"
	@echo ""
	@echo "To re-build the code, use: % make <sys>"
	@echo "   where <sys> = 'banzai | bart | bonsai | home'"
	@echo ""

banzai:
	rm -rf $(HERE)/__hosts__.h >> /dev/null 2>&1
	ln -sf $(HERE)/__hosts__.banzai.h $(HERE)/__hosts__.h
	make everything

bart:
	rm -rf $(HERE)/__hosts__.h >> /dev/null 2>&1
	ln -sf $(HERE)/__hosts__.bart.h $(HERE)/__hosts__.h
	make everything

bonsai:
	rm -rf $(HERE)/__hosts__.h >> /dev/null 2>&1
	ln -sf $(HERE)/__hosts__.bonsai.h $(HERE)/__hosts__.h
	make everything

home:
	rm -rf $(HERE)/__hosts__.h >> /dev/null 2>&1
	ln -sf $(HERE)/__hosts__.home.h $(HERE)/__hosts__.h
	make everything

steward:
	rm -rf $(HERE)/__hosts__.h >> /dev/null 2>&1
	ln -sf $(HERE)/__hosts__.steward.h $(HERE)/__hosts__.h
	make everything

all:
	for target in $(ATARGETS); do\
	 gcc $(CFLAGS) -o $(HERE)/$$target $(HERE)/$$target.c $(LLIBS) $(GLIBS);\
	done
	for target in $(DTARGETS); do\
	 gcc $(CFLAGS) -o $(HERE)/$$target $(HERE)/$$target.c $(LLIBS) $(DLIBS) $(GLIBS);\
	done
	for target in $(STARGETS); do\
	 gcc $(CFLAGS) -o $(HERE)/$$target $(HERE)/$$target.c $(LLIBS) $(SLIBS) $(GLIBS);\
	done
	for target in $(TTARGETS); do\
	 gcc $(CFLAGS) -o $(HERE)/$$target $(HERE)/$$target.c $(LLIBS) $(GLIBS);\
	done
	for target in $(UTARGETS); do\
	 gcc $(CFLAGS) -o $(HERE)/$$target $(HERE)/$$target.c $(LLIBS) $(GLIBS);\
	done

clean:
	for target in $(ATARGETS); do\
	 rm -rf $(HERE)/$$target >> /dev/null 2>&1;\
	 rm -rf $(BOK_BIN)/$$target >> /dev/null 2>&1;\
	done
	for target in $(DTARGETS); do\
	 rm -rf $(HERE)/$$target >> /dev/null 2>&1;\
	 rm -rf $(BOK_BIN)/$$target >> /dev/null 2>&1;\
	done
	for target in $(PTARGETS); do\
	 rm -rf $(BOK_BIN)/$$target >> /dev/null 2>&1;\
	done
	for target in $(STARGETS); do\
	 rm -rf $(HERE)/$$target >> /dev/null 2>&1;\
	 rm -rf $(BOK_BIN)/$$target >> /dev/null 2>&1;\
	done
	for target in $(TTARGETS); do\
	 rm -rf $(HERE)/$$target >> /dev/null 2>&1;\
	 rm -rf $(BOK_BIN)/$$target >> /dev/null 2>&1;\
	done
	for target in $(UTARGETS); do\
	 rm -rf $(HERE)/$$target >> /dev/null 2>&1;\
	 rm -rf $(BOK_BIN)/$$target >> /dev/null 2>&1;\
	done

extraclean:
	rm -f $(HERE)/libbokGalil.a >> /dev/null 2>&1
	rm -f $(HERE)/libbokGalil.so >> /dev/null 2>&1
	rm -f $(BOK_LIB)/libbokGalil.a >> /dev/null 2>&1
	rm -f $(BOK_LIB)/libbokGalil.so >> /dev/null 2>&1
	rm -rf $(HERE)/__pycache__ >> /dev/null 2>&1
	rm -rf /dev/shm/tcp_shm /dev/shm/udp_shm >> /dev/null 2>&1
	rm -f $(BOK_SRC)/__hosts__.py >> /dev/null 2>&1
	sed 's?\#define ??g' $(BOK_SRC)/__hosts__.h | awk '{gsub(/[ ]+/," = ")}1' >> $(BOK_SRC)/__hosts__.py

everything:
	make clean
	make extraclean
	make libbokGalil.so libbokGalil.a
	make all
	make install

install:
	for target in $(ATARGETS); do\
	 mv $(HERE)/$$target $(BOK_BIN)/$$target; \
	done
	for target in $(DTARGETS); do\
	 mv $(HERE)/$$target $(BOK_BIN)/$$target; \
	done
	for target in $(PTARGETS); do\
	 cp $(HERE)/$$target $(BOK_BIN)/$$target; \
	done
	for target in $(STARGETS); do\
	 mv $(HERE)/$$target $(BOK_BIN)/$$target; \
	done
	for target in $(TTARGETS); do\
	 mv $(HERE)/$$target $(BOK_BIN)/$$target; \
	done
	for target in $(UTARGETS); do\
	 mv $(HERE)/$$target $(BOK_BIN)/$$target; \
	done
	mv $(HERE)/libbokGalil.a  $(BOK_LIB)/libbokGalil.a
	mv $(HERE)/libbokGalil.so $(BOK_LIB)/libbokGalil.so
	touch $(BOK_BIN)/*

libbokGalil.a: bokGalil.c
	rm -rf $(HERE)/$@ $(BOK_LIB)/$@ $(HERE)/bokGalil.o >> /dev/null 2>&1
	gcc -x c++ $(CFLAGS) -c $(BOK_SRC)/$^ -o $(HERE)/bokGalil.o
	ar rcsv $(HERE)/$@ $(HERE)/bokGalil.o
	chmod 777 $(HERE)/$@
	rm -rf $(HERE)/bokGalil.o >> /dev/null 2>&1
	cp $(HERE)/$@  $(BOK_LIB)/$@

libbokGalil.so: bokGalil.c
	rm -rf $(HERE)/$@ $(BOK_LIB)/$@ $(HERE)/bokGalil.o >> /dev/null 2>&1
	gcc -x c++ $(CFLAGS) -fPIC -shared -c $(BOK_SRC)/$< -o $(HERE)/bokGalil.o
	ld -shared -o $(HERE)/$@ $(HERE)/bokGalil.o -lc -ldl -lgclib
	chmod 777 $(HERE)/$@
	rm -rf $(HERE)/bokGalil.o >> /dev/null 2>&1
	cp $(HERE)/$@ $(BOK_LIB)/$@
