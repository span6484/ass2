// tuple.h ... interface to functions on Tuples
// part of Multi-attribute Linear-hashed Files
// A Tuple is just a '\0'-terminated C string
// Consists of "val_1,val_2,val_3,...,val_n"
// See tuple.c for details on functions
// Last modified by John Shepherd, July 2019

#ifndef TUPLE_H
#define TUPLE_H 1

typedef char *Tuple;

#include "reln.h"
#include "bits.h"

int tupLength(Tuple t);
Tuple readTuple(Reln r, FILE *in);
Bits tupleHash(Reln r, Tuple t);               //针对tuple做一个hash
void tupleVals(Tuple t, char **vals);           //tuple转成二进制的数
void freeVals(char **vals, int nattrs);
Bool tupleMatch(Reln r, Tuple t1, Tuple t2);        //判断两个是否匹配， 一个是tuple， 一个是R 后面的100，？，？ 这个
void tupleString(Tuple t, char *buf);               // tuple 放入string里


#endif
