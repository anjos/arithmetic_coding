/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Este programa implementa um teste simples nas rotinas do
   codificador aritmetico. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mcheck.h>
#include <time.h>

#include "arith.h"
#include "cdf.h"
#include "fifo.h"
#include "bound.h"

int main (int argc, char** argv)
{
  bound_t* limit; /* limites globais */
  cdf_t* dict;   /* o dicionario com simbolos */
  FILE* msg; /* o arquivo com a mensagem codificada */
  FILE* dmsg; /* o arquivo com a mensagem decodificada */
  fifo_t* coded; /* a mensagem codificada */
  unsigned long buff; /* caracter decodificado */
  clock_t start, end; /* cronometro */
  double cpu_time_used;
  unsigned long lsize; /* um contador */

#ifdef DEBUGGING
  mtrace();
#endif

  if (argc != 3) {
    fprintf(stderr, "uso: %s <dicionario> <arquivo-codificada>\n", argv[0]);
    exit(1);
  }

  /* carrega dicionario */
  dict = cdf_load(argv[1]);

  /* inicializa limites */
  limit = init_bound(); /* ajusta limites para 0 e 1 */
  cdf_min_ulimit(dict,limit); /* ajusta limite */

  /* inicializa fifo com a mensagem a ser decodificada */
  coded = init_fifo();
  if ( (msg = fopen(argv[2],"r")) == NULL ) {
    perror("[test_encode] *ERROR* ");
    exit(1);
  }
  fifo_load(coded, msg);
  fclose(msg);
#ifdef DEBUGGING
  fprintf(stderr, "[test_decode] fifo inicializada.\n");
#endif

  /* decodifica mensagem */
  if ( (dmsg = fopen("decoded.mesg","w")) == NULL ) {
    perror("[test_encode] *ERROR* ");
    exit(1);
  }

  lsize = 0;
  start = clock();
  for (;;) /* forever */ {
    arith_decode(dict, coded, limit, &buff);
    if ( buff == cdf_terminator(dict) ) break;
    ++lsize;
    fprintf(dmsg, "%lu\n", buff);
  }
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  fclose(dmsg);
    
  fprintf(stderr, "[test_decode] mensagem decodificada,");
  fprintf(stderr, " %lu simbolos\n", lsize);
  fprintf(stderr, "[test_cdf] Tempo total = %e s\n",cpu_time_used);
  fprintf(stderr, "[test_cdf] Por simbolo ~= %e s\n",cpu_time_used/lsize);

  /* libera memoria alocada */
  free_bound(limit);
  free_cdf(dict);
  free_fifo(coded);

  return 0;
}
