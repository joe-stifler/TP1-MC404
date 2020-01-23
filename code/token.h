#ifndef TOKEN_H
#define TOKEN_H

/**
 *  Instrucao: Todas as possíveis instruções
 *  Diretiva: Diretivas como ".org"
 *  DefRotulo: Tokens de definiçao de rótulos, ex.: "label:"
 *  Hexadecimal, Decimal: São as mesmas definições da seção de Diretivas
 *  Nome: São as palavras dos símbolos e rótulos.
 **/
typedef enum TipoDoToken { Instrucao, Diretiva, DefRotulo, Hexadecimal, Decimal, Nome } TipoDoToken;

typedef struct Token {
    TipoDoToken tipo;
    char* palavra;
    unsigned linha;
} Token;

unsigned adicionarToken(Token novoToken);
void removerToken(unsigned pos);
Token recuperaToken(unsigned pos);
void imprimeListaTokens();
unsigned getNumberOfTokens();

#endif
