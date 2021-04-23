// Copyright (c) 2019 Alex Bongiovanni

#ifndef CJSON_WRAPPER_H
#define CJSON_WRAPPER_H

#include "cJSON/cJSON.h"

/*
 * This is a wrapper for cJSON.  cJSON is a nice library, but I want to 
 * streamline the error checking and reading to reduce boilerplate overhead. 
 */

int keyExistsCJSON(cJSON *root, const char *keyName);
//Pass a pointer to an int for the error counter.  If there is an error, the
//value will be incremented.  Thus if you start with 0, run a bunch of reads
//with the same error counter, you can check the number of fails at the end.
//On failure, it logs an error and it returns NULL or 0, which MAY not cause 
//breakage of later code, so please be sure to check errors and respond
//accordingly.
int cjson_readInt(cJSON *root, const char *keyName, int *errorCount);
int cjson_readIntFromArray(cJSON *root, const int index, int *errorCount);

double cjson_readDouble(cJSON *root, const char *keyName, int *errorCount);
double cjson_readDoubleFromArray(cJSON *root, const int index, int *errorCount);

float cjson_readFloat(cJSON *root, const char *keyName, int *errorCount);
float cjson_readFloatFromArray(cJSON *root, const int index, int *errorCount);

int cjson_readBoolean(cJSON *root, const char *keyName, int *errorCount);

cJSON *cjson_readArray(cJSON *root, const char *keyName, int *errorCount);
cJSON *cjson_readArrayFromArray(cJSON *root, const int index, int *errorCount);
int cjson_readArrayLength(cJSON *root, const char *keyName, int *errorCount);
int cjson_getArrayLength(cJSON *root, int *errorCount);

cJSON *cjson_readObject(cJSON *root, const char *keyName, int *errorCount);
cJSON *cjson_readObjectFromArray(cJSON *root, const int index, int *errorCount);

char *cjson_readString(cJSON *root, const char *keyName, int *errorCount);
char *cjson_readStringFromArray(cJSON *root, const int index, int *errorCount);

#endif
