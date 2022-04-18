#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef unsigned int Bits;
typedef unsigned int Count;
Bits *recursion(Bits known, Bits unknown, int cur, Bits *knowns);
Bits setBit(Bits val, int position);
int pow1(int i, Count depth);
int bitIsSet(Bits val, int position);
Bits unsetBit(Bits val, int position);
void printBits(Bits val);
void bitsString(Bits val, char *buf);
Bits *recursion(Bits known, Bits unknown, int cur, Bits *knowns) {

    if (unknown == 0) {
        int i = 0;
        while(1) {
            if (knowns[i] == 0) {
                knowns[i] = known;
                break;
            }
            else {
                i++;
            }
        }
        return knowns;
    }

    for (int i = cur -1; i >= 0; i--) {
        if (bitIsSet(unknown,i)) {
            unknown = unsetBit(unknown, i);
            recursion(known, unknown, i,knowns);
            known = setBit(known, i);
            recursion(known,unknown, i, knowns);
            break;
        }
    }
    printf("recursion\n");
    return knowns;
}

int main(){
    Bits known_lower = 36;
    Bits unknown_lower = 27;
    Bits known = 8538404;
    Bits* knowns = malloc(sizeof(Bits) * 16);
    knowns = recursion(known_lower,unknown_lower, 32, knowns);
    for (int i = 0; i < pow1(2, 3); i++) {
        knowns[i] = knowns[i] | known;
        printBits(knowns[i]);
    }
    return 0;
}


int pow1(int i, Count depth) {
    int j;
    int val = 1;
    for(j = 0; j < depth; j++) {
        val *= 2;
    }

    return val;
}

void printBits(Bits val) {
    char buf[100];
    bitsString(val, buf);
    printf("%s\n", buf);
}

int bitIsSet(Bits val, int position)        // 判断位置是否 set为1
{
    assert(0 <= position && position <= 31);
    Bits mask = (1 << position);
    return ((val & mask) != 0);
}
Bits unsetBit(Bits val, int position)
{
    assert(0 <= position && position <= 31);
    Bits mask = (~(1 << position));
    return (val & mask);
}
Bits setBit(Bits val, int position)
{
    assert(0 <= position && position <= 31);
    Bits mask = (1 << position);
    return (val | mask);
}
void bitsString(Bits val, char *buf)
{
    int i,j; char ch;
    Bits bit = 0x80000000;

    i = j = 0;
    while (bit != 0) {
        ch = ((val & bit) != 0) ? '1' : '0';
        buf[i++] = ch;
        j++;
        if (j%8 == 0) buf[i++] = ' ';
        bit = bit >> 1;
    }
    buf[--i] = '\0';
}