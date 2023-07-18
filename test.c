#include <stdio.h>
#include <stdlib.h>

void printBits(size_t const size, void const * const ptr, 
    char* end, char* label, int use_label)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    if (use_label == 1)
        printf("%s: ", label);

    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    printf("%s", end);
}

int main() {

    int a = -14;
    int b = -15;
    printf("u_a: %d u_b: %d a: %d b: %d\n", (unsigned int) a, (unsigned int) b, a, b);
    int r1 = a >> b;
    int r2 = (unsigned int) a >> (unsigned int) b;

    printBits(sizeof(int), &a, " ", "a", 1);
    printBits(sizeof(int), &b, "\n", "b", 1);
    printBits(sizeof(int), &r1, " ", "r1", 1);
    printBits(sizeof(int), &r2, "\n", "r2", 1);

    printf("r1: 0x%x r2: 0x%x\n", r1, r2);
    
}