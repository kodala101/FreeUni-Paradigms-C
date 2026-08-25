#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation) {
    if (initialAllocation == 0) initialAllocation = 4;
    assert(elemSize > 0 && initialAllocation > 0);
    v->alocLen = initialAllocation;
    v->loglen = 0;
    v->freeFunction = freeFn;
    v->elemSize = elemSize;
    v->startOfElems = malloc(elemSize * initialAllocation);
}

void VectorDispose(vector *v) {
    if (v->freeFunction != NULL) {  //intebis vectori ro iyos rato ar mwirdeba washla?
        for (int i = 0; i < v->loglen; i++) {
            v->freeFunction((char*)v->startOfElems + i * v->elemSize);
        }
    }
    free(v->startOfElems);
    v->loglen = 0;
    v->alocLen = 0;
    //v->startOfElems = NULL;
}

int VectorLength(const vector *v) { 
    return v->loglen; 
}

void* VectorNth(const vector *v, int position) {
    assert(position >= 0 && position < v->loglen);
    return ((char*)v->startOfElems + position * v->elemSize);
    return NULL; //!!!!
}

void VectorReplace(vector *v, const void *elemAddr, int position) {
    assert(position >= 0 && position < v->loglen);
    char* elemToChange = (char*)v->startOfElems + position * v->elemSize;
    if (v->freeFunction != NULL) v->freeFunction(elemToChange);
    memcpy(elemToChange, elemAddr, v->elemSize); //imis shemowmeba ar unda baitebis raodenoba sworia tu ara?
}

void VectorInsert(vector *v, const void *elemAddr, int position) {
    assert(position >= 0 && position <= v->loglen);
    if (v->alocLen == v->loglen) {
        v->startOfElems = realloc(v->startOfElems, 2 * v->alocLen * v->elemSize);
        v->alocLen *= 2;
    }
    char* elemToShift = (char*)v->startOfElems + position * v->elemSize;
    memmove(elemToShift + v->elemSize, elemToShift, v->elemSize * (v->loglen - position));
    memcpy(elemToShift, elemAddr, v->elemSize); //memccpy raia
    v->loglen++;
} 

void VectorAppend(vector *v, const void *elemAddr) {
    if (v->alocLen == v->loglen) {
        v->startOfElems = realloc(v->startOfElems, 2 * v->alocLen * v->elemSize);
        v->alocLen *= 2;
    }
    memcpy((char*)v->startOfElems + v->loglen * v->elemSize, elemAddr, v->elemSize);
    v->loglen++;
}

void VectorDelete(vector *v, int position) {  
    assert(position >= 0 && position < v->loglen);
    char* elemToDelete = (char*)v->startOfElems + position * v->elemSize;
    if (v->freeFunction != NULL) v->freeFunction(elemToDelete);
    memmove(elemToDelete, elemToDelete + v->elemSize, v->elemSize * (v->loglen - position - 1));
    v->loglen--;
}

void VectorSort(vector *v, VectorCompareFunction compare) {
    assert(compare != NULL);
    qsort(v->startOfElems, v->loglen, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData) {
    assert(mapFn != NULL);
    for (int i = 0; i < v->loglen; i++) {
        mapFn((char*)v->startOfElems + i * v->elemSize, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted) { 
    assert(startIndex >= 0 && startIndex < v->loglen && searchFn != NULL && key != NULL);
    if (isSorted) {
        void* poi = bsearch(key, (char*)v->startOfElems + startIndex * v->elemSize, v->loglen - startIndex, v->elemSize, searchFn);
        if (poi == NULL) return kNotFound;
        return ((char*)poi - (char*)v->startOfElems)/v->elemSize;
    } else {
        for (int i = startIndex; i < v->loglen; i++) {
            if (!searchFn((char*)v->startOfElems + i * v->elemSize, key)) return i;
        }
    }
    return kNotFound; 
} 