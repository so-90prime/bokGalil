#///////////////////////////////////////////////////////////////////////////////
#//
#//  tcl Makefile
#//
#///////////////////////////////////////////////////////////////////////////////


#///////////////////////////////////////////////////////////////////////////////
#// primary dependencies
#///////////////////////////////////////////////////////////////////////////////
HERE     := $(shell pwd)


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
	rm -rf $(HERE)/bokParams.txt >> /dev/null 2>&1
	ln -sf $(HERE)/bokParams.banzai.txt $(HERE)/bokParams.txt

bart:
	rm -rf $(HERE)/bokParams.txt >> /dev/null 2>&1
	ln -sf $(HERE)/bokParams.bart.txt $(HERE)/bokParams.txt

bonsai:
	rm -rf $(HERE)/bokParams.txt >> /dev/null 2>&1
	ln -sf $(HERE)/bokParams.bonsai.txt $(HERE)/bokParams.txt

home:
	rm -rf $(HERE)/bokParams.txt >> /dev/null 2>&1
	ln -sf $(HERE)/bokParams.home.txt $(HERE)/bokParams.txt

