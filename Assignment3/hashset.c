#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn) {
			assert(elemSize > 0 && numBuckets > 0 && hashfn != NULL && comparefn != NULL);
			h->elemSize = elemSize;
			h->numOfBuckets = numBuckets;
			h->numOfElems = 0;
			h->hashfunction = hashfn;
			h->comparefunction = comparefn;
			h->freefunction = freefn;
			h->buckets = malloc(numBuckets * sizeof(vector));
			for (int i = 0; i < numBuckets; i++) {
				VectorNew(&h->buckets[i], elemSize, freefn, 0);
			}
}

void HashSetDispose(hashset *h) {
	for (int i = 0; i < h->numOfBuckets; i++) VectorDispose(&h->buckets[i]);
	free(h->buckets);
	h->numOfBuckets = 0;
	h->numOfElems = 0;
}

int HashSetCount(const hashset *h) { 
	return h->numOfElems; 
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData) {
	assert(mapfn != NULL);
	for (int i = 0; i < h->numOfBuckets; i++) {
		for (int j = 0; j < VectorLength(&h->buckets[i]); j++) mapfn(VectorNth(&h->buckets[i], j), auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr) {
	assert(elemAddr != NULL);
	int ind = h->hashfunction(elemAddr, h->numOfBuckets);
	assert(ind >= 0 && ind < h->numOfBuckets);
	void* check = HashSetLookup(h, elemAddr);
	if (check == NULL) {
		VectorAppend(&h->buckets[ind], elemAddr);
		h->numOfElems++;
	} else {
		memcpy(check, elemAddr, h->elemSize);
	}
}

void *HashSetLookup(const hashset *h, const void *elemAddr) { 
	assert(elemAddr != NULL);
	int ind = h->hashfunction(elemAddr, h->numOfBuckets);
	assert(ind >= 0 && ind < h->numOfBuckets);
	for (int j = 0; j < VectorLength(&h->buckets[ind]); j++) {
		if (!h->comparefunction(VectorNth(&h->buckets[ind], j), elemAddr)) {
			return VectorNth(&h->buckets[ind], j);
		}
	}
	return NULL;
}
