/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Implementa fifo.h */

#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <gmp.h>

fifo_t* init_fifo (void)
{
  fifo_t* f = (fifo_t*) malloc (sizeof(fifo_t));
  mpz_init_set_ui(f->data,0);
  f->size = 0;
  return f;
}

void free_fifo (fifo_t* f)
{
  mpz_clear(f->data);
  free(f);
}

void fifo_write (fifo_t* f, const bit_t x)
{
/*    if ( f->size == ULONG_MAX ) { */
/*        fprintf(stderr, "[fifo::write] fifo lotada (max=%lu)\n", */
/*  	      ULONG_MAX); */
/*        exit(1); */
/*    } */
  mpz_mul_2exp(f->data,f->data,1);
  if ( x == 1 ) mpz_setbit(f->data,0);
  ++(f->size);
}

void fifo_write_str (fifo_t* f, const char* s)
{
  register size_t i; /* iterador */
  for (i=0; i<strlen(s); ++i) {
    if ( s[i] == '0' ) fifo_write(f, 0);
    else fifo_write(f, 1);
  }
}

bit_t fifo_read (fifo_t* f)
{
  if (f->size != 0) {
    bit_t retval = mpz_tstbit(f->data,f->size-1);
    mpz_clrbit(f->data,f->size-1);
    --(f->size);
    return retval;
  }
  fprintf(stderr, "[fifo::read] ATENCAO: Tamanho atual e zero!\n");
  return 0;
}

void fifo_nread (fifo_t* f, mpz_t buffer, const size_t n)
{
  register size_t i; /* iterador */
  mpz_set_ui(buffer, 0);
  for (i=0; i<n; ++i)
    if (fifo_read(f)) mpz_setbit(buffer,n-i-1);
}

void fifo_shift_in (fifo_t* f, mpz_t val, const size_t length)
{
  if (f->size == 0) {
    fprintf(stderr, "[fifo::shift_in] A fila acabou!\n");
    exit(1);
  }
  mpz_clrbit(val,length-1);
  mpz_mul_2exp(val,val,1); /* shift left */
  if (fifo_read(f)) mpz_setbit(val,0);
}

void fifo_print (const fifo_t* f, FILE* fp)
{
  register size_t i;
  for (i=mpz_sizeinbase(f->data,2); i<f->size; ++i)
    fprintf(fp, "%c", '0');
  mpz_out_str(fp, 2, f->data);
  fprintf(fp, "\n");
}
  
void fifo_load (fifo_t* f, FILE* fp)
{
  char bchar;
  while ( fscanf(fp, "%c", &bchar) != EOF ) {
    fifo_write(f,(bchar=='0')?0:1);
  }
}
