############################################################################
##
## Copyright (C) 2002, Karlsruhe University
##
## File path:	doc/notes/Makefile
## Description:	Create documents from LaTeX documents.
##
## @LICENSE@
##
## $Id: Makefile,v 1.2 2002/10/04 11:14:38 joshua Exp $
##
############################################################################

BUILD = tex-build
PPC_SRC = ppc.tex notes.bib

all: ppc.ps

ppc.pdf: $(PPC_SRC)
	mkdir -p $@-build
	cp -f $^ $@-build/.
	(cd $@-build && \
	 pdflatex ppc.tex && \
	 bibtex ppc && \
	 pdflatex ppc.tex && \
	 pdflatex ppc.tex && \
	 cp ppc.pdf ../. \
	)

ppc.ps: $(PPC_SRC)
	mkdir -p $@-build
	cp -f $^ $@-build/.
	(cd $@-build && \
	 latex ppc.tex && \
	 bibtex ppc && \
	 latex ppc.tex && \
	 latex ppc.tex && \
	 dvips ppc -o ppc.ps && \
	 cp ppc.ps ../. \
	)


clean:
	rm -rf ppc.pdf-build
	rm -rf ppc.ps-build

