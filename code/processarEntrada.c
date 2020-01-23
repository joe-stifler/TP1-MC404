/**
 * José Ribeiro Neto
 * Turma A
 * RA 176665
 **/
#include "montador.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum Erro { NULO, LEXICO, GRAMATICAL } Erro;

typedef enum Diretivas { SET, ORG, ALIGN, WFILL, WORD } Diretivas;

typedef enum Instrucoes {
        LOAD, LOADM, LOADB,
        LOADmq, LOADmq_mx,
        STOR, JUMP, JMPP,
        ADD, ADDB, SUB,
        SUBB, MUL, DIV,
        LSH, RSH, STORA
} Instrucoes;

/**
 * converte a cadeia de caracteres em tokens
 **/
void preProcessamento(char *entrada, unsigned tamanho) {
    Token token;
    unsigned j, linha = 1;

    tamanho = strlen(entrada);

    for (unsigned i = 0; i < tamanho; i++) {
        /** ignora qualquer 'espaco' ou 'tab' **/
        while (i < tamanho && (entrada[i] == ' ' || entrada[i] == '\t')) i++;

        /** captura a palavra, caso exista **/
        for (j = 0, token.palavra = NULL; i < tamanho && entrada[i] != ' '
                && entrada[i] != '\t' && entrada[i] != '\n' && entrada[i] != '#'; j++, i++) {

            token.palavra = (char *) realloc(token.palavra, sizeof(char) * (j + 1));
            token.palavra[j] = entrada[i];
        }

        /** por fim, armazena a palavra como um novo token **/
        if (j > 0) {
            token.palavra[j] = 0;
            token.linha = linha;

            adicionarToken(token);
        }

        /** ignora todo caractere a direita de um '#' ateh que haja uma quebra de linha **/
        if (i < tamanho && entrada[i] == '#')
            while (i < tamanho && entrada[i] != '\n') i++;

        if (i < tamanho && entrada[i] == '\n') linha++;
    }
}

int isAlpha(char c) {
    return (c >= '0' && c <= '9') ||
                (c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z') || c == '_';
}

Instrucoes convertToInstrucao(char *str) {
    if (!strcmp(str, "LOAD")) return LOAD;
    else if (!strcmp(str, "LOAD-")) return LOADM;
    else if (!strcmp(str, "LOAD|")) return LOADB;
    else if (!strcmp(str, "LOADmq")) return LOADmq;
    else if (!strcmp(str, "LOADmq_mx")) return LOADmq_mx;
    else if (!strcmp(str, "STOR")) return STOR;
    else if (!strcmp(str, "JUMP")) return JUMP;
    else if (!strcmp(str, "JMP+")) return JMPP;
    else if (!strcmp(str, "ADD")) return ADD;
    else if (!strcmp(str, "ADD|")) return ADDB;
    else if (!strcmp(str, "SUB")) return SUB;
    else if (!strcmp(str, "SUB|")) return SUBB;
    else if (!strcmp(str, "MUL")) return MUL;
    else if (!strcmp(str, "DIV")) return DIV;
    else if (!strcmp(str, "LSH")) return LSH;
    else if (!strcmp(str, "RSH")) return RSH;
    else return STORA;
}

Diretivas getDiretiva(char * palavra) {
    if (!strcmp(palavra, ".set")) return SET;
    else if(!strcmp(palavra, ".org")) return ORG;
    else if (!strcmp(palavra, ".align")) return ALIGN;
    else if (!strcmp(palavra, ".wfill")) return WFILL;
    else return WORD;
}

/**
 * se existir erro lexical, retorna LEXICO,
 * do contrario NULO.
 **/
Erro validTokenType(Token *token) {
    int n = strlen(token->palavra), status;

    // rotulo
    if (token->palavra[n-1] == ':') {
        // rotulo comecando com numero eh invalido
        if (token->palavra[0] >= '0' && token->palavra[0] <= '9') return LEXICO;

        // somente caracteres alfanumericos sao validos
        for (int i = 0; i < n - 1; i++) if (!isAlpha(token->palavra[i])) return LEXICO;

        token->tipo = DefRotulo;

        return NULO;
    }

    // diretiva
    if (token->palavra[0] == '.') {
        if (!strcmp(token->palavra, ".set")
                || !strcmp(token->palavra, ".org")
                    || !strcmp(token->palavra, ".align")
                        || !strcmp(token->palavra, ".wfill")
                            || !strcmp(token->palavra, ".word")) {

                token->tipo = Diretiva;

                return NULO;
        } else return LEXICO; // nao eh uma diretiva
    }

    // Instrucao
    if (!strcmp(token->palavra, "LOAD") ||
        !strcmp(token->palavra, "LOAD-") ||
        !strcmp(token->palavra, "LOAD|") ||
        !strcmp(token->palavra, "LOADmq") ||
        !strcmp(token->palavra, "LOADmq_mx") ||
        !strcmp(token->palavra, "STOR") ||
        !strcmp(token->palavra, "JUMP") ||
        !strcmp(token->palavra, "JMP+") ||
        !strcmp(token->palavra, "ADD") ||
        !strcmp(token->palavra, "ADD|") ||
        !strcmp(token->palavra, "SUB") ||
        !strcmp(token->palavra, "SUB|") ||
        !strcmp(token->palavra, "MUL") ||
        !strcmp(token->palavra, "DIV") ||
        !strcmp(token->palavra, "LSH") ||
        !strcmp(token->palavra, "RSH") ||
        !strcmp(token->palavra, "STORA")) {

            token->tipo = Instrucao;

            return NULO;
        }

    // remove as aspas da string
    if (token->palavra[0] == '"' && n > 1) {
        for (int i = 0; i < n-1; i++) token->palavra[i] = token->palavra[i+1];
        n--;

        if (token->palavra[n-1] == '"') {
            token->palavra[n-1] = 0;
            n--;
        }
    }

    status = 1;

    // verifica se eh um numero decimal
    for (int i = 0; i < n; i++)
        if (!((token->palavra[i] >= '0' && token->palavra[i] <= '9') ||
                (i == 0 && token->palavra[i] == '-'))) status = 0;

    if (status == 1) {
        token->tipo = Decimal;

        return NULO;
    }

    // verifica se eh um numero hexadecimal
    if (n == 12 && token->palavra[0] == '0' && token->palavra[1] == 'x') {
        status = 1;

        for (int i = 2; i < n; i++)
            if (!((token->palavra[i] >= '0' && token->palavra[i] <= '9') ||
                        (token->palavra[i] >= 'a' && token->palavra[i] <= 'f') ||
                                (token->palavra[i] >= 'A' && token->palavra[i] <= 'F'))) status = 0;

        if (status == 1) {
            token->tipo = Hexadecimal;

            return NULO;
        }
    }

    // SYM ou ROT

    // palavra comeca com numero
    if (token->palavra[0] >= '0' && token->palavra[0] <= '9') return LEXICO;

    status = 1;
    for (int i = 0; i < n; i++) {
        if (!(isAlpha(token->palavra[i]))) {
            status = 0;
            break;
        }
    }

    if (status == 1) {
        token->tipo = Nome;

        return NULO;
    }

    return LEXICO;
}

/**
 * Verifica se existe algum erro gramatical / lexico no conjunto de
 * tokens gerados pela funcao preProcessamento. Imprime ListaTokens
 * caso nao exista erro.
 **/
int validaEntrada() {
    int linha;
    Erro erro = NULO;
    unsigned tamanho = getNumberOfTokens();
    Token tokens[tamanho + 1];

    /** verifica a ausencia de erros gramaticais e lexicos **/
    for (unsigned i = 0; i < tamanho && erro == NULO; i++) {
        tokens[i] = recuperaToken(0);
        removerToken(0);

        erro = validTokenType(&tokens[i]);
        linha = tokens[i].linha;

        // verifica erro lexico
        if (erro == LEXICO) break;

        switch(tokens[i].tipo) {
            case Instrucao:
                // se eh uma instrucao e o token anterior nao eh
                // um rotulo, mas possui a mesma linha que o token atual,
                // entao erro GRAMATICAL
                if (i >= 1 && tokens[i-1].tipo != DefRotulo
                        && tokens[i-1].linha == tokens[i].linha) {
                        erro = GRAMATICAL;

                        break;
                }

                switch(convertToInstrucao(tokens[i].palavra)) {
                    case LOAD:
                    case LOADM:
                    case LOADB:
                    case LOADmq_mx:
                    case STOR:
                    case JUMP:
                    case JMPP:
                    case ADD:
                    case ADDB:
                    case SUB:
                    case SUBB:
                    case MUL:
                    case DIV:
                    case STORA:
                        // captura o argumento 1
                        if (i + 1 < tamanho) {
                            tokens[++i] = recuperaToken(0);
                            removerToken(0);

                            erro = validTokenType(&tokens[i]);
                            linha = tokens[i].linha;

                            // verifica erro lexico
                            if (erro == LEXICO) break;

                            // verifica erro gramatical
                            if ((tokens[i].tipo != Hexadecimal && tokens[i].tipo != Decimal
                                    && tokens[i].tipo != Nome) || (tokens[i].linha != tokens[i-1].linha)) {
                                erro = GRAMATICAL;

                                break;
                            } else if (tokens[i].tipo == Decimal) {
                                // erro de sintaxe (ultrapassa limites estabelecidos)
                                if ((long) strtol(tokens[i].palavra, NULL, 10) >= (long) 1024 ||
                                            (long) strtol(tokens[i].palavra, NULL, 10) < (long) 0) {
                                    erro = LEXICO;

                                    break;
                                }
                            }

                        } else { // faltam argumentos
                            erro = GRAMATICAL;
                            linha = tokens[i].linha;

                            break;
                        }

                        break;
                    default: break;
                }

                break;
            case Diretiva:
                // se eh uma diretiva e o token anterior nao eh
                // um rotulo mas possui a mesma linha que o token atual,
                // entao erro GRAMATICAL
                if (i >= 1 && tokens[i-1].tipo != DefRotulo
                        && tokens[i-1].linha == tokens[i].linha) {
                        erro = GRAMATICAL;

                        break;
                }

                switch(getDiretiva(tokens[i].palavra)) {
                    case SET:

                        if (i + 2 < tamanho) {
                            // captura o argumento 1
                            tokens[++i] = recuperaToken(0);
                            removerToken(0);

                            erro = validTokenType(&tokens[i]);
                            linha = tokens[i].linha;

                            // verifica erro lexico
                            if (erro == LEXICO) break;

                            // verifica erro gramatical
                            if (tokens[i].tipo != Nome || tokens[i].linha != tokens[i-1].linha) {
                                erro = GRAMATICAL;

                                break;
                            }

                            tokens[++i] = recuperaToken(0);
                            removerToken(0);

                            erro = validTokenType(&tokens[i]);
                            linha = tokens[i].linha;

                            // verifica erro lexico
                            if (erro == LEXICO) break;

                            // verifica erro gramatical
                            if ((tokens[i].tipo != Hexadecimal
                                    && tokens[i].tipo != Decimal) || (tokens[i].linha != tokens[i-2].linha)) {
                                erro = GRAMATICAL;

                                break;
                            } else if (tokens[i].tipo == Decimal) {
                                // erro de sintaxe (ultrapassa limites estabelecidos)
                                if ((long) strtol(tokens[i].palavra, NULL, 10) >= (long) 2147483648 ||
                                            (long) strtol(tokens[i].palavra, NULL, 10) < (long) -2147483648) {
                                    erro = LEXICO;

                                    break;
                                }
                            }

                        } else { // faltam argumentos
                            erro = GRAMATICAL;
                            linha = tokens[i].linha;

                            break;
                        }

                        break;
                    case ORG:

                        if (i + 1 < tamanho) {
                            // captura o argumento 1
                            tokens[++i] = recuperaToken(0);
                            removerToken(0);

                            erro = validTokenType(&tokens[i]);
                            linha = tokens[i].linha;

                            // verifica erro lexico
                            if (erro == LEXICO) break;

                            // verifica erro gramatical
                            if ((tokens[i].tipo != Hexadecimal &&
                                    tokens[i].tipo != Decimal) || tokens[i].linha != tokens[i-1].linha) {
                                erro = GRAMATICAL;

                                break;
                            } else if (tokens[i].tipo == Decimal) {
                                // erro de sintaxe (ultrapassa limites estabelecidos)
                                if ((long) strtol(tokens[i].palavra, NULL, 10) >= (long) 1024 ||
                                            (long) strtol(tokens[i].palavra, NULL, 10) < (long) 0) {
                                    erro = LEXICO;

                                    break;
                                }
                            }

                        } else { // faltam argumentos
                            erro = GRAMATICAL;
                            linha = tokens[i].linha;

                            break;
                        }

                        break;
                    case ALIGN:

                        if (i + 1 < tamanho) {
                            // captura o argumento 1
                            tokens[++i] = recuperaToken(0);
                            removerToken(0);

                            erro = validTokenType(&tokens[i]);
                            linha = tokens[i].linha;

                            // verifica erro lexico
                            if (erro == LEXICO) break;

                            // verifica erro gramatical
                            if (tokens[i].tipo != Decimal || tokens[i].linha != tokens[i-1].linha) {
                                erro = GRAMATICAL;

                                break;
                            } else if (tokens[i].tipo == Decimal) {
                                // erro de sintaxe (ultrapassa limites estabelecidos)
                                if ((long) strtol(tokens[i].palavra, NULL, 10) >= (long) 1024 ||
                                            (long) strtol(tokens[i].palavra, NULL, 10) < (long) 1) {
                                    erro = LEXICO;

                                    break;
                                }
                            }

                        } else { // faltam argumentos
                            erro = GRAMATICAL;
                            linha = tokens[i].linha;

                            break;
                        }

                        break;
                    case WFILL:

                        if (i + 2 < tamanho) {
                            // captura o argumento 1
                            tokens[++i] = recuperaToken(0);
                            removerToken(0);

                            erro = validTokenType(&tokens[i]);
                            linha = tokens[i].linha;

                            // verifica erro lexico
                            if (erro == LEXICO) break;

                            // verifica erro gramatical
                            if (tokens[i].tipo != Decimal || tokens[i].linha != tokens[i-1].linha) {
                                erro = GRAMATICAL;

                                break;
                            } else if (tokens[i].tipo == Decimal) {
                                // erro de sintaxe (ultrapassa limites estabelecidos)
                                if ((long) strtol(tokens[i].palavra, NULL, 10) >= (long) 1024 ||
                                            (long) strtol(tokens[i].palavra, NULL, 10) < (long) 1) {
                                    erro = LEXICO;

                                    break;
                                }
                            }

                            // captura argumento 2
                            tokens[++i] = recuperaToken(0);
                            removerToken(0);

                            erro = validTokenType(&tokens[i]);
                            linha = tokens[i].linha;

                            // verifica erro lexico
                            if (erro == LEXICO) break;

                            // verifica erro gramatical
                            if ((tokens[i].tipo != Hexadecimal && tokens[i].tipo != Decimal &&
                                    tokens[i].tipo != Nome) || tokens[i].linha != tokens[i-2].linha) {
                                erro = GRAMATICAL;

                                break;
                            } else if (tokens[i].tipo == Decimal) {
                                // erro de sintaxe (ultrapassa limites estabelecidos)
                                if ((long) strtol(tokens[i].palavra, NULL, 10) >= (long) 2147483648 ||
                                            (long) strtol(tokens[i].palavra, NULL, 10) < (long) -2147483648) {
                                    erro = LEXICO;

                                    break;
                                }
                            }

                        } else { // faltam argumentos
                            erro = GRAMATICAL;
                            linha = tokens[i].linha;

                            break;
                        }
                        break;
                    case WORD:

                        if (i + 1 < tamanho) {
                            // captura o argumento 1
                            tokens[++i] = recuperaToken(0);
                            removerToken(0);

                            erro = validTokenType(&tokens[i]);
                            linha = tokens[i].linha;

                            // verifica erro lexico
                            if (erro == LEXICO) break;

                            // verifica erro gramatical
                            if ((tokens[i].tipo != Hexadecimal && tokens[i].tipo != Decimal &&
                                    tokens[i].tipo != Nome) || tokens[i].linha != tokens[i-1].linha) {
                                erro = GRAMATICAL;

                                break;
                            } else if (tokens[i].tipo == Decimal) {
                                // erro de sintaxe (ultrapassa limites estabelecidos)
                                if ((long) strtol(tokens[i].palavra, NULL, 10) >= (long) 4294967296 ||
                                            (long) strtol(tokens[i].palavra, NULL, 10) < (long) -2147483648) {
                                    erro = LEXICO;

                                    break;
                                }
                            }

                        } else { // faltam argumentos
                            erro = GRAMATICAL;
                            linha = tokens[i].linha;

                            break;
                        }

                        break;
                }

                break;
            case DefRotulo:
                // esta condicao eh valida quando a estrutura
                // [rotulo:] [diretiva / instrucao] nao eh satisfeita
                if (i >= 1 && tokens[i-1].linha == tokens[i].linha) {
                    erro = GRAMATICAL;
                    linha = tokens[i].linha;

                    break;
                }

                break;
            case Hexadecimal:
            case Decimal:
            case Nome:
                erro = GRAMATICAL;
                linha = tokens[i].linha;

                break;
        }
    }

    if (erro == NULO) {
        for (unsigned  i = 0; i < tamanho; i++) adicionarToken(tokens[i]);

        // imprimeListaTokens();
    } else {
        if (erro == GRAMATICAL)
            printf("ERRO GRAMATICAL: palavra na linha %d!\n", linha);
        else
            printf("ERRO LEXICO: palavra inválida na linha %d!\n", linha);

        return 1;
    }

    return 0;
}

/*
 * Argumentos:
 *  entrada: cadeia de caracteres com o conteudo do arquivo de entrada.
 *  tamanho: tamanho da cadeia.
 * Retorna:
 *  1 caso haja erro na montagem;
 *  0 caso não haja erro.
 */
int processarEntrada(char *entrada, unsigned tamanho)  {
    preProcessamento(entrada, tamanho);

    return validaEntrada();
}
