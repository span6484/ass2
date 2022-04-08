// page.h ... interface to functions on Pages
// part of Multi-attribute Linear-hashed Files
// See pages.c for descriptions of Page type and functions
// Last modified by John Shepherd, July 2019

#ifndef PAGE_H
#define PAGE_H 1

typedef struct PageRep *Page;

#include "defs.h"
#include "tuple.h"

Page newPage();
PageID addPage(FILE *);             //文件上添加一个page， pageID形式就是1000111
Page getPage(FILE *, PageID);       //根据0011这种获取
Status putPage(FILE *, PageID, Page);       //写入文件， 如果调用putPage， p会被free掉, 如果free了会报错
Status addToPage(Page, Tuple);              // page上添加1个tuple
char *pageData(Page);               // tuple是用字符串一个个存， ex： page head:aa\0bb\0cc\0, 所以bb位置是p->data + strlen(p->data) + 1
Count pageNTuples(Page);
Offset pageOvflow(Page);            // 是否有overflow Page, 没有的话就是NOPage
void pageSetOvflow(Page, PageID);       //SET一个Overflow Page
Count pageFreeSpace(Page);          // CHECK 有没有空间

#endif
