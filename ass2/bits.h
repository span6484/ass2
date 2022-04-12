// bits.h ... interface to functions on bit-strings
// part of Multi-attribute Linear-hashed Files
// See bits.c for details of functions
// Last modified by John Shepherd, July 2019

#ifndef BITS_H
#define BITS_H 1

typedef unsigned int Bits;

int bitIsSet(Bits, int);                // 判断位置是否 set为1
Bits setBit(Bits, int);                 // 设置为1
Bits unsetBit(Bits, int);               //设置为0
Bits getLower(Bits, int);               //Bit对应hash， int对应index， 比如 b = 1000011, n = 2. getLower(b,n) = 11
void bitsString(Bits, char *);          // bit to string

#endif
