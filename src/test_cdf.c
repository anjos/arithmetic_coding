/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Este programa implementa um teste simples nas rotinas do modulo CDF
   (cdf.h). Este modulo representa uma funcao de distribuicao
   cumulativa e facilidades para a manipulacao desta classe.. */

#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <mcheck.h>
#include <time.h>

#include "cdf.h" /* para symbol_t e cdf_t */
#include "bound.h" /* para bound_t */

int main (int argc, char** argv)
{
  register size_t i; /* iterador */
  cdf_t* dict;   /* o dicionario com simbolos */
  mpz_t buf; /* outro buffer */
  clock_t start, end; /* cronometro */
  double cpu_time_used;
  char msg[] = "32312311123123331231111232312313123123123123123123";
  const size_t msg_size = 50;
  size_t mi; /* indice da mensagem */
  bound_t* b;

#ifdef DEBUGGING
  mtrace();
#endif

  mpz_init(buf);
  b = init_bound();

  if (argc != 2) {
    fprintf(stderr, "uso: %s <dicionario>\n",argv[0]);
    exit(1);
  }
  
  /* carrega o dicionario com simbolos e o imprime */
  dict = cdf_load(argv[1]);
  fprintf(stderr, "[test_cdf] dicionario carregado\n");
  fprintf(stderr, "[test_cdf] organizacao dos simbolos:\n");
  cdf_print(dict, stderr);
  cdf_print_order(dict, stderr);

  /* atualiza algumas entradas */
  fprintf(stderr, "[test_cdf] Testando re-organizacao...\n");
  mpz_set_ui(buf, 40);
  cdf_replace_count(dict, 1, buf);
  mpz_set_ui(buf, 45);
  cdf_replace_count(dict, 2, buf);
  mpz_set_ui(buf, 5);
  cdf_replace_count(dict, 3, buf);
  cdf_print(dict, stderr);
  cdf_print_order(dict, stderr);
  fprintf(stderr, "[test_cdf] feito!\n");

  fprintf(stderr, "[test_cdf] Testando acesso...\n");
  cdf_min_ulimit(dict,b);
  start = clock();
  mi = 0;
  for (i=0; i<10000000; ++i) {
    cdf_bounds(b,dict,msg[mi++]-'0');
    if (mi==msg_size) mi = 0;
  }
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  fprintf(stderr, "[test_cdf] Cada chamada de cdf_bounds() leva %e us\n",
	  cpu_time_used/10);

  free_cdf(dict);
  free_bound(b);
  mpz_clear(buf);
  exit(0);
}

