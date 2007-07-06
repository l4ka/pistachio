LATEX=		latex
PDFLATEX=	pdflatex
MAKEINDEX=	makeindex
DVIPS=		dvips
RM_F=		rm -f
CP=		cp
TOUCH=		touch
SORT=		sort -bfi
MKDIR=		mkdir
SHELL=		bash

TARGET=		whitepaper
SOURCE=		whitepaper.tex
RMFILE=		$(TARGET).dvi $(TARGET).pdf
RUNLATEX=$(LATEX)

.SUFFIXES:
.SUFFIXES:	.tex .toc .idx .ind .glo .gls .pdf .dvi .ps


all:	pdf

ps:	$(TARGET).ps
pdf:	$(TARGET).pdf

.tex.pdf:
	${TOUCH} $*.gls $*.ind
	${PDFLATEX} $(LATEXFLAGS) $<
	${PDFLATEX} $(LATEXFLAGS) $<

.tex.dvi:
	${TOUCH} $*.gls $*.ind 
	${LATEX} $(LATEXFLAGS) $<

.dvi.ps:
	${DVIPS} -q $< -o

.tex.idx:
	${TOUCH} $*.gls $*.ind 
	${RUNLATEX} $(LATEXFLAGS) $<

.idx.ind:
	${MAKEINDEX} -q $<
	${RM_F} $(RMFILE)

.tex.glo:
	${TOUCH} $*.gls $*.ind 
	${RUNLATEX} $(LATEXFLAGS) $<

.glo.gls:
	${SORT} $< > $@
	${RM_F} $(RMFILE)

.tex.toc:
	${TOUCH} $*.gls $*.ind 
	${RUNLATEX} $(LATEXFLAGS) $<
	${RM_F} $(RMFILE)


whitepaper.dvi:	whitepaper.cls
