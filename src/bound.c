/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Implementa bound.h */

#include <stdlib.h>
#include <stdio.h>
#include <gmp.h>

#include "bound.h"
#include "fifo.h" /* para bit_t */

bound_t* init_bound (void)
{
  bound_t* b = (bound_t*) malloc (sizeof(bound_t));
  mpz_init_set_ui(b->lower,0);
  mpz_init_set_ui(b->upper,1);
  b->length = 1;
  return b;
}

bound_t* set_bound (bound_t* b, const mpz_t lower, const mpz_t upper,
		    const size_t length)
{
  mpz_set(b->lower, lower);
  mpz_set(b->upper, upper);
  b->length = length;
  return b;
}

void free_bound (bound_t* b)
{
  mpz_clear(b->lower);
  mpz_clear(b->upper);
  free(b);
}

bound_t* adjust_bound (bound_t* init, const bound_t* limit, mpz_t total)
{
  mpz_t lw_buf, up_buf; /* buffer temporario lw=lower, up=upper */

  mpz_init_set(lw_buf,init->upper); /* lw_buf = u */
  mpz_sub(lw_buf, lw_buf, init->lower); /* lw_buf = u-l */
  mpz_add_ui(lw_buf, lw_buf, 1L); /* lw_buf = u-l+1 */

  mpz_init_set(up_buf,lw_buf); /* up_buf = lw_buf */

  mpz_mul(lw_buf, lw_buf, limit->lower); /*lw_buf*=lower_cum_counter*/
  mpz_mul(up_buf, up_buf, limit->upper); /*up_buf*=upper_cum_counter*/ 
  mpz_tdiv_q(lw_buf, lw_buf, total); /*lw_buf=floor(lw_buf/total)*/
  mpz_tdiv_q(up_buf, up_buf, total); /*up_buf=floor(up_buf/total)*/
  mpz_sub_ui(up_buf, up_buf, 1L); /*up_buf-=1*/
    
  mpz_add(init->upper, init->lower, up_buf);
  mpz_add(init->lower, init->lower, lw_buf);

  mpz_clear(up_buf);
  mpz_clear(lw_buf);

  return init;
}

int bound_msb_match (const bound_t* b)
{
  return mpz_tstbit(b->upper,b->length-1) ==
    mpz_tstbit(b->lower,b->length-1);
}

int bound_E3_holds (const bound_t* b)
{
  if ( (mpz_tstbit(b->lower,b->length-2) == 1) &&
       (mpz_tstbit(b->upper,b->length-2) == 0) ) return 1;
  return 0;
}
 
bit_t bound_next_msb (bound_t* b)
{
  bit_t retval; /* espaco temporario para o resultado */
  retval = mpz_tstbit(b->lower,b->length-1);
  mpz_clrbit(b->lower,b->length-1);
  mpz_clrbit(b->upper,b->length-1);
  mpz_mul_2exp(b->lower,b->lower,1); /* shift */
  mpz_mul_2exp(b->upper,b->upper,1); /* shift */
  mpz_setbit(b->upper,0);
  return retval;
}

void bound_apply_E3 (bound_t* b)
{
  /* ja sabe-se que o MSB de lower e 1 e o de upper e zero! */
  mpz_clrbit(b->lower,b->length-1);
  mpz_setbit(b->upper,b->length-1);
}

void bound_print (const bound_t* b, FILE* fp)
{
  register size_t i; /* iterador */
  fprintf (fp, "lower: ");
  mpz_out_str(fp, 10, b->lower);
  fprintf (fp, " (");
  for (i=mpz_sizeinbase(b->lower,2); i<b->length; ++i) 
    fprintf(fp,"%c",'0');
  mpz_out_str(fp, 2, b->lower);
  fprintf (fp, ")\n");
  fprintf (fp, "upper: ");
  mpz_out_str(fp, 10, b->upper);
  fprintf (fp, " (");
  for (i = mpz_sizeinbase(b->upper,2); i < b->length; ++i) 
    fprintf(fp,"%c",'0');
  mpz_out_str(fp, 2, b->upper);
  fprintf (fp, ")\n");
}    
