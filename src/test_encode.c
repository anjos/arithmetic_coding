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
  register unsigned long i; /* iterador */
  bound_t* limit; /* limites globais */
  cdf_t* dict;   /* o dicionario com simbolos */
  fifo_t* coded; /* a mensagem codificada */
  FILE* msg; /* o arquivo com a mensagem */
  unsigned long* symbol; /* lista com os simbolos p/ codificar */
  unsigned long lsize; /* o tamanho da lista a codificar */
  FILE* cmsg; /* o arquivo com a mensagem codificada */
  unsigned long cs; /* o simbolo atual, lido do arquivo */
  clock_t start, end; /* cronometro */
  double cpu_time_used;

#ifdef DEBUGGING
  mtrace();
#endif

  if (argc != 3) {
    fprintf(stderr, "uso: %s <dicionario> <arquivo-mensagem>\n",
	    argv[0]);
    fprintf(stderr, "<arquivo-mensagem>: um simbolo por linha\n");
    exit(1);
  }

  /* carrega dicionario */
  dict = cdf_load(argv[1]);

  /* inicializa limites */
  limit = init_bound(); /* ajusta limites para 0 e 1 */
  cdf_min_ulimit(dict,limit); /* ajusta limite */

  /* carrega arquivo-mensagem */
  fprintf(stderr, "[test_encode] Lendo arquivo a ser codificado...");
  if ( (msg = fopen(argv[2], "r")) == NULL ) {
    perror("[test_encode] *ERRO* ");
    exit(1);
  }
  symbol = NULL;
  lsize = 0;
  while ( fscanf(msg, "%lu", &cs) != EOF ) {
    symbol = realloc(symbol,(lsize+1)*sizeof(unsigned long));
    symbol[lsize] = cs;
    ++lsize;
  }
  fclose(msg);
  fprintf(stderr, "feito!\n");

  /* codifica mensagem */
  coded = init_fifo();
  fprintf(stderr, "[test_encode] fifo inicializada.\n");

  start = clock();
  /* ---------------------
     CODFICACAO: INICIO 
     --------------------- */
  for (i=0; i<lsize; ++i)
    arith_encode(dict, coded, limit, &symbol[i]);
  /* termina mensagem */
  arith_encode(dict, coded, limit, &((dict->terminator)->c));
  /* ---------------------
     CODFICACAO: FIM 
     --------------------- */
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  fprintf(stderr, "[test_encode] mensagem codificada, ");
  fprintf(stderr, "%lu simbolos.\n", lsize);
  fprintf(stderr, "[test_cdf] Tempo total = %e s\n",cpu_time_used);
  fprintf(stderr, "[test_cdf] Por simbolo ~= %e s\n",cpu_time_used/lsize);

  /* Imprime mensagem na saida padrao */
  if ( (cmsg = fopen("coded.mesg","w")) == NULL ) {
    perror("[test_encode] *ERROR* ");
    exit(1);
  }
  fifo_print(coded, cmsg);
  fclose(cmsg);

  /* libera memoria alocada */
  free_bound(limit);
  free_cdf(dict);
  free_fifo(coded);
  free(msg);

  return 0;
}
