/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Implementa cdf.h */

#define _GNU_SOURCE /* para getline() */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#include "cdf.h"

/* Esta funcao e interna ao modulo. Ela procura a posicao de um
   simbolo no alfabeto do contador dado, mas faz isto de forma
   eficiente, usando bsearch(), da biblioteca padrao. */
symbol_t* bsearch_symbol (const cdf_t* c, const unsigned long x);

/* Esta funcao e interna ao modulo. Ela procura a posicao de um
   simbolo no alfabeto do contador dado, mas faz isto de forma
   eficiente, procurando de traz para frente (segundo a contagem de
   cada simbolo). [DEFAULT] */
symbol_t* search_symbol (const cdf_t* c, const unsigned long x);

#ifdef CDF_LINEAR_SEARCH
symbol_t* (*_cdf_search) (const cdf_t*, const unsigned long) = &search_symbol;
#elif defined CDF_BINARY_SEARCH
symbol_t* (*_cdf_search) (const cdf_t*, const unsigned long) = &bsearch_symbol;
#else
#error "[CDF_MACRO] Defina o tipo de procura!"
#endif

/* Esta funcao retorna a primeira maior contagem no dicionario
   dado. */
symbol_t* greatest_count (const cdf_t* c);

/* Uma funcao interna para comparar caracteres de um simbolo. Ela
   retorna >0 se a > b, ==0 se a == b e <0 se a < b. */
int compare_symbol (const void* a, const void* b); 

/* Retorna o ponteiro para um simbolo que eh o proximo simbolo com a
   menor contagem depois do simbolo atual, no alfabeto considerado. */
symbol_t* just_bigger (cdf_t* c, const symbol_t* s);

/* Re-organiza o dicionario baseado nas frequencias atuais para seus
   simbolos. Normalmente, voce chamara esta funcao com `start' usando
   o &(cdf_t.alphabet[0]). Porem, se este nao for o caso, escolha a
   patir de qual simbolo comecar a re-organizacao usando este
   parametro. */
void cdf_sort_counts(cdf_t* c, symbol_t* start);

/* Esta funcao refaz as somas acumuladas para as contagens de simbolos
   no dicionario. Na inicializacao de um novo dicionario, esta funcao,
   alem de efetuar as somas, tambem faz a conexao dos simbolos na
   ordem em que aparecem no dicionario. */
void cdf_sort_appear(cdf_t* c, symbol_t* start);

/* Uma vez que a ordenacao foi feita, a re-ordenacao das contagens
   acumuladas e feita de forma mais eficiente chamando esta funcao. */
void cdf_resort_counts(cdf_t* c, symbol_t* change); 

/* O equivalente da funcao acima, para simbolos organizados na ordem
   em que aparecem no dicionario. */
void cdf_resort_appear(cdf_t* c, symbol_t* change);

/* Esta funcao divide por dois (shift right) cada contagem. Esta funcao
   e chamada a cada vez que o numero de bits necessario para representar
   as contagens na codificacao aritmetica e alterado. */
void cdf_shift(cdf_t* c);


/* Elimina uma linha inteira da stream padrao `fp'. */
void cdf_parse_comment(FILE* fp);

/* Faz o `parsing' da string que representa a contagem para um
   simbolo. Esta string podera ter qualquer numero de digitos e
   somente esta limitada pela memoria de seu computador. Esta funcao
   colocara em `buf' o conteudo desta contagem (no formato de uma
   string). Checagens de tamanho sao realizadas automaticamente. */
char* cdf_parse_count(FILE* fp);

/* Checa a existencia de um simbolo em um conjunto de simbolos
   apontado por `alphabet', com tamanho dado por `size'. Retorna 1
   caso o simbolo seja achado no alfabeto e 0 caso contrario. */
int cdf_check_symbol(const unsigned long c, const unsigned long* alphabet, 
		     const size_t size);

#ifdef CDF_LOWER_PROB_FIRST
void (*_cdf_sort) (cdf_t*, symbol_t*) = &cdf_sort_counts;
void (*_cdf_resort) (cdf_t*, symbol_t*) = &cdf_resort_counts;
#elif defined CDF_AS_APPEAR
void (*_cdf_sort) (cdf_t*, symbol_t*) = &cdf_sort_appear;
void (*_cdf_resort) (cdf_t*, symbol_t*) = &cdf_resort_appear;
#else
#error "[CDF_MACRO] Defina o tipo de ordenacao!"
#endif

int compare_symbol (const void* a, const void* b)
{
  const symbol_t* sa = (const symbol_t*) a;
  const symbol_t* sb = (const symbol_t*) b;
  return (sa->c > sb->c) - (sa->c < sb->c);
}

symbol_t* greatest_count (const cdf_t* c)
{
  register size_t i; /* iterador */
  symbol_t* s; /* ajudante */
  s = &(c->alphabet[0]);

  for (i=1; i<=c->size; ++i)
    if ( mpz_cmp (c->alphabet[i].count, s->count) > 0 )
      s = &(c->alphabet[i]);

  return s;
}

symbol_t* just_bigger (cdf_t* c, const symbol_t* s)
{
  register size_t i; /* iterador */
  symbol_t* jb; /* o inteiro maior que o atual, mas menor que todos os
		   outros */

  if (c->last == NULL) c->last = greatest_count(c);
  jb = c->last;
  for (i=1; i<=c->size; ++i) {
    if (mpz_cmp(c->alphabet[i].count,s->count) >= 0 &&
	mpz_cmp(c->alphabet[i].count,jb->count) <= 0) {
      if (c->alphabet[i].next == NULL && 
	  compare_symbol(s,&(c->alphabet[i])) != 0) 
	jb = &(c->alphabet[i]);
    }
  }
  if ( compare_symbol(s,c->last) == 0 ) return NULL;
  else return jb;
}

void cdf_sort_counts(cdf_t* c, symbol_t* start)
{
  symbol_t* s; /* buffer */
  symbol_t* t; /* buffer */
  symbol_t* last = NULL; /* outro buffer */

  /* ordena */
  s = start;
  while ( s->next != NULL) { /* reset */
    t = s->next;
    s->next = NULL;
    s = t;
  }
  s = start;
  while ( s != NULL) {
    s->next = just_bigger(c,s);
    if ( s->next != NULL ) {
      (s->next)->prev = s; 
      mpz_add((s->next)->acc, (s->next)->count, s->acc); /*acumula*/
    }
    last = s;
    s = s->next;
  }
  c->alphabet[0].prev = NULL;

  /* marcador */
  c->first = c->alphabet[0].next;
  c->last = last;
}

void cdf_sort_appear(cdf_t* c, symbol_t* start)
{
  symbol_t* s;

  /* se a organizacao nao estiver feita, faca! */
  if (c->alphabet[0].next == NULL && 
      c->alphabet[c->size].prev == NULL) {
    register size_t i; /* iterador */

    for (i=0; i<c->size; ++i) { /* organiza do simbolo 0 em diante */
      c->alphabet[i].next = &(c->alphabet[i+1]);
      c->alphabet[i+1].prev = &(c->alphabet[i]);
    }
    c->alphabet[0].prev = NULL;
    c->alphabet[c->size].next = NULL;
    c->last = &(c->alphabet[c->size]);
    c->first = c->alphabet[0].next;
  }

  /* realiza a soma */
  s = start;
  while ( s->next != NULL ) {
    mpz_add((s->next)->acc, (s->next)->count, s->acc); /*acumula*/
    s = s->next;
  }
}

void cdf_resort_counts(cdf_t* c, symbol_t* change)
{
  size_t length;
  int lower, higher; /* ajudantes */
  symbol_t* mark; /* marcador de posicao para inicio da re-ordenacao */

  lower = mpz_cmp((change->prev)->count,change->count);
  if (change->next != NULL)
    higher = mpz_cmp((change->next)->count,change->count);
  else higher = 0;

  /* o que e mais obvio, i.e., manter as posicoes */
  if ( lower <= 0 && higher >= 0) {
    /* atualiza a partir deste simbolo */
    while ( change != NULL ) {
      mpz_add(change->acc,change->count,(change->prev)->acc);
      change = change->next;
    }
    return;
  }

  /* tenho que trocar posicoes */
  mark = change;
  if ( lower > 0 )
    while (mpz_cmp((mark->prev)->count,change->count) >= 0)
      mark = mark->prev;
  mark = mark->prev; /* comeca do simbolo anterior */
  if ( higher < 0 ) /* mudo o ponteiro de last para outro simbolo */
    c->last = greatest_count(c);
  cdf_sort_counts(c,mark);

  /* acha o tamanho em bits, dos limites que serao usados na
     codificacao aritmetica. Normaliza o dicionario se for necessario. */
  length = cdf_bound_length(c);
  while ( length > c->bound_length ) {
#ifdef DEBUGGING
    fprintf(stderr, "[cdf::resort_count] Normalizando contagens...");
#endif
    cdf_shift(c);
    cdf_sort_counts(c,&(c->alphabet[0])); /* refaz contagem */
#ifdef DEBUGGING
    fprintf(stderr, "feito!\n");
#endif
    --length;
  }
}

void cdf_resort_appear(cdf_t* c, symbol_t* change)
{
  size_t length;

  cdf_sort_appear(c,change);

  /* acha o tamanho em bits, dos limites que serao usados na
     codificacao aritmetica. Normaliza o dicionario se for necessario. */
  length = cdf_bound_length(c);
  while ( length > c->bound_length ) {
#ifdef DEBUGGING
    fprintf(stderr, "[cdf::resort_appear] Normalizando contagens...");
#endif
    cdf_shift(c);
    cdf_sort_appear(c,&(c->alphabet[0])); /* refaz contagem */
#ifdef DEBUGGING
    fprintf(stderr, "feito!\n");
#endif
    --length;
  }
}

void cdf_shift(cdf_t* c)
{
  register size_t i; /* iterador */

  for (i=1; i<=c->size; ++i)
    if (mpz_cmp_ui(c->alphabet[i].count,1) > 0)
      mpz_fdiv_q_2exp(c->alphabet[i].count,c->alphabet[i].count,1);
}

cdf_t* init_cdf (const unsigned long* alphabet, char** init_count,
		 const size_t size)
{
  register size_t i; /* iterador */

  cdf_t* c = (cdf_t*) malloc (sizeof(cdf_t));
  /* aloca uma posicao a mais para o limite inferior do primeiro
     simbolo (i.e., 0) e outra para o caracter terminador */
  c->alphabet = (symbol_t*) malloc ((size+2)*sizeof(symbol_t));
  c->size = size;

  /* limite inferior */
  mpz_init_set_ui(c->alphabet[0].count,0);
  mpz_init_set_ui(c->alphabet[0].acc,0);
  c->alphabet[0].c = 0;
  c->alphabet[0].next = NULL;
  c->alphabet[0].prev = NULL;
  
  /* inicia todos os simbolos */
  for (i=1; i<=size; ++i) {
    c->alphabet[i].c = alphabet[i-1];
    mpz_init(c->alphabet[i].acc);
    mpz_init_set_str(c->alphabet[i].count, init_count[i-1], 10);
    c->alphabet[i].next = NULL;
    c->alphabet[i].prev = NULL;
  }

  /* ordena lexicograficamente */
  qsort(c->alphabet, c->size+1, sizeof(symbol_t), compare_symbol);

  /* aloca o ultimo simbolo como sendo o caracter terminador. Este
     simbolo e escolhido como o primeiro disponivel. */
  c->alphabet[size+1].c = c->alphabet[size].c + 1;
  mpz_init(c->alphabet[size+1].acc);
  mpz_init_set_ui(c->alphabet[size+1].count,1);
  c->alphabet[size+1].next = NULL;
  c->alphabet[size+1].prev = NULL;
  ++(c->size);

  /* encontra os `proximos' simbolos na ordem de frequencia. */
  c->last = NULL;
  c->first = NULL;
  _cdf_sort(c, &(c->alphabet[0]));

  /* acha o tamanho em bits, dos limites que serao usados na
     codificacao aritmetica */
  c->bound_length = cdf_bound_length(c);
#ifdef DEBUGGING
  fprintf(stderr, "[cdf::init] Operandos terao %u bits\n",
	  c->bound_length);
#endif

  /* define o simbolo terminador */
  c->terminator = &(c->alphabet[size+1]);

  return c;
}

void free_cdf (cdf_t* c)
{
  register size_t i; /* iterador */

  /* libera os contadores */
  for (i=0; i<=c->size; ++i) {
    mpz_clear(c->alphabet[i].acc);
    mpz_clear(c->alphabet[i].count);
  }

  /* libera o alfabeto */
  free(c->alphabet);

  /* Destroi o ponteiro */
  free (c);
}

symbol_t* bsearch_symbol (const cdf_t* c, const unsigned long x)
{
  symbol_t s;
  symbol_t* sp;

  s.c = x; /* inicializa somente o caracter */

  /* procura um simbolo no dicionario */
  sp = (symbol_t*) bsearch(&s, &c->alphabet[1], c->size,
			   sizeof(symbol_t), compare_symbol);

  if ( sp == NULL ) {
    fprintf(stderr, "[cdf::bsearch_symbol] nao posso achar o simbolo");
    fprintf(stderr, " %lu\n", x);
    exit(1);
  }
  return sp;
}

symbol_t* search_symbol (const cdf_t* c, const unsigned long x)
{
  symbol_t* s = c->last;
  while ( s != NULL ) {
    if ( s->c == x ) break;
    s = s->prev;
  }
  if ( s == NULL ) {
    fprintf(stderr, "[cdf::search_symbol] nao posso achar o simbolo");
    fprintf(stderr, " %lu\n", x);
    exit(1);
  }

  return s;
}

void cdf_replace_count (cdf_t* c, const unsigned long x, mpz_t count)
{
  mpz_t diff; /* quanto atualizar */
  symbol_t* s;

  if ( mpz_cmp_ui(count, 0) <= 0 ) {
    fprintf(stderr, "[cdf::replace_count] nao posso alterar uma ");
    fprintf(stderr, " contagem para valor <= 0!\n");
    exit(1);
  }

  s = _cdf_search(c,x); /* procura simbolo  */

  /* acha a diferenca na atualizacao */
  mpz_init(diff);
  mpz_sub(diff, count, s->count); /* o que quero atualizar */
  cdf_update_count(c, x, diff);
  
  /* termina */
  mpz_clear(diff);
}

void cdf_update_count (cdf_t* c, const unsigned long x, mpz_t add)
{
  symbol_t* s;

  s = _cdf_search(c,x); /* procura simbolo  */

  mpz_add(s->count, s->count, add);

  /* checa erros */
  if ( mpz_cmp_ui(s->count, 0) <= 0 ) {
    fprintf(stderr, "[cdf::update_count] nao posso alterar uma ");
    fprintf(stderr, " contagem para valor <= 0!\n");
    exit(1);
  }

  /* re-ordena e atualiza as contagens acumuladas */
  _cdf_resort(c,s);
}

bound_t* cdf_bounds (bound_t* b, const cdf_t* c, const unsigned long x)
{
  symbol_t* s = _cdf_search(c,x); /* procura simbolo */
  set_bound(b, (s->prev)->acc, s->acc, c->bound_length);
  return b;
}

mpz_t* cdf_total_count (const cdf_t* c)
{
  return &((c->last)->acc);
}

size_t cdf_bound_length (const cdf_t* c)
{
  register size_t i; /* iterador */
  mpz_t buf; /* a temporary buffer */
  mpz_t min; /* other buffer */
  const size_t MAXBITS = ULONG_MAX;

  mpz_init_set(min, *cdf_total_count(c)); 
  mpz_mul_ui(min, min, 4);
  mpz_init(buf);

  for (i=1; i<MAXBITS; ++i) {
    mpz_ui_pow_ui(buf,2L,i);
    if ( mpz_cmp(buf, min) >= 0 ) { /* achei o valor */
      mpz_clear(buf);
      mpz_clear(min);
      return i;
    }
  }

  fprintf(stderr, "[cdf::min_ulimit] Nao posso achar um numero de");
  fprintf(stderr, " bits que representem => ");
  mpz_out_str(stderr, 2, min);
  mpz_clear(buf);
  mpz_clear(min);
  fprintf(stderr, "\n");
  fprintf(stderr, "[cdf::min_ulimit] Procurei de 1 a %u bits\n",
	  MAXBITS);
  fprintf(stderr, "[cdf::min_ulimit] Considere a re-implementacao");
  fprintf(stderr, " desta funcao...\n");
  exit(1);
}

void cdf_min_ulimit (const cdf_t* c, bound_t* limit)
{
  mpz_ui_pow_ui(limit->upper, 2, c->bound_length);
  mpz_sub_ui(limit->upper, limit->upper, 1);
  limit->length = c->bound_length;
}

unsigned long cdf_find_tag (const cdf_t* c, bound_t* b, mpz_t tag)
{
  mpz_t buf; /* o numerador */
  mpz_t den; /* o denominador */
  bound_t* new; /* os novos limites */
  symbol_t* s;

  /* calcula o lado esquerdo da comparacao */
  /* numerador */
  mpz_init_set(buf,tag);
  mpz_sub(buf,buf,b->lower);
  mpz_add_ui(buf,buf,1);
  mpz_mul(buf,buf,*cdf_total_count(c));
  mpz_sub_ui(buf,buf,1);
  /* denominador */
  mpz_init_set(den,b->upper);
  mpz_sub(den,den,b->lower);
  mpz_add_ui(den,den,1);
  /* conta final (trunca na direcao de zero) */
  mpz_tdiv_q(buf,buf,den);
  mpz_clear(den); /* nao vou mais precisar */

  s = c->last; /* comeca pelo fim (mais provavel) */
  while ( s != NULL ) {
    if (mpz_cmp(buf,s->acc) >= 0) break; /* achei o simbolo */
    s = s->prev;
  }
  mpz_clear(buf); /* nao vou mais precisar disto */

  /* encontra os novos limites e altera os antigos */
  new = init_bound();
  new = cdf_bounds(new,c,(s->next)->c);
  b = adjust_bound(b,new,*cdf_total_count(c));
  free_bound(new);

  /* finaliza */
  return (s->next)->c;
}

/* Imprime o dicionario na stream padrao `fp'. */
void cdf_print(const cdf_t* c, FILE* fp)
{
  register size_t i; /* um iterador */

  for (i=1; i<=c->size; ++i) { /* para todos os simbolos */
    fprintf(fp, "%lu ", c->alphabet[i].c);
    mpz_out_str(fp, 10, c->alphabet[i].count);
    fprintf(fp, "\n");
  }
}

void cdf_print_order(const cdf_t* c, FILE* fp)
{
  symbol_t* s; /* um buffer */

  s = c->first;
  fprintf(stderr, "ordem direta: [COMECO] -> ");
  while ( s != NULL ) {
    fprintf (stderr, "%lu(", s->c);
    mpz_out_str(stderr, 10, s->acc);
    fprintf (stderr, ") -> ");
    s = s->next;
  }
  fprintf(stderr, "[FIM]\n");

  s = c->last;
  fprintf(stderr, "ordem inversa: [FIM] -> ");
  while ( s->prev != NULL ) {
    fprintf (stderr, "%lu(", s->c);
    mpz_out_str(stderr, 10, s->acc);
    fprintf (stderr, ") -> ");
    s = s->prev;
  }
  fprintf(stderr, "[COMECO]\n");
}

void cdf_save (const cdf_t* c, const char* filename)
{
  FILE* fp;
  /* abre o arquivo */
  if ( (fp=fopen(filename,"w")) == NULL ) { /* problemas */
    perror("sem permissao para escrita?");
    exit(1);
  }
  cdf_print(c, fp);
  fclose(fp);
}

cdf_t* cdf_load (const char* filename)
{
  FILE* fp;
  char c; /* um buffer */
  unsigned long d; /* outro buffer */
  unsigned long* alphabet; /* um lugar temporario para o alfabeto */
  char** count; /* um lugar temporario para a contagem */
  size_t size; /* o numero de simbolos lidos ate agora */
  size_t nlines; /* o numero de linhas lidas ate agora */
  cdf_t* retval;

  /* abre o arquivo */
  if ( (fp=fopen(filename,"r")) == NULL ) { /* problemas */
    perror("sem permissao para leitura?");
    exit(1);
  }

  /* le todos os simbolos declarados */
  size = 0;
  nlines = 1;
  alphabet = NULL;
  count = NULL;
  while ( (c = fgetc(fp)) != EOF ) {
    if ( c == '#' ) { 
      cdf_parse_comment(fp);
      ++nlines;
      continue;
    }
    if ( c == '\n' ) {
      ++nlines;
      continue;
    }
    if isspace(c) continue;
    /* se nao e espaco ou `#', entao devolve e le o simbolo */
    ungetc(c,fp);
    fscanf(fp,"%lu",&d);
    if ( cdf_check_symbol(d,alphabet,size) ) {
      fprintf(stderr, "[cdf::load] O simbolo %lu ja existe\n",d);
      fprintf(stderr, "[cdf::load] Erro na linha %u\n", nlines);
      exit(1);
    }

    while ( isspace(c=fgetc(fp)) ) continue;
    if ( ungetc(c,fp) == EOF ) { /* repoe o ultimo caracter nao-espaco */
      fprintf(stderr, "[cdf::load] Nao posso repor caracter %c\n",c);
      exit(1);
    }
    ++size;
    alphabet = (unsigned long*)realloc(alphabet,size*sizeof(unsigned long));
    count = (char**)realloc(count,size*sizeof(char*));
    alphabet[size-1] = d; /* coloca o novo caracter no alfabeto */
    if ( (count[size-1] = cdf_parse_count(fp)) != NULL ) ++nlines;
    else {
      fprintf(stderr, "[cdf::load] Erro lendo contagem do simbolo");
      fprintf(stderr, " %lu na linha %u\n", d, nlines);
      exit(-1);
    }
#ifdef DEBUGGING
    fprintf(stderr, "%lu: %s\n", d, count[size-1]); /* depura */
#endif
  }
  fprintf(stderr, "[cdf::load] %u simbolos lidos em %u linhas\n", 
	  size, nlines);
  retval = init_cdf(alphabet, count, size);

  /* libera espaco usado */
  free(alphabet);
  while ( size != 0 ) free(count[--size]);
  free (count);
  fclose(fp);

  /* retorna */
  return retval;
}

unsigned long cdf_terminator(const cdf_t* c)
{
  return (c->terminator)->c;
}

void cdf_parse_comment(FILE* fp)
{
  char* line = NULL;
  size_t line_size = 0;
  getline(&line, &line_size, fp);
  free(line);
}  

char* cdf_parse_count(FILE* fp)
{
  char* count = NULL;
  size_t count_size = 0;
  if ( getline(&count, &count_size, fp) != -1 ) {
    if ( count_size <= 2 ) { /* somente \n\0! */
      fprintf(stderr, "[cdf::parse_count] Nao achei contagem\n");
      return NULL;
    }
    count[strlen(count)-1] = '\0'; /* elimina '\n' */
  }
  return count;
}

int cdf_check_symbol(const unsigned long c, 
		     const unsigned long* alphabet, const size_t size)
{
  register size_t i; /* iterador */
  for (i=0; i<size; ++i)
    if ( alphabet[i] == c ) return 1;
  return 0;
}
