/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Este modulo define o conceito de limites e operacoes sobre estes
   limites, de forma similar ao resto da biblioteca. */

#ifndef BOUND_H
#define BOUND_H

#include <gmp.h>
#include <stdio.h>
#include "fifo.h" /* para bit_t */

/* A estrutura abaixo define os limites de contagem para um simbolo
   dado. Esta estrutura e usada com a funcao cdf_bounds, para retornar
   ambos os limites simultaneamente. */
typedef struct bound_t {
  mpz_t lower; /* o limite inferior da contagem */
  mpz_t upper; /* o superior */
  size_t length; /* o tamanho dos operandos, em bits */
} bound_t;

/* Para criar, destruir e ajustar limites, use as funcoes abaixo */
bound_t* init_bound (void); /* inicia com lower=0 e upper=1 */
bound_t* set_bound (bound_t* b, const mpz_t lower, const mpz_t upper,
		    const size_t length);
void free_bound (bound_t* b);

/* Ajusta os limites aplicando a formula descrita na pagina 98 de
   "Introduction to Data Compresion" por Khalid Sayood. O parametro
   `init' contem o endereco dos limites a serem ajustados. O parametro
   `limit' contem os limites atuais para o simbolo sendo analisado e
   `total', a contagem total do dicionario de simbolos. */
bound_t* adjust_bound (bound_t* init, const bound_t* limit, mpz_t total);

/* A proxima funcao determina se os bits MSB de cada limite sao
   iguais, retornando 1 caso sejam e 0 caso contrario. */
int bound_msb_match (const bound_t* b);

/* A proxima funcao determina se o segundo MSB do limite inferior eh 1
   e o do limite superior e zero (condicao E3). O enfoque aqui e
   parecido com o da funcao bound_msb_match(). */
int bound_E3_holds (const bound_t* b);

/* Retorna o valor o bit MSB (em um bit_t), e aplica um `shift' a cada
   um dos limites (inferior e superior), colocando um 1 no LSB do
   limite superior e 0 no LSB do limite inferior. Esta funcao deve ser
   usada apos a checagem provida por bound_msb_match(...). */
bit_t bound_next_msb (bound_t* b);

/* Aplica um mapeamento E3 nos limites dados. Esta funcao somente deve
   ser utilizada depois de uma chamada a bound_E3_holds() e
   bound_msb_match() (para fazer um shift-left nos limites), ja que
   nao faz as mesmas verificacoes (por questoes de eficiencia). */
void bound_apply_E3 (bound_t* b);

/* Imprime os limites de `b' na stream padrao `fp' */
void bound_print (const bound_t* b, FILE* fp);

#endif /* BOUND_H */
