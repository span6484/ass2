// query.c ... query scan functions
// part of Multi-attribute Linear-hashed Files
// Manage creating and using Query objects
// Last modified by John Shepherd, July 2019

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"
#include "hash.h"

// A suggestion ... you can change however you like

struct QueryRep {
	Reln    rel;       // need to remember Relation info 检索
	Bits    known;     // the hash value from MAH
	Bits    unknown;   // the unknown bits from MAH
	PageID  curpage;   // current page in scan  当前bucket pageiD， 这个不改变
	int     is_ovflow; // are we in the overflow pages?
	Offset  curtup;    // offset of current tuple within page           //当前tuple在哪,tuple的位置, 通常为字符串索引位置,EX: 0x0001   , curpage永远不变，是dataPage
	//TODO
    //添加自己的属性
    Offset curTupleIndex;
    PageID curScanPage;    //当前正在检索哪个位置, 这个是 data page or overflow page
    Tuple tuple;
    char* quesryString;   //检索传入的字符串
};
//00 -> a b ,c -> o01->d,e,f            00 是datapage， o01是 overflow page， curpage指向bucket page， 这个固定死了， 可以理解成 array的第一个， curPage指向00， curScanPage指向a,b,c
//01 -> h, i, g -> o02 -> x, y, z -> o03 -> m,n
// take a query string (e.g. "1234,?,abc,?")            1234:known  ?:unknown
// set up a QueryRep object for the scan
// DATA PAGE -> OVERFLOW PAGE
//PAGE 上tuple


// getlower(known)
// 0 1 0 0            <- curDataPage
// 0 1 0 1
// 1 1 0 0
// 1 1 0 1

// known <= index <= (known | unknow)
Query startQuery(Reln r, char *q)
{
	Query new = malloc(sizeof(struct QueryRep));
	assert(new != NULL);
	// TODO
	// Partial algorithm:
	// form known bits from known attributes
	// form unknown bits from '?' attributes
	// compute PageID of first page
	//   using known bits and first "unknown" value
	// set all values in QueryRep object
    char buf[MAXBITS+1];// set tuple into array, ignore the , , ex: 123,?,? => ['123','?','?']

    Count nvals = nattrs(r);
    char **tuple_vals = malloc(nvals*sizeof(char *));
    Bits h[nvals];              //hash for each attribute
    Bits hash = 0;
    assert(tuple_vals != NULL);
    // set tuple into array, ignore the , , ex: 123,?,? => ['123','?','?']
    tupleVals(q,tuple_vals);
    printf("arribute num: %d\n", nattrs(r));
    int i,a,b;
    ChVecItem *cv = chvec(r);
    for (i = 0; i < nvals; i ++) {
        h[i] = hash_any((unsigned char *)tuple_vals[i], strlen(tuple_vals[i]));
    }
    for (i = 0; i < MAXCHVEC; i++) {
        a = cv[i].att;
        b = cv[i].bit;
        printf("%s\n", *(tuple_vals+a));
        if (bitIsSet(h[a],b) && strcmp("?", *(tuple_vals+a))) {
            hash = setBit(hash, i);
        }
        //TODO UNHASH
//        else{
//
//        }
    }
    //FINISH-------
//	hash = hash_any((unsigned char *)vals[0],strlen(vals[0]));
    bitsString(hash,buf);
    printf("%s\n", buf);
    return new;
}

// get next tuple during a scan
//
//遍历当前的page, 每次扫描的是curScanPage， curPage 是dataPage不变的， 变化的是curScanPage
// 如果有Overflow Page，curScanPage转到了OverFlowPage上
//如果当前Page没有， 往下一个bucket走，bucket 是page和overFlowPage

Tuple getNextTuple(Query q)
{
	// TODO
	// Partial algorithm:
	// if (more tuples in current page)
	//    get next matching tuple from current page
	// else if (current page has overflow)
	//    move to overflow page     curScanPage转
	//    grab first matching tuple from page
	// else
	//    move to "next" bucket
	//    grab first matching tuple from data page
	// endif
	// if (current page has no matching tuples)
	//    go to next page (try again)
	// endif

//    Page page = getPage(dataFile(q->rel), pageID);
//    Tuple t = pageData(page);
//    while (t != NULL) {
//        if (tupleMatch(q->rel, t, q->quesryString)) {
//            return t;
//        }
//        t = t + tupLength(t) + 1;
//
//    }

	return NULL;
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	// TODO
}
