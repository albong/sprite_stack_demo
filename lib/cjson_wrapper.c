// Copyright (c) 2019 Alex Bongiovanni

#include "cjson_wrapper.h"
#include "../src/logging.h"

int keyExistsCJSON(cJSON *root, const char *keyName){
    cJSON *object = cJSON_GetObjectItem(root, keyName);
    if (object == NULL){
        return 0;
    } else {
        return 1;
    }
}

int cjson_readInt(cJSON *root, const char *keyName, int *errorCount){
    int result = 0;
    cJSON *object = cJSON_GetObjectItem(root, keyName);
    if (object == NULL){
        LOG_ERR("No key named %s in the current JSON", keyName);
        (*errorCount)++;
    } else if (!cJSON_IsNumber(object)){
        LOG_ERR("Value for key %s is not a number in current JSON", keyName);
        (*errorCount)++;
    } else {
        result = object->valueint;
    }

    return result; 
}

double cjson_readDouble(cJSON *root, const char *keyName, int *errorCount){
    double result = 0;
    cJSON *object = cJSON_GetObjectItem(root, keyName);
    if (object == NULL){
        LOG_ERR("No key named %s in the current JSON", keyName);
        (*errorCount)++;
    } else if (!cJSON_IsNumber(object)){
        LOG_ERR("Value for key %s is not a number in current JSON", keyName);
        (*errorCount)++;
    } else {
        result = object->valuedouble;
    }

    return result;
}

float cjson_readFloat(cJSON *root, const char *keyName, int *errorCount){
    return (float) cjson_readDouble(root, keyName, errorCount);
}

int cjson_readBoolean(cJSON *root, const char *keyName, int *errorCount){
    int result = 0;
    cJSON *object = cJSON_GetObjectItem(root, keyName);
    if (object == NULL){
        LOG_ERR("No key named %s in the current JSON", keyName);
        (*errorCount)++;
    } else if (!cJSON_IsBool(object)){
        LOG_ERR("Value for key %s is not a boolean in current JSON", keyName);
        (*errorCount)++;
    } else {
        result = !!cJSON_IsTrue(object); //the !! flips to a 1 or 0 in case its not
    }

    return result;
}

cJSON *cjson_readArray(cJSON *root, const char *keyName, int *errorCount){
    cJSON *result = NULL;
    cJSON *object = cJSON_GetObjectItem(root, keyName);
    if (object == NULL){
        LOG_ERR("No key named %s in the current JSON", keyName);
        (*errorCount)++;
    } else if (!cJSON_IsArray(object)){
        LOG_ERR("Value for key %s is not an array in current JSON", keyName);
        (*errorCount)++;
    } else {
        result = object;
    }

    return result;
}

int cjson_readArrayLength(cJSON *root, const char *keyName, int *errorCount){
    int result = 0;
    cJSON *object = cJSON_GetObjectItem(root, keyName);
    if (object == NULL){
        LOG_ERR("No key named %s in the current JSON", keyName);
        (*errorCount)++;
    } else if (!cJSON_IsArray(object)){
        LOG_ERR("Value for key %s is not an array in current JSON", keyName);
        (*errorCount)++;
    } else {
        result = cJSON_GetArraySize(object);
    }

    return result;
}

cJSON *cjson_readObject(cJSON *root, const char *keyName, int *errorCount){
    cJSON *result = NULL;
    cJSON *object = cJSON_GetObjectItem(root, keyName);
    if (object == NULL){
        LOG_ERR("No key named %s in the current JSON", keyName);
        (*errorCount)++;
    } else if (!cJSON_IsObject(object)){
        LOG_ERR("Value for key %s is not an object in current JSON", keyName);
        (*errorCount)++;
    } else {
        result = object;
    }

    return result;
}

char *cjson_readString(cJSON *root, const char *keyName, int *errorCount){
    char *result = NULL;
    cJSON *object = cJSON_GetObjectItem(root, keyName);
    if (object == NULL){
        LOG_ERR("No key named %s in the current JSON", keyName);
        (*errorCount)++;
    } else if (!cJSON_IsString(object)){
        LOG_ERR("Value for key %s is not a string in current JSON", keyName);
        (*errorCount)++;
    } else {
        result = object->valuestring;
    }

    return result;
}

cJSON *cjson_readObjectFromArray(cJSON *root, const int index, int *errorCount){
    cJSON *result = NULL;
    cJSON *object = cJSON_GetArrayItem(root, index);
    if (object == NULL){
        LOG_ERR("Invalid JSON array index %d", index);
        (*errorCount)++;
    } else if (!cJSON_IsObject(object)){
        LOG_ERR("Value for index %d is not an object in current JSON", index);
        (*errorCount)++;
    } else {
        result = object;
    }

    return result;
}

cJSON *cjson_readArrayFromArray(cJSON *root, const int index, int *errorCount){
    cJSON *result = NULL;
    cJSON *object = cJSON_GetArrayItem(root, index);
    if (object == NULL){
        LOG_ERR("Invalid JSON array index %d", index);
        (*errorCount)++;
    } else if (!cJSON_IsArray(object)){
        LOG_ERR("Value for index %d is not an array in current JSON", index);
        (*errorCount)++;
    } else {
        result = object;
    }

    return result;
}

int cjson_readIntFromArray(cJSON *root, const int index, int *errorCount){
    int result = 0;
    cJSON *object = cJSON_GetArrayItem(root, index);
    if (object == NULL){
        LOG_ERR("Invalid JSON array index %d", index);
        (*errorCount)++;
    } else if (!cJSON_IsNumber(object)){
        LOG_ERR("Value for index %d is not a number in current JSON", index);
        (*errorCount)++;
    } else {
        result = object->valueint;
    }

    return result;
}

double cjson_readDoubleFromArray(cJSON *root, const int index, int *errorCount){
    double result = 0;
    cJSON *object = cJSON_GetArrayItem(root, index);
    if (object == NULL){
        LOG_ERR("Invalid JSON array index %d", index);
        (*errorCount)++;
    } else if (!cJSON_IsNumber(object)){
        LOG_ERR("Value for index %d is not a number in current JSON", index);
        (*errorCount)++;
    } else {
        result = object->valuedouble;
    }

    return result;
}

float cjson_readFloatFromArray(cJSON *root, const int index, int *errorCount){
    return (float) cjson_readDoubleFromArray(root, index, errorCount);
}

char *cjson_readStringFromArray(cJSON *root, const int index, int *errorCount){
    char *result = NULL;
    cJSON *object = cJSON_GetArrayItem(root, index);
    if (object == NULL){
        LOG_ERR("Invalid JSON array index %d", index);
        (*errorCount)++;
    } else if (!cJSON_IsString(object)){
        LOG_ERR("Value for index %d is not a string in current JSON", index);
        (*errorCount)++;
    } else {
        result = object->valuestring;
    }

    return result;
}

int cjson_getArrayLength(cJSON *root, int *errorCount){
    int result = 0;
    if (!cJSON_IsArray(root)){
        LOG_ERR("Requested array size of something not an array in JSON");
        (*errorCount)++;
    } else {
        result = cJSON_GetArraySize(root);
    }

    return result;
}
