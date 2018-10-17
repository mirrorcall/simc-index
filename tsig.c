// tsig.c ... functions on Tuple Signatures (tsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"

// helper function that generates codeword for tuple signatures

static Bits codeword(char *attr, int m, int k)
{
	Count nbits = 0;
	Bits cword  = newBits(m);
	srandom(hash_any(attr, strlen(attr)));
	while (nbits < k) {
		int i = random() % m;
		if (!bitIsSet(cword, i)) {
			setBit(cword, i);
			nbits++;
		}
	}

	return cword;
}

// make a tuple signature

Bits makeTupleSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
	Bits tsig = newBits(tsigBits(r));
	char **attrs = tupleVals(r, t);
	int i;

	for (i = 0; i < nAttrs(r); i++) {
		if (attrs[i][0] == '?')		// ? makes no contribution to descriptor
			continue;
		Bits cw = codeword(attrs[i], tsigBits(r), codeBits(r));
		orBits(tsig, cw);
		freeBits(cw);
	}
	freeVals(attrs, nAttrs(r));

	return tsig;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q)
{
	assert(q != NULL);
	//TODO

	Bits qsig = makeTupleSig(q->rel, q->qstring);
	unsetAllBits(q->pages);

	PageID tgpid;
	Offset pos;
	Page tgp;
	Bits tsig  = newBits(8 * tsigsize(q->rel));
	Bits pages = newBits(nPages(q->rel));

	for (tgpid = 0; tgpid < nTsigPages(q->rel); tgpid++) {
		tgp = getPage(tsigFile(q->rel), tgpid);
		for (pos = 0; pos < nTsigs(q->rel); pos++) {
			getBits(tgp, pos, tsig);
			showBits(qsig); printf("\n");
			showBits(tsig); printf("\n\n");
			if (isSubset(qsig, tsig))
				setBit(pages, tgpid);
		}
	}

	q->pages = pages;

	// The printf below is primarily for debugging
	// Remove it before submitting this function
	printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}
