/* Hello emacs, this is -*- c -*- */
/* $Id */
/* Andre Rabello <Andre.Rabello@ufrj.br> */

/* Este modulo define o que e um histograma de contagem para o
   algoritmo de codificacao e decodificacao. Este histograma e
   equivalente a funcao de probabilidade cumulativa (CDF) de um 
   alfabeto. Para iniciar um novo alfabeto, use a funcao apropriada.

   As contagens podem ser tao longas o quanto se deseja, ja que
   utilizam a biblioteca de precisao arbitraria `gmp' (info gmp).

   Este modulo tambem descreve funcoes para manipular a contagem do 
   alfabeto criado, facilitando a atualizacao dos codificadores
   aritmeticos. */

#ifndef CDF_H
#define CDF_H

#include <stdlib.h> 
#include <gmp.h>

#include "bound.h"

/* Define o tipo de procura que se deseja realizar. As opcoes sao
   CDF_LINEAR_SEARCH (busca linear comecando pelo ultimo simbolo, que
   tambem e o mais provavel) e CDF_BINARY_SEARCH (busca binaria,
   usando a ordem lexicografica do alfabeto. */
#define CDF_LINEAR_SEARCH
#undef CDF_BINARY_SEARCH

/* Define o tipo de organizacao dos simbolos com relacao a sua
   contagem acumulada. As opcoes sao CDF_LOWER_PROB_FIRST (os simbolos
   menos provaveis estao no inicio do dicionario) e CDF_AS_APPEAR (os
   simbolos sao organizados como aparecem no arquivo de simbolos e
   contagens). */
#undef CDF_AS_APPEAR
#define CDF_LOWER_PROB_FIRST

typedef struct symbol_t {
  unsigned long c; /* o inteiro representativo deste simbolo */
  mpz_t count; /* a contagem para este simbolo */
  mpz_t acc; /* a contagem (acumulada) para este simbolo */
  struct symbol_t* next; /* o proximo simbolo na ordem */
  struct symbol_t* prev; /* o simbolo anterior */
} symbol_t;

typedef struct cdf_t {
  size_t size; /* o tamanho do alfabeto */
  symbol_t* alphabet; /* os simbolos e contagens */
  size_t bound_length; /* o tamanho, em bits dos limites */
  symbol_t* first; /* o primeiro simbolo da contagem */
  symbol_t* last; /* o ultimo simbolo da contagem */
  symbol_t* terminator; /* marca o simbolo final de uma mensagem */
} cdf_t;

/* Inicializa um novo alfabeto, com tamanho dado por `size' e com os
   simbolos descritos em `alphabet'. A contagem inicial de simbolos e
   dada em `init_count', que e do tipo char** (um vetor de tamanho
   `size' com as contagens de cada simbolo do alfabeto dado em
   `alphabet'). Deve-se inicializar a variavel pertinente em seu
   programa e passa-la por referencia aqui. */
cdf_t* init_cdf (const unsigned long* alphabet, char** init_count,
		 const size_t size);

void free_cdf (cdf_t* c);

/* Dado uma distribuicao, e possivel reajustar as probabilidades de
   um simbolo `x', indicando qual simbolo deseja-se ajustar e o novo
   valor da contagem para aquele simbolo. */
void cdf_replace_count (cdf_t* c, const unsigned long x, mpz_t count);

/* Se quiser mudar (adicionar) um valor a contagem de um simbolo, use a
   funcao abaixo. Neste caso, todos os simbolos seguintes deverao ser
   atualizados. */
void cdf_update_count (cdf_t* c, const unsigned long x, mpz_t add);

/* Dado um simbolo `x' e uma contagem `c', e possivel descobrir os
   limites inferiores e superiores da contagem. Estes limites sao
   usados no algoritmo de codificacao e decodificacao aritmetica, e
   retornados no parametro `b', que deve ser previamente inicializado
   com `init_bound()'. */
bound_t* cdf_bounds (bound_t* b, const cdf_t* c, const unsigned long x);

/* A funcao abaixo retorna o valor da contagem total dos
   simbolos. Este valor eh igual ao limite superior do ultimo
   simbolo. Esta funcao, portanto, e tao somente um "wrapper" para
   cdf_upper_bound (c, last_symbol). */
mpz_t* cdf_total_count (const cdf_t* c);

/* Calcula o tamanho minimo, em bits para os limites, que satisfaz o
   criterio de numero minimo de bits para a representacao da parte
   superior dos limites de calculo na codificacao aritmetica. 

   A biblioteca de precisao arbitraria sendo utilizada nao possui uma
   funcao automatica para o calculo de logaritmos na base 2. Por esta
   razao, o procedimento adotado aqui e a busca exaustiva pelo inteiro
   (long) sem sinal, tal que 2 elevado a este inteiro, seja
   imediatamente maior que 4*cdf_total_count(). Por esta razao, o
   sistema tem precisao limitada a 2 elevado ao maior inteiro sem
   sinal possivel. Em um sistema GNU/Linux rodando em um i386 isto
   significa que o expoente maximo sera 4.294.967.295. Para sistemas
   praticos, isto geralmente e suficiente, mas se voce se incomoda com
   esta limitacao, re-projete a implementacao desta funcao para que
   consiga precisao arbitraria de fato. */
size_t cdf_bound_length (const cdf_t* c);

/* Ajusta o limite superior dado para que seja o maximo valor dado o
   tamanho, em bits, deste tipo de operando (o que pode ser obtido com
   cdf_bound_length(). */
void cdf_min_ulimit (const cdf_t* c, bound_t* limit);

/* Encontra o simbolo de um tag qualquer, dado os limites atuais. */
unsigned long cdf_find_tag (const cdf_t* c, bound_t* b, mpz_t tag);

/* Imprime o dicionario na stream padrao `fp'. */
void cdf_print(const cdf_t* c, FILE* fp);

/* Imprime a ordem do dicionario (segundo as contagens) na stream
   padrao `fp'. */
void cdf_print_order(const cdf_t* c, FILE* fp);

/* Esta funcao salva o dicionario em um arquivo, cujo o nome e dado
   como parametro. Ela chama cdf_print() depois de ter aberto o
   arquivo para escrita... */
void cdf_save (const cdf_t* c, const char* filename);

/* Esta funcao carrega o dicionario de um arquivo, cujo o nome e dado
   como parametro. A sintaxe e bastante simples, cada linha deve conter
   um simbolo (de 1 inteiro) seguido de espaco, e um inteiro (positivo)
   de tamanho arbitrario, indicando o numero de ocorrencias para aquele
   simbolo. A rotina automaticamente devera ler e construir o dicionario
   deste arquivo. Ao prefixar uma linha ou o final de uma linha com o
   caracter `#', a linha ou o que resta dela e ignorado, de tal forma
   que este caracter seja usado como inicio de cometario. Veja os
   exemplos em <root>/example/. */
cdf_t* cdf_load (const char* filename);

/* Esta funcao retorna o simbolo (automatico) de terminacao de mensagens */
unsigned long cdf_terminator (const cdf_t* c);

#endif /* CDF_H */
