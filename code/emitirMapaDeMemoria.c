#include "montador.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum Tipo { ROT, SYM, DIR, INSTRU } Tipo;
typedef enum Diretivas { SET, ORG, ALIGN, WFILL, WORD } Diretivas;

typedef enum Instrucoes {
        LOAD = 1, LOADM = 2, LOADB = 3,
        LOADmq = 10, LOADmq_mx = 9,
        STOR = 33, JUMP = 13, JMPP = 15,
        ADD = 5, ADDB = 7, SUB = 6,
        SUBB = 8, MUL = 11, DIV = 12,
        LSH = 20, RSH = 21, STORA = 18
} Instrucoes;

typedef struct DirRot {
    Tipo tipo;
    int flag; // 0: posicao de montagem a esquerda e 1: a direita
    long int valor;
    char *palavra;
    int instruUpdate;
} DirRot;

Instrucoes getInstrucao2(char *str) {
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

Diretivas getDiretiva2(char * palavra) {
    if (!strcmp(palavra, ".set")) return SET;
    else if(!strcmp(palavra, ".org")) return ORG;
    else if (!strcmp(palavra, ".align")) return ALIGN;
    else if (!strcmp(palavra, ".wfill")) return WFILL;
    else return WORD;
}

int erro() {
    printf("Impossível montar o código!\n");
    return 1;
}

/* Retorna:
 *  1 caso haja erro na montagem;
 *  0 caso não haja erro.
 */
int emitirMapaDeMemoria() {
    Token token;
    DirRot *dirRots = NULL, *pendentes = NULL;
    long int valor1, valor2, tam;
    char hex[66], mapa[1026][14];
    // em ladoMontagem, tem-se 0: esquerda e 1: direita
    unsigned int posMontagem = 0, ladoMontagem = 0, dirRotPos = 0, posPendent = 0;

    // seta a primeira coluna como 0, indicando que a principio
    // a linha nao sera exibida na saida. Inicializa o resto com 0.
    for (int i = 0; i < 1026; i++) {
        sprintf(hex, "%010lX", (long) i);
        tam = (int) strlen(hex);

        for (int j = 0; j < 14; j++) mapa[i][j] = '0';
        for (int k = 0, l = 3; k < 3; k++, l--) mapa[i][l] = hex[tam - 1 - k];
    }

    // primeira etapa: encontra todos os rotulos e diretivas set
    // (preenchendo parcialmente a memoria neste processo)
    for (unsigned int i = 0; i < getNumberOfTokens(); i++) {
        token = recuperaToken(i);

        if (token.tipo == DefRotulo ||
                (token.tipo == Diretiva && getDiretiva2(token.palavra) == SET)) {
            if (token.tipo == Diretiva) token = recuperaToken(++i);

            tam = strlen(token.palavra);
            strcpy(hex, token.palavra);

            if (hex[tam-1] == ':') hex[tam-1] = 0;
            else {
                hex[tam] = ':';
                hex[tam + 1] = 0;
            }

            // dois rotulos / diretivas com o mesmo nome sao invalidos...
            for (unsigned int j = 0; j < dirRotPos; j++)
                if (!strcmp(dirRots[j].palavra, token.palavra) ||
                        !strcmp(dirRots[j].palavra, hex)) return erro();

            dirRots = (DirRot *) realloc(dirRots, (dirRotPos + 1) * sizeof(DirRot));

            if (token.tipo == DefRotulo) {
                dirRots[dirRotPos].tipo = ROT;
                dirRots[dirRotPos].flag = ladoMontagem;
                dirRots[dirRotPos].valor = posMontagem;
                dirRots[dirRotPos].palavra = token.palavra;
                dirRots[dirRotPos].instruUpdate = 0;
            } else { // diretiva .set
                dirRots[dirRotPos].tipo = SYM;
                dirRots[dirRotPos].palavra = token.palavra; // nome da diretiva
                dirRots[dirRotPos].instruUpdate = 0;

                token = recuperaToken(++i);

                // converte valor associado a diretiva para (int)
                if (token.tipo == Hexadecimal)
                    dirRots[dirRotPos].valor = strtol(token.palavra, NULL, 0);
                else dirRots[dirRotPos].valor = strtol(token.palavra, NULL, 10);
            }

            dirRotPos++;
        } else if (token.tipo == Diretiva && getDiretiva2(token.palavra) == ORG) {
            token = recuperaToken(++i);

            // muda o ponto de montagem
            if (token.tipo == Hexadecimal)
                posMontagem = strtol(token.palavra, NULL, 0);
            else posMontagem = strtol(token.palavra, NULL, 10);

            ladoMontagem = 0;
        } else if (token.tipo == Diretiva && getDiretiva2(token.palavra) == ALIGN) {
            token = recuperaToken(++i);

            valor1 = strtol(token.palavra, NULL, 10);

            if (ladoMontagem == 1) {
                ladoMontagem = 0;
                posMontagem++;
            }

            // encontra primeira linha mhultipla de valor1
            while (posMontagem % valor1 != 0) posMontagem++;
        } else if (token.tipo == Diretiva && getDiretiva2(token.palavra) == WFILL) {
            // caso montagem esteja na direita, nao serah possivel
            // inserir o dado em decorrencia de palavras terem 40 bits
            if (ladoMontagem == 1) return erro();

            token = recuperaToken(++i);

            valor1 = strtol(token.palavra, NULL, 10);

            token = recuperaToken(++i);

            if (token.tipo == Hexadecimal) valor2 = strtol(token.palavra, NULL, 0);
            else if (token.tipo == Decimal) valor2 = strtol(token.palavra, NULL, 10);

            // converte valor inteiro para string hexadecimal
            if (token.tipo == Hexadecimal || token.tipo == Decimal) {
                sprintf(hex, "%010lX", (long) valor2);
                tam = (int) strlen(hex);
            }

            for (int j = 0; j < valor1; j++, posMontagem++) {
                mapa[posMontagem][0] = '1';

                // inicializa posicao na memoria
                if (token.tipo == Hexadecimal || token.tipo == Decimal)
                    for (int k = 0, l = 13; k < 10; k++, l--) mapa[posMontagem][l] = hex[tam - 1 - k];
                else if (token.tipo == Nome) {
                    pendentes = (DirRot *) realloc(pendentes, (posPendent + 1) * sizeof(DirRot));

                    pendentes[posPendent].tipo = DIR;
                    pendentes[posPendent].flag = 1; // posicao de montagem a direita (palavras de 40 bits)
                    pendentes[posPendent].valor = posMontagem;
                    pendentes[posPendent].palavra = token.palavra;
                     // nao eh instrucao, por isso nao precisa de update
                     // caso posicao de montagem de rotulo esteja a direita
                    pendentes[posPendent].instruUpdate = 0;

                    posPendent++;
                }
            }
        } else if (token.tipo == Diretiva && getDiretiva2(token.palavra) == WORD) {
            // caso montagem esteja na direita, nao serah possivel
            // inserir o dado em decorrencia de palavras terem 40 bits
            if (ladoMontagem == 1) return erro();

            token = recuperaToken(++i);

            if (token.tipo == Hexadecimal) valor2 = strtol(token.palavra, NULL, 0);
            else if (token.tipo == Decimal) valor2 = strtol(token.palavra, NULL, 10);
            else if (token.tipo == Nome) {
                pendentes = (DirRot *) realloc(pendentes, (posPendent + 1) * sizeof(DirRot));

                pendentes[posPendent].tipo = DIR;
                pendentes[posPendent].flag = 1;
                pendentes[posPendent].valor = posMontagem;
                pendentes[posPendent].palavra = token.palavra;
                pendentes[posPendent].instruUpdate = 0; // vide passagem em wfill

                posPendent++;
            }

            // converte valor inteiro para string hexadecimal
            if (token.tipo == Hexadecimal || token.tipo == Decimal) {
                sprintf(hex, "%010lX", (long) valor2);
                tam = (int) strlen(hex);
            }


            mapa[posMontagem][0] = '1'; // linha devera ser exibida na saida

            // inicializa posicao na memoria
            if (token.tipo == Hexadecimal || token.tipo == Decimal)
                for (int k = 0, l = 13; k < 10; k++, l--) mapa[posMontagem][l] = hex[tam - 1 - k];

            posMontagem++;
        }  else if (token.tipo == Instrucao) {
            valor1 = getInstrucao2(token.palavra);

            sprintf(hex, "%010lX", (long) valor1);
            tam = (int) strlen(hex);

            // verifica se instrucao requer endereco
            if (valor1 != LSH && valor1 != RSH && valor1 != LSH && valor1 != LOADmq) {
                token = recuperaToken(++i);

                if (token.tipo == Hexadecimal) valor2 = strtol(token.palavra, NULL, 0);
                else if (token.tipo == Decimal) valor2 = strtol(token.palavra, NULL, 10);
                else if (token.tipo == Nome) {
                    pendentes = (DirRot *) realloc(pendentes, (posPendent + 1) * sizeof(DirRot));

                    pendentes[posPendent].tipo = INSTRU;
                    pendentes[posPendent].flag = ladoMontagem;
                    pendentes[posPendent].valor = posMontagem;
                    pendentes[posPendent].palavra = token.palavra;

                    // indica a possibilidade de atualizacao do opcode da instrucao
                    if (valor1 == JUMP || valor1 == JMPP || valor1 == STORA)
                        pendentes[posPendent].instruUpdate = 1;
                    else pendentes[posPendent].instruUpdate = 0;

                    posPendent++;
                }
            } else valor2 = 0;

            if (ladoMontagem == 0) {
                mapa[posMontagem][0] = '1';

                // coloca instrucao na memoria
                for (int k = 0, l = 5; k < 2; k++, l--) mapa[posMontagem][l] = hex[tam - 1 - k];

                // coloca endereco associado a instrucao na memoria
                if (token.tipo == Hexadecimal || token.tipo == Decimal) {
                    sprintf(hex, "%010lX", (long) valor2);
                    tam = (int) strlen(hex);
                    for (int k = 0, l = 8; k < 3; k++, l--) mapa[posMontagem][l] = hex[tam - 1 - k];
                }

                ladoMontagem = 1;
            } else {
                 // coloca instrucao na memoria
                 for (int k = 0, l = 10; k < 2; k++, l--) mapa[posMontagem][l] = hex[tam - 1 - k];

                 // coloca endereco associado a instrucao na memoria
                 if (token.tipo == Hexadecimal || token.tipo == Decimal) {
                     sprintf(hex, "%010lX", (long) valor2);
                     tam = (int) strlen(hex);
                     for (int k = 0, l = 13; k < 3; k++, l--) mapa[posMontagem][l] = hex[tam - 1 - k];
                 }

                 ladoMontagem = 0;
                 posMontagem++;
             }
        }
    }

    // segunda etapa: preenche posicoes na memoria associadas a
    // rotulos e diretivas, ignoradas na etapa 1
    for (unsigned int i = 0; i < posPendent; i++) {
        unsigned int j;
        for (j = 0; j < dirRotPos; j++) {
            if (!strcmp(dirRots[j].palavra, pendentes[i].palavra)) break;

            tam = strlen(pendentes[i].palavra);
            strcpy(hex, pendentes[i].palavra);

            hex[tam] = ':';
            hex[tam + 1] = 0;

            if (!strcmp(dirRots[j].palavra, hex)) break;
        }

        if (j == dirRotPos) {
            printf("USADO MAS NÃO DEFINIDO: %s!\n", pendentes[i].palavra);

            return 1;
        } else {
            if (pendentes[i].tipo == INSTRU && dirRots[j].tipo == SYM) return erro();

            sprintf(hex, "%010lX", (long) dirRots[j].valor);
            tam = (int) strlen(hex);

            valor1 = pendentes[i].flag;

            if (pendentes[i].tipo == DIR) {
                for (int k = 0, l = 13; k < 10; k++, l--)
                    mapa[pendentes[i].valor][l] = hex[tam - 1 - k];
            } else {
                for (int k = 0, l = (valor1 == 0 ? 8 : 13);
                            k < 3; k++, l--) mapa[pendentes[i].valor][l] = hex[tam - 1 - k];
            }

            // se rotulo encontra-se a direita da linha e instrucao
            // eh sensivel a tal caso, realiza atualizacao do opcode
            if (pendentes[i].instruUpdate == 1 && dirRots[j].flag == 1) {
                hex[0] = mapa[pendentes[i].valor][(valor1 == 0 ? 4 : 9)];
                hex[1] = mapa[pendentes[i].valor][(valor1 == 0 ? 5 : 10)];
                hex[2] = 0;

                valor2 = strtol(hex, NULL, 16) + 1;

                sprintf(hex, "%010lX", (long) valor2);
                tam = (int) strlen(hex);

                for (int k = 0, l = (valor1 == 0 ? 5 : 10);
                            k < 2; k++, l--) mapa[pendentes[i].valor][l] = hex[tam - 1 - k];
            }
        }
    }

    for (unsigned int i = 0; i < 1024; i++) {
        if (mapa[i][0] == '1') {
            for (int j = 1; j < 14; j++) {
                printf("%c", mapa[i][j]);
                if (j == 3 || j ==  5 || j == 8 || j == 10) printf(" ");
            }
            printf("\n");
        }
    }

    // libera a memória alocada no processamento da entrada
    while (getNumberOfTokens() != 0) {
        token = recuperaToken(0);

        free(token.palavra);
        removerToken(0);
    }

    return 0;
}
