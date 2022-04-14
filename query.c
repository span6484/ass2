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
	Bits    unknown;   // the unknown bits from MAH,unknown 可以理解为一个flag
	PageID  curpage;   // current page in scan  当前bucket pageiD， 这个不改变
	int     is_ovflow; // are we in the overflow pages?
	Offset  curtup;    // offset of current tuple within page           //当前tuple在哪,tuple的位置, 通常为字符串索引位置,EX: 0x0001   , curpage永远不变，是dataPage
	//TODO
    //添加自己的属性
    Offset curTupleIndex;
    PageID curScanPage;    //当前正在检索哪个位置, 这个是 data page or overflow page
    int nstars;
    Bits* unknowns;
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

int countBits();

int new_pow(int i, Count depth);

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
    Bits known = 0;
    Bits unknown = 0;
    assert(tuple_vals != NULL);
    // set tuple into array, ignore the , , ex: 123,?,? => ['123','?','?']
    tupleVals(q,tuple_vals);
    printf("arribute num: %d\n", nattrs(r));
    int i,a,b;
    ChVecItem *cv = chvec(r);
    for (i = 0; i < nvals; i ++) {
        h[i] = hash_any((unsigned char *)tuple_vals[i], strlen(tuple_vals[i]));
        bitsString(h[i], buf);
        printf("%s: %s\n", tuple_vals[i], buf);
    }
    for (i = 0; i < MAXCHVEC; i++) {
        a = cv[i].att;
        b = cv[i].bit;
        if (strcmp("?", *(tuple_vals+a))) {
            if (bitIsSet(h[a],b)) {
                known = setBit(known, i);
            }
        }
        //if ?, straingtly set in unknown
        else {
            unknown = setBit(unknown,i);
        }
    }
    int nstars = countBits(getLower(unknown, depth(r)));
    Bits* unknowns = malloc(sizeof(Bits) * nvals);
    assert(unknowns != NULL);
    for
    //FINISH-------
//    bitsString(known,buf);
//    printf("known: %s\n", buf);
//    bitsString(unknown,buf);
//    printf("unknown: %s\n", buf);
//    Bits hash_lower = getLower(known | unknown, depth(r));
//    bitsString(hash_lower,buf);
//    printf("depth %d\n", depth(r));
//    printf("hash_lower: %s\n", buf);
    new->nstars = nstars;
    new->known = known;
    new->unknown = unknown;
    new->rel = r;
    return new;
}

int countBits(unsigned int value)
{

    int count = 0;
    int cmp = 1;
    int i = 0;
    while(i < 33) {
        if ((cmp & value) != 0)  {
            count += 1;
        }
        cmp = cmp<<1;
        i++;
    }
    printf("count :%d\n", count);
    return count;
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

    char buf[MAXBITS+1];
    Bits known = q->known;
    Bits unknown = q->unknown;
    Reln r = q->rel;
    bitsString(known,buf);
    printf("In getNext Tuple-----------\n");
    printf("known: %s\n", buf);
    bitsString(unknown,buf);
    printf("unknown: %s\n", buf);
    Bits hash_lower = getLower(known | unknown, depth(r));
    bitsString(hash_lower,buf);
    printf("depth %d\n", depth(r));
    printf("hash_lower: %s\n", buf);
//    Page p = getPage(dataFile(r),hash_lower);
//    char *data = pageData(p);
    bitsString(getLower(unknown, depth(r)),buf);
    printf("unknown lower is: %s\n",buf);
    printf("valid_unknown: %d\n", q->nstars);
//    int i;

//    for (i = 0; i < new_pow(2, depth(r)); i++) {
//        p =
//    }
	return NULL;
}

int new_pow(int i, Count depth) {
    int j;
    int val = 1;
    for(j = 0; j < depth; j++) {
        val *= 2;
    }

    return val;
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	// TODO
}
