/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Este modulo implementa uma fila (FIFO) de tamanho arbitrario. A
   fila e formada de inteiros '1' e '0' (definidos por bit_t)
   exclusivamente e nao tem acesso aleatorio. Para ler ou escrever na
   fila, deve-se usar as funcoes basicas descritas abaixo. A fila deve
   ser inicializada e destruida usando-se a funcao pertinente, tambem
   descrita abaixo. Evite mexer internamente na estrutura da fila, ja
   que isto podera corromper os dados de forma irrecuperavel. */

#ifndef FIFO_H
#define FIFO_H

#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>

/* Define uma fifo */
typedef struct fifo_t {
  mpz_t data; /* os dados da fila */
  size_t size; /* o tamanho atual da fila */
} fifo_t;

typedef enum BIT_ENUM {LOW=0, HIGH=1} bit_t;

/* Inicia uma fifo */
fifo_t* init_fifo (void);

/* Destroi uma fifo */
void free_fifo (fifo_t* f);

/* Apendiciona um caracter (1 ou 0) a fifo */
void fifo_write (fifo_t* f, const bit_t x);

/* Apendiciona todos os caracteres de uma string na fifo. Todos os
   elementos da string que sao '0' sao considerados 0, enquanto que os
   restantes, 1. */
void fifo_write_str (fifo_t* f, const char* s);

/* Le o proximo bit disponivel */
bit_t fifo_read (fifo_t* f);

/* Le os proximos `n' bits, inicializando `buffer' com o inteiro
   resultante da concatenacao destes bits. */
void fifo_nread (fifo_t* f, mpz_t buffer, const size_t n);

/* Faz um left-shift em `val' usando o proximo dado da fifo `f' */
void fifo_shift_in (fifo_t* f, mpz_t val, const size_t length);

/* Imprime, na stream padrao dada, o conteudo da fila. */
void fifo_print (const fifo_t* f, FILE* fp);

/* Carrega a fifo da stream padrao dada. Neste caso, a fifo deve ser
   inicializada antes de ser utilizada. */
void fifo_load (fifo_t* f, FILE* fp);

#endif /* FIFO_H */
