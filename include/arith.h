/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Este modulo define o codificador e o decodificador
   aritmeticos. Para chamar o codificador aritmetico, e necessario que
   o usuario inicialize uma fila (fifo, em fifo.h) e um dicionario
   contendo as probabilidades acumuladas do alfabeto que se deseja
   codificar (cdf, em cdf.h). O dicionario e a fila sao passados para
   o codificador, que entao podera realizar o processo a que se
   destina. 

   O decodificador, a partir dos mesmos parametros, podera decodificar
   os dados diretamente da fila, como explicitado em "Introduction to
   Data Compression" por Khalid Sayood, capitulo 4 ("Arithmethic
   Coding"). Um parametro adicional sao os limites que se esta usando no
   momento. Estes limites devem ser inicializados pelo usuario usando
   init_bound() e cdf_min_ulimit(), que inicializam as variaveis
   internas ao tipo.

   Espera-se que um simbolo seja (de)codificado por vez, e portanto e
   isto que sera esperado em `cp': apenas um simbolo. Para terminar a
   mensagem na codificacao envie o caracter terminador de `c', apontado
   por `c->terminator'. */

#ifndef ARITH_H 
#define ARITH_H

#include "cdf.h"
#include "fifo.h"

void arith_encode (const cdf_t* c, fifo_t* f, bound_t* l,
		   const unsigned long* cp);
void arith_decode (const cdf_t* c, fifo_t* f, bound_t* l,
		   unsigned long* cp);

#endif /* ARITH_H */
