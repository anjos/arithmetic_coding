/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Implementa arith_decode() de arith.h */

#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <gmp.h>

#include "arith.h"
#include "cdf.h"
#include "bound.h"
#include "fifo.h"

/* Esta funcao (privada) aplica o mapeamento E3 ao numero `tag', cujo
   o comprimento (calculado pelas probabilidades) contidas no
   dicionario. */
void tag_apply_E3(mpz_t tag, const size_t length);

void arith_decode (const cdf_t* c, fifo_t* f, bound_t* l,
		   unsigned long* cp)
{
  static mpz_t tag; /* um buffer para o tag atual sendo decodificado */
  static bit_t started=0; /* flag */
  int E3_holds=0; /* se a condicao de mapeamento E3 prevalece */

#ifdef DEBUGGING
  fprintf(stderr, "******** START ********\n");
  fprintf(stderr, "FIFO: ");
  fifo_print(f,stderr);
#endif

  /* Le `length' caracteres da fila fifo de entrada */
  if ( started == 0 ) {
    started = 1;
    if (f->size < l->length) { /* testa o que tenho a decodificar */
      fprintf (stderr, "[arith::decode] Nada para decodificar! Saindo...\n");
      exit(1);
    }
    mpz_init(tag);
    fifo_nread(f, tag, l->length);
  }

#ifdef DEBUGGING
  fprintf(stderr, "tag => ");
  mpz_out_str(stderr, 10, tag);
  fprintf(stderr, " (");
  mpz_out_str(stderr, 2, tag);
  fprintf(stderr, ")\n");
  bound_print(l,stderr);
#endif

  /* acha o simbolo e reajusta limites */
  *cp = cdf_find_tag(c,l,tag);

#ifdef DEBUGGING
  bound_print(l,stderr);
  fprintf(stderr, "caracter => %lu\n", *cp);
#endif

  /* entra no loop de analise */
  while( bound_msb_match(l) ||
	 (E3_holds = bound_E3_holds(l)) ) {
    bound_next_msb(l); /* auto-shifting */
    fifo_shift_in(f,tag,l->length);
#ifdef DEBUGGING
    fprintf(stderr, " >> shift\n");
#endif
    if (E3_holds) {
#ifdef DEBUGGING
      fprintf(stderr, " >> E3 holds\n");
#endif
      bound_apply_E3(l);
      tag_apply_E3(tag,l->length);
    }
  }

#ifdef DEBUGGING
  fprintf(stderr, "FIFO: ");
  fifo_print(f,stderr);
#endif

  if ( *cp == cdf_terminator(c) ) {
    started=0;
    mpz_clear(tag);
  }
}

void tag_apply_E3(mpz_t tag, const size_t length)
{
  if (mpz_tstbit(tag,length-1)) mpz_clrbit(tag,length-1);
  else mpz_setbit(tag,length-1);
}
