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
STARGETS  = Galil_DMC_22x0_NgClient Galil_DMC_22x0_NgServer
TTARGETS  = Galil_DMC_22x0_TCP_Write Galil_DMC_22x0_TCP_Read
UTARGETS  = Galil_DMC_22x0_UDP_Write Galil_DMC_22x0_UDP_Read Galil_DMC_22x0_Write_Memory
LIBRARIES = libbokGalil.so libbokGalil.a

HERE     := $(shell pwd)
BOK_HOME := $(shell env | grep BOK_GALIL_HOME | cut -d'=' -f2)
BOK_BIN  := $(shell env | grep BOK_GALIL_BIN  | cut -d'=' -f2)
BOK_ETC  := $(shell env | grep BOK_GALIL_ETC  | cut -d'=' -f2)
BOK_LIB  := $(shell env | grep BOK_GALIL_LIB  | cut -d'=' -f2)
BOK_LOG  := $(shell env | grep BOK_GALIL_LOG  | cut -d'=' -f2)
BOK_SRC  := $(shell env | grep BOK_GALIL_SRC  | cut -d'=' -f2)
BOK_TCL  := $(shell env | grep BOK_GALIL_TCL  | cut -d'=' -f2)


#///////////////////////////////////////////////////////////////////////////////
#// includes and flags
#///////////////////////////////////////////////////////////////////////////////
CFLAGS = -O2 -Wall -I$(BOK_SRC) -g -DDEBUG
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
	@echo "   where <sys> = 'banzai | bart | bonsai | lab'"
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

lab:
	rm -rf $(HERE)/__hosts__.h >> /dev/null 2>&1
	ln -sf $(HERE)/__hosts__.lab.h $(HERE)/__hosts__.h
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
	rm -f $(HERE)/libbokGalil.a >> /dev/null 2>&1
	rm -f $(HERE)/libbokGalil.so >> /dev/null 2>&1
	rm -f $(BOK_LIB)/libbokGalil.a >> /dev/null 2>&1
	rm -f $(BOK_LIB)/libbokGalil.so >> /dev/null 2>&1
	rm -rf $(HERE)/__pycache__ >> /dev/null 2>&1
	rm -f $(BOK_BIN)/Galil_DMC_22x0_TCP_Read.py >> /dev/null 2>&1
	rm -f $(BOK_BIN)/Galil_DMC_22x0_UDP_Read.py >> /dev/null 2>&1
	rm -rf /dev/shm/tcp_shm /dev/shm/udp_shm >> /dev/null 2>&1
	rm -f $(BOK_SRC)/__hosts__.py >> /dev/null 2>&1
	sed 's?\#define ??g' $(BOK_SRC)/__hosts__.h | awk '{gsub(/[ ]+/," = ")}1' >> $(BOK_SRC)/__hosts__.py
	rm -f $(BOK_TCL)/bokHosts.tcl >> /dev/null 2>&1
	echo "proc bokHosts { } {" >> $(BOK_TCL)/bokHosts.tcl
	echo "  global env bokLocals bokVersions bokVariables bokWidgets bokXopts" >> $(BOK_TCL)/bokHosts.tcl
	sed 's?\#define ?  set bokLocals\(?g' $(BOK_SRC)/__hosts__.h | awk  '{gsub(/[ ]+\"/,"\) \"")}1' | sed 's/_PORT/_PORT\)/g' >> $(BOK_TCL)/bokHosts.tcl
	echo "}" >> $(BOK_TCL)/bokHosts.tcl
	chmod 755 $(BOK_TCL)/bokHosts.tcl

everything:
	make clean
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
	cp $(HERE)/Galil_DMC_22x0_TCP_Read.py $(BOK_BIN)/
	cp $(HERE)/Galil_DMC_22x0_UDP_Read.py $(BOK_BIN)/
	touch $(BOK_BIN)/*

libbokGalil.a: bokGalil.c
	rm -rf $(HERE)/$@ $(BOK_LIB)/$@ $(HERE)/bokGalil.o >> /dev/null 2>&1
	gcc $(CFLAGS) -c $(BOK_SRC)/$< -o $(HERE)/bokGalil.o
	ar rcsv $(HERE)/$@ $(HERE)/bokGalil.o
	chmod 777 $(HERE)/$@
	rm -rf $(HERE)/bokGalil.o >> /dev/null 2>&1
	cp $(HERE)/libbokGalil.a  $(BOK_LIB)/libbokGalil.a

libbokGalil.so: bokGalil.c
	rm -rf $(HERE)/$@ $(BOK_LIB)/$@ $(HERE)/bokGalil.o >> /dev/null 2>&1
	gcc $(CFLAGS) -fPIC -shared -c $(BOK_SRC)/$< -o $(HERE)/bokGalil.o
	ld -shared -o $(HERE)/$@ $(HERE)/bokGalil.o -lc -ldl
	chmod 777 $(HERE)/$@
	rm -rf $(HERE)/bokGalil.o >> /dev/null 2>&1
	cp $(HERE)/libbokGalil.so $(BOK_LIB)/libbokGalil.so
