# Hello emacs, this is -*- makefile -*-
# $Id$
# Andre Rabello <Andre.Rabello@ufrj.br>

# Este makefile constroi a documentacao da biblioteca nos formatos ps,
# html e info para o usuario. Nao tente construir a documentacao
# manualmente, ja que algumas otimizacoes devem ser feitas antes de
# mais nada, conversao de caracteres acentuados, etc.

%.dvi: %.texinfo
	@./xtexi2dvi $<

%_toc.html: %.texinfo
	texi2html -split_chapter $<
	@ln -sf $@ index.html

%.info: %.texinfo
	makeinfo $<

%.ps: %.dvi
	dvips -t a4 $< -o

all: print html info

print: arith.ps

html: arith_toc.html

info: arith.info

clean:
	rm -f *.html *.ps *.dvi *.info *~