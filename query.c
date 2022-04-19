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
    int curPageIndex;
    PageID curScanPage;    //当前正在检索哪个位置, 这个是 data page or overflow page
    int nstars;
    int preTupleLen;
    Bits* knowns;
    char* quesryString;
    Page curpage_p;
    int visited_pid[][1024];
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

int pow1(int i, Count depth);

Bits *recursion(Bits known, Bits unknown, int cur, Bits *knowns);

void printBits(Bits val);

Bool tupleMatch_new(Reln r, Tuple t1, Tuple t2);

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
    ////////printf("arribute num: %d\n", nattrs(r));
    int i,a,b;
    ChVecItem *cv = chvec(r);
    for (i = 0; i < nvals; i ++) {
        h[i] = hash_any((unsigned char *)tuple_vals[i], strlen(tuple_vals[i]));
        bitsString(h[i], buf);
        ////////printf("%s: %s\n", tuple_vals[i], buf);
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
//    //printf("unknow:\n");
//    //printBits(unknown);
    //printf("depth:%d\n", depth(r));
    int nstars = countBits(getLower(unknown, depth(r)+1));
    Bits* knowns = malloc(sizeof(Bits) * pow1(2,nvals+1));
    assert(knowns != NULL);
    Bits known_lower = getLower(known, depth(r)+1);
    Bits unknown_lower = getLower(unknown, depth(r)+1);
//    printf("%d\n",known);
//////    //printf("known_lower:    ");

//    //printBits(known);
//////    //printf("%d\n", known_lower);
//////    //printf("unknown_lower:    ");
//    //printBits(unknown_lower);
//////    //printf("%d\n", unknown_lower);
    knowns = recursion(known_lower,unknown_lower, 32, knowns);

    //get out all possibles for knowns
    //printf("knowns:\n");
    for (int i = 0; i < pow1(2, nstars); i++) {
        knowns[i] = knowns[i] | known;
        //printBits(knowns[i]);
    }
    //FINISH-------

//    bitsString(known,buf);
//////    ////printf("known: %s\n", buf);
//    bitsString(unknown,buf);
//////    ////printf("unknown: %s\n", buf);
//    Bits hash_lower = getLower(known | unknown, depth(r));
//    bitsString(hash_lower,buf);
//////    ////printf("depth %d\n", depth(r));
//////    ////printf("hash_lower: %s\n", buf);


    new->nstars = nstars;
    new->known = known;
    new->unknown = unknown;
    new->rel = r;
    new->knowns = knowns;
    new->quesryString = q;
    new->curTupleIndex = 0;
    new->curPageIndex = 0;
//    for (int f = 0; f < nvals; f++) {
//        free(tuple_vals[f]);
//    }
//    free(tuple_vals);
//    free(knowns);
    return new;
}

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
    return knowns;
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
    ////////printf("count :%d\n", count);
    return count;
}


// get next tuple during a scan
//
//遍历当前的page, 每次扫描的是curScanPage， curPage 是dataPage不变的， 变化的是curScanPage
// 如果有Overflow Page，curScanPage转到了OverFlowPage上
//如果当前Page没有， 往下一个bucket走，bucket 是page和overFlowPage

Tuple getEachTuple(Query q)
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


    Reln r = q->rel;
    Bits real_known;
    PageID pid;
    int nstars;
    int curPageIndex;           // knowns index knowns[curPageIndex] = knowns[0]
    Offset curTupleIndex;
    Count nTuples;
    assert(q->knowns != NULL);
    READPAGE:
    nstars = q->nstars;
    curPageIndex = q->curPageIndex;
    curTupleIndex = q->curTupleIndex;
    Page curPage;
    Tuple t = NULL;
    //printf("knowns:\n");
//    for (int i = 0; i < pow1(2, nstars); i++) {
//        //printBits(q->knowns[i]);
//    }

    //printf("real known0_0\n");
//    printf("%d\n",q->visited_pid[curPageIndex][0]);

    if (curPageIndex < pow1(2,nstars)) {

        real_known = q->knowns[curPageIndex];
        //printf("real known0:     \n");
        //printBits(real_known);
        //printf("curpage Index0 is: %d\n",curPageIndex);
        // overflowpid
        if (q->is_ovflow == 1) {
            pid = q->curScanPage;
            //printf("pid0_1:    ");
            //printBits(pid);
            //printf("test0_1     ");
            //printBits(q->knowns[4]);
        }
            // normal pid
        else {
            if (depth(q->rel) == 0) {
                pid = 0;
            } else {
                //printf("real known0_2:     \n");
                //printBits(real_known);
                //printf("depth0_2:     %d\n", depth(q->rel));
                pid = getLower(real_known, depth(q->rel));
                //printf("pid0_2:    ");
                //printBits(pid);
                //printf("test0_2     ");
                //printBits(q->knowns[4]);
                if (pid < splitp(q->rel)) pid = getLower(real_known, depth(q->rel) + 1);
                //printf("pid0_3:    ");
                //printBits(pid);

                //printf("test0_3     ");
                //printBits(q->knowns[4]);
            }
            q->curpage = pid;
            q->curScanPage = pid;
        }
        if (q->is_ovflow == 1) {
            curPage = getPage(ovflowFile(r), pid);
            nTuples = pageNTuples(curPage);
            q->curpage_p = curPage;
        }
        else{
            curPage = getPage(dataFile(r), pid);
            nTuples = pageNTuples(curPage);
            q->curpage_p = curPage;
        }
        //printf("pid0:    ");
        //printBits(pid);
        //printf("curpage Index0 is: ");
        //printf("%d\n", q->curPageIndex);
        if (q->curTupleIndex < nTuples) {
            t = pageData(q->curpage_p) + q->preTupleLen;
            q->preTupleLen = strlen(t) + q->preTupleLen + 1;
            if (curTupleIndex < nTuples) {
                curTupleIndex++;
                q->curTupleIndex = curTupleIndex;
//                q->curScanPage = pid;
            }
            else if (pageOvflow(q->curpage_p) != NO_PAGE) {
                q->is_ovflow = 1;
                q->curScanPage = pageOvflow(q->curpage_p);
                //printf("ovf1:    %d\n",q->curScanPage);
                q->curTupleIndex = 0;
                q->preTupleLen = 0;
                goto READPAGE;
            }
            else {
                //printf("else1\n");
                q->is_ovflow = 0;
                q->preTupleLen = 0;
                q->curTupleIndex = 0;
                curPageIndex++;
                q->curPageIndex = curPageIndex;
                goto READPAGE;
            }
        }

            //start overflow
        else if (pageOvflow(q->curpage_p) != NO_PAGE) {
            q->is_ovflow = 1;
            q->curScanPage = pageOvflow(q->curpage_p);
            //printf("ovf2:    %d\n",q->curScanPage);
            q->curTupleIndex = 0;
            q->preTupleLen = 0;
            goto READPAGE;
        }
        else {
//            printf("create new Page,  curPage: %d,  curPageIndex: %d\n",q->curScanPage, q->curPageIndex);
//            printf("curScanPage:    %d\n",q->curScanPage);
//            //printf("here3---create new Page\n");
            ////printf("%s\n", t);
            //printf("else2\n");
            q->is_ovflow = 0;
            q->preTupleLen = 0;
            q->curTupleIndex = 0;
            curPageIndex++;
            //printf("pid2:    ");
            //printBits(pid);
            //printf("curpage Index2 is: %d\n",curPageIndex);

            q->curPageIndex = curPageIndex;
            //printf("------------------\n\n");
            goto READPAGE;
        }

        //printf("test3     ");
        //printBits(q->knowns[4]);
        //printf("return t\n");
        return t;
    }
    return NULL;
}
Tuple getNextTuple(Query q)
{
    Tuple t = getEachTuple(q);
    //printf("test4     ");
    //printBits(q->knowns[4]);
    while (t != NULL) {
        //printf("test5     ");
        //printBits(q->knowns[4]);
        if (tupleMatch_new(q->rel, t, q->quesryString) && q->visited_pid[q->curPageIndex][q->curTupleIndex] == 0) {
            q->visited_pid[q->curPageIndex][q->curTupleIndex] = 1;
//            printf("here  pid:    %d    tupleIndex:   %d\n", q->curScanPage, q->curTupleIndex);
            //printf("test5_1     ");
            //printBits(q->knowns[4]);
            return t;
        }

        //printf("test6     ");
        //printBits(q->knowns[4]);
        t = getEachTuple(q);
//        //printf("%s      pid:    %d    tupleIndex:   %d\n",t, q->curScanPage, q->curTupleIndex);
//        t = getEachTuple(q);
    }
    //printf("%s      pid:    %d    tupleIndex:   %d\n",t, q->curScanPage, q->curTupleIndex);
    return NULL;
}

Bool tupleMatch_new(Reln r, Tuple t1, Tuple t2) {
    Count na = nattrs(r);
    char **v1 = malloc(na*sizeof(char *));
    tupleVals(t1, v1);
    char **v2 = malloc(na*sizeof(char *));
    tupleVals(t2, v2);
    Bool match = TRUE;
    int i;
    for (i = 0; i < na; i++) {
        // assumes no real attribute values start with '?'
        if (v1[i][0] == '?' || v2[i][0] == '?') continue;
        if (strcmp(v1[i],v2[i]) == 0) continue;
        match = FALSE;
    }
    return match;
}

//Tuple getNextTuple(Query q) {
//    for (int i = 0; i < pow1(2, q->nstars);i++) {
//        //printBits(q->knowns[i]);
//    }
//    //printf("%d\n", q->nstars);
//    return NULL;
//}





void printBits(Bits val) {
    char buf[100];
    bitsString(val, buf);
    printf("%s\n", buf);
}

int pow1(int i, Count depth) {
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
//    free(q->knowns);
////    free(q->quesryString);
//    free(q);
}
