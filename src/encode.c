/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Implementa arith_encode() de arith.h */

#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <gmp.h>

#include "arith.h"
#include "cdf.h"
#include "bound.h"
#include "fifo.h"

void arith_encode (const cdf_t* c, fifo_t* f, bound_t* l, 
		   const unsigned long* cp)
{
  static size_t scale3=0; /* numero de mapeamentos E3 feitos ate agora */
  bound_t* curr; /* limites para o simbolo atual */
  int msb_match=0; /* se os MSB dos limites sao iguais */
  int E3_holds=0; /* se a condicao de mapeamento E3 prevalece */

  if (cp == NULL) { /* testa o que tenho a codificar */
    fprintf (stderr, "[arith::encode] Nada para codificar! Saindo...\n");
    exit(1);
  }

  /* pega os proximos limites e ajusta limites atuais */
  curr = init_bound();
  curr = cdf_bounds(curr, c, *cp);
#ifdef DEBUGGING
  fprintf(stderr, "******** START ********\n");
  bound_print(l,stderr);
#endif
  adjust_bound(l, curr, *cdf_total_count(c));
#ifdef DEBUGGING
  bound_print(l,stderr);
  fprintf(stderr, "FIFO: ");
  fifo_print(f,stderr);
#endif
  free_bound(curr);
  
  /* entra no loop de analise e transmissao */
  while( (msb_match =bound_msb_match(l)) ||
	 (E3_holds = bound_E3_holds(l)) ) {
    bit_t tmp = bound_next_msb(l); /* auto-shifting */
    if (msb_match) {
#ifdef DEBUGGING
      fprintf(stderr, " >> MSB shift\n");
#endif
      fifo_write(f, tmp);
      while (scale3 > 0) {
#ifdef DEBUGGING
	fprintf(stderr, " >> Scale3>0\n");
#endif
	fifo_write(f,!tmp);
	--scale3;
      }
    }
    if (E3_holds) {
#ifdef DEBUGGING
      fprintf(stderr, " >> E3 holds\n");
#endif
      bound_apply_E3(l);
      ++scale3;
    }
  }

  /* Final de sequencia */
  if ( *cp == cdf_terminator(c)) {
    /* Toma limite inferior e o envia, respeitando `scale3'. */
    char tmp;
    register int i;
    for (i=0; i<l->length; ++i) {
      tmp = mpz_tstbit(l->lower,l->length-1-i);
      fifo_write(f,tmp);
      while (scale3 > 0) {
	fifo_write(f, !tmp); /* nega bit */
	--scale3;
      }
    }
  }
#ifdef DEBUGGING
  bound_print(l,stderr);
  fprintf(stderr, "FIFO: ");
  fifo_print(f,stderr);
#endif
}

