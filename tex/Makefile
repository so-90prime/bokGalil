all: clean
	[ -f bokGalil.pdf ] && rm bokGalil.pdf || echo ""
	pdflatex bokGalil.tex
	pdflatex bokGalil.tex
	pdflatex bokGalil.tex
	make clean

clean:
	[ -f bokGalil.aux ] && rm bokGalil.aux || echo ""
	[ -f bokGalil.lof ] && rm bokGalil.lof || echo ""
	[ -f bokGalil.log ] && rm bokGalil.log || echo ""
	[ -f bokGalil.lot ] && rm bokGalil.lot || echo ""
	[ -f bokGalil.out ] && rm bokGalil.out || echo ""
	[ -f bokGalil.toc ] && rm bokGalil.toc || echo ""
