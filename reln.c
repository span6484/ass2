// reln.c ... functions on Relations
// part of Multi-attribute Linear-hashed Files
// Last modified by John Shepherd, July 2019

#include "defs.h"
#include "reln.h"
#include "page.h"
#include "tuple.h"
#include "chvec.h"
#include "bits.h"
#include "hash.h"

#define HEADERSIZE (3*sizeof(Count)+sizeof(Offset))

struct RelnRep {
	Count  nattrs; // number of attributes
	Count  depth;  // depth of main data file	d， 从后往前
	Offset sp;     // split pointer
    Count  npages; // number of main data pages
    Count  ntups;  // total number of tuples
	ChVec  cv;     // choice vector
	char   mode;   // open for read/write
	FILE  *info;   // handle on info file
	FILE  *data;   // handle on data file         DATA PAGE
	FILE  *ovflow; // handle on ovflow file
};

// create a new relation (three files)




void Splitting(Reln r);

Offset new_pow(int i, Count depth);

Status newRelation(char *name, Count nattrs, Count npages, Count d, char *cv)
{
    char fname[MAXFILENAME];
	Reln r = malloc(sizeof(struct RelnRep));
	r->nattrs = nattrs; r->depth = d; r->sp = 0;
	r->npages = npages; r->ntups = 0; r->mode = 'w';
	assert(r != NULL);
	if (parseChVec(r, cv, r->cv) != OK) return ~OK;
	sprintf(fname,"%s.info",name);
	r->info = fopen(fname,"w");
	assert(r->info != NULL);
	sprintf(fname,"%s.data",name);
	r->data = fopen(fname,"w");
	assert(r->data != NULL);
	sprintf(fname,"%s.ovflow",name);
	r->ovflow = fopen(fname,"w");
	assert(r->ovflow != NULL);
	int i;
	for (i = 0; i < npages; i++) addPage(r->data);
	closeRelation(r);
	return 0;
}

// check whether a relation already exists

Bool existsRelation(char *name)
{
	char fname[MAXFILENAME];
	sprintf(fname,"%s.info",name);
	FILE *f = fopen(fname,"r");
	if (f == NULL)
		return FALSE;
	else {
		fclose(f);
		return TRUE;
	}
}

// set up a relation descriptor from relation name
// open files, reads information from rel.info

Reln openRelation(char *name, char *mode)
{
	Reln r;
	r = malloc(sizeof(struct RelnRep));
	assert(r != NULL);
	char fname[MAXFILENAME];
	sprintf(fname,"%s.info",name);
	r->info = fopen(fname,mode);
	assert(r->info != NULL);
	sprintf(fname,"%s.data",name);
	r->data = fopen(fname,mode);
	assert(r->data != NULL);
	sprintf(fname,"%s.ovflow",name);
	r->ovflow = fopen(fname,mode);
	assert(r->ovflow != NULL);
	// Naughty: assumes Count and Offset are the same size
	int n = fread(r, sizeof(Count), 5, r->info);
	assert(n == 5);
	n = fread(r->cv, sizeof(ChVecItem), MAXCHVEC, r->info);
	assert(n == MAXCHVEC);
	r->mode = (mode[0] == 'w' || mode[1] =='+') ? 'w' : 'r';
	return r;
}

// release files and descriptor for an open relation
// copy latest information to .info file

void closeRelation(Reln r)
{
	// make sure updated global data is put in info
	// Naughty: assumes Count and Offset are the same size
	if (r->mode == 'w') {
		fseek(r->info, 0, SEEK_SET);
		// write out core relation info (#attr,#pages,d,sp)
		int n = fwrite(r, sizeof(Count), 5, r->info);
		assert(n == 5);
		// write out choice vector
		n = fwrite(r->cv, sizeof(ChVecItem), MAXCHVEC, r->info);
		assert(n == MAXCHVEC);
	}
	fclose(r->info);
	fclose(r->data);
	fclose(r->ovflow);
	free(r);
}

// insert a new tuple into a relation
// returns index of bucket where inserted
// - index always refers to a primary data page
// - the actual insertion page may be either a data page or an overflow page
// returns NO_PAGE if insert fails completely
// TODO: include splitting and file expansion
///by myself
PageID addToRelationPage(Reln r, PageID p, Tuple t) {


    Page pg = getPage(r->data,p);       // 第一次从data Page去get
    // 如果有位置， 就插入
    if (addToPage(pg,t) == OK) {
        putPage(r->data,p,pg);
        return p;
    }
    // primary data page full
    // 获取overflow page
    if (pageOvflow(pg) == NO_PAGE) {    // 如果没有overflow page， 则添加一个overflowpage
        // add first overflow page in chain
        PageID newp = addPage(r->ovflow);
        pageSetOvflow(pg,newp);         // 原来的dataPage上面添加了一个overflow page
        putPage(r->data,p,pg);          // 插入到文件里面，
        Page newpg = getPage(r->ovflow,newp);           // 重新get一下， 拿出来
        // can't add to a new page; we have a problem
        if (addToPage(newpg,t) != OK) return NO_PAGE;       // 把tuple再添加进去
        putPage(r->ovflow,newp,newpg);
        return p;
    }
    else {
        // 如果有overflow Page
        // scan overflow chain until we find space
        // worst case: add new ovflow page at end of chain
        Page ovpg, prevpg = NULL;
        PageID ovp, prevp = NO_PAGE;
        ovp = pageOvflow(pg);
//        如果page后面有overflow, 则继续往后, 一直到没有overflow
        while (ovp != NO_PAGE) {
            ovpg = getPage(r->ovflow, ovp); // 获取page
            //尝试把tuple添加进page，
            if (addToPage(ovpg,t) != OK) {          // 如果添加tuple不成功
                prevp = ovp; prevpg = ovpg;
                ovp = pageOvflow(ovpg);
            }
            else {
                if (prevpg != NULL) free(prevpg);
                putPage(r->ovflow,ovp,ovpg);
                return p;
            }
        }
        // all overflow pages are full; add another to chain
        // at this point, there *must* be a prevpg
        assert(prevpg != NULL);
        // make new ovflow page
        PageID newp = addPage(r->ovflow);
        // insert tuple into new page
        Page newpg = getPage(r->ovflow,newp);
        if (addToPage(newpg,t) != OK) return NO_PAGE;
        putPage(r->ovflow,newp,newpg);
        // link to existing overflow chain
        pageSetOvflow(prevpg,newp);
        putPage(r->ovflow,prevp,prevpg);
        r->ntups++;
        return p;
    }
    return NO_PAGE;
}
PageID addToRelation(Reln r, Tuple t)
{
	Bits h, p;
	// char buf[MAXBITS+1];
	h = tupleHash(r,t);
	if (r->depth == 0) {
        p = 0;              // p所在的位置， page_id 一开始是1
    }
	else {
		p = getLower(h, r->depth);
		if (p < r->sp) p = getLower(h, r->depth+1);
	}

    PageID result = addToRelationPage(r,p,t);
    //插入成功
    if (result != NO_PAGE) {
        r->ntups ++;
        Count c = 1024/(10* r->nattrs);         //取mod
        if(r->ntups % c == 0) {
            //split algorithm pseudo algorithm
//            newp = sp + 2^d;
//            oldp = sp;
//            for all tuples t in P[oldp] and its overflows {
//                p = bits(d+1, hahs(t.k));
//            }
//            if (p == newp):
//                add tuple t to bucket[newp]
//
//            else:
//                add tuple to bucket[oldp]
//            sp++
//            if (sp == 2^d){
//                d++;
//                sp=0;
//            }
            Splitting(r);
        }
        if(r->sp == new_pow(2,r->depth)) {
            r->depth++;
            r->sp = 0;
        }
    }
    return result;
}

Offset new_pow(int i, Count depth) {
    int j;
    int val = 1;
    for(j = 0; j < depth; j++) {
        val *= 2;
    }

    return (unsigned int)val;
}


// 000->a,b,c-> d, e, f
//Page p = getPage(r->data,pid);        // 当前dataPage
//page blakPage = new Page()
//如果原来p有overflow page，绑定就可以了
//PageID oldOverflowPageId = pageOvflow(p)
//pagesetOvflow(blankPage, ldOverflowPageID);
//putPage(r->data, p, blankPage)    再写回Pag，在这里就移除了， 给了一个空的blackPage
// p的数据都在内存的
//遍历page里面的tuple：t
//hash_t = hash(t)
//temp_pid = getlower(hash_t, d)
//addToRelationPage(r,pid,t)
//while pageOvflow(p) != NO_PAGE
//....
void Splitting(Reln r) {
    PageID newp = addPage(dataFile(r));
//    PageID new_pageid = addPage(dataFile(r));
    r->npages++;
    PageID oldp = r->sp;
    Bits hash_t;
    Bits tmp_pid;
    Tuple cur_t;
    int i;
    //dataPage
    Page emptyPage = newPage();
    Page oldpg = getPage(r->data,oldp);
    PageID oldOverflowPageId = pageOvflow(oldpg);
    pageSetOvflow(emptyPage, oldOverflowPageId);
    putPage(r->data, oldp, emptyPage);
    char* data_p = pageData(oldpg);  // 获得tuple
    Count tuple_num = pageNTuples(oldpg);
//
//    //遍历tuple
    for (i = 0; i < tuple_num; i++) {
        cur_t = data_p;
        hash_t = tupleHash(r, cur_t);
        tmp_pid = getLower(hash_t, r->depth+1);
        if (tmp_pid == newp) {
            addToRelationPage(r, newp, cur_t); //new page
        } else {
            addToRelationPage(r, oldp, cur_t);  //old page
        }
        data_p += strlen(data_p) + 1;
    }
    //overflow
    Page prev_page = oldpg;
    PageID ovp;
//    Tuple cur;
    while (pageOvflow(prev_page) != NO_PAGE ){
        ovp = pageOvflow(prev_page);
        // if tuple is null
        Page ovpg = getPage(r->ovflow, ovp); // 获取page
        PageID curp = pageOvflow(ovpg);
        Page emptyPage = newPage();
        pageSetOvflow(emptyPage, curp); // 设置空页的ovflow为curp
        putPage(r->ovflow, ovp, emptyPage);
        data_p = pageData(ovpg);  // 获得tuple
        Count tuple_num = pageNTuples(ovpg);
        for (i = 0; i < tuple_num; i++) {
            cur_t = data_p;
            hash_t = tupleHash(r, cur_t);
            tmp_pid = getLower(hash_t, r->depth+1);
            if (tmp_pid == newp) {
                addToRelationPage(r, newp, cur_t); //new page
            } else {
                addToRelationPage(r, oldp, cur_t);  //old page
            }
            data_p += strlen(data_p) + 1;
        }
        prev_page = ovpg;
    }

    r->sp++;
}

// external interfaces for Reln data

FILE *dataFile(Reln r) { return r->data; }
FILE *ovflowFile(Reln r) { return r->ovflow; }
Count nattrs(Reln r) { return r->nattrs; }
Count npages(Reln r) { return r->npages; }
Count ntuples(Reln r) { return r->ntups; }
Count depth(Reln r)  { return r->depth; }
Count splitp(Reln r) { return r->sp; }
ChVecItem *chvec(Reln r)  { return r->cv; }


// displays info about open Reln

void relationStats(Reln r)
{
	printf("Global Info:\n");
	printf("#attrs:%d  #pages:%d  #tuples:%d  d:%d  sp:%d\n",
	       r->nattrs, r->npages, r->ntups, r->depth, r->sp);
	printf("Choice vector\n");
	printChVec(r->cv);
	printf("Bucket Info:\n");
	printf("%-4s %s\n","#","Info on pages in bucket");
	printf("%-4s %s\n","","(pageID,#tuples,freebytes,ovflow)");
	for (Offset pid = 0; pid < r->npages; pid++) {
		printf("[%2d]  ",pid);
		Page p = getPage(r->data, pid);
		Count ntups = pageNTuples(p);
		Count space = pageFreeSpace(p);
		Offset ovid = pageOvflow(p);
		printf("(d%d,%d,%d,%d)",pid,ntups,space,ovid);
		free(p);
		while (ovid != NO_PAGE) {
			Offset curid = ovid;
			p = getPage(r->ovflow, ovid);
			ntups = pageNTuples(p);
			space = pageFreeSpace(p);
			ovid = pageOvflow(p);
			printf(" -> (ov%d,%d,%d,%d)",curid,ntups,space,ovid);
			free(p);
		}
		putchar('\n');
	}
}
