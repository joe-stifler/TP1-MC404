#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main() {
    char hex[30] = "-10";
    //
    // memset(hex, '0', sizeof(hex));
    //
    // sprintf(hex, "%x", num);
    // printf("%s %d\n", hex, (int) strlen(hex));

    int numero = -2;
    sprintf(hex,"%10lX", (long)numero);
    printf("%s", hex);
    // long long int dec = 1111111110000000000000000000000000000001;
    // FFFFFFFFFE
    // FF80000001

    // numero = convertBinaryToDecimal(dec);
    return 0;
}
