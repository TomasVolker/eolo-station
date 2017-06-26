
#include "string_utils.h"

uint32_t stringLength(const char * string){

    uint32_t result = 0;

    while(*string != '\0'){
        result++;
        string++;
    }

    return result;
}

Bool isEmpty(const char * string) {
    return (Bool) (*string == '\0');
}

Bool isDigit(char c) {
    return (Bool) ('0' <= c && c <= '9');
}

Bool isWhiteChar(char c){
    switch(c){
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return TRUE;
        default:
            return FALSE;
    }
}

int8_t parseDigit(char c){
    return (int8_t) (isDigit(c) ? c - '0' : -1);
}

const char * parseInt(const char * string, int32_t* result) {

    if(!isDigit(*string)) {
        *result = PARSE_ERROR;
        return string;
    }

    int32_t aux = 0;

    while(isDigit(*string)) {

        aux *= 10;

        aux += parseDigit(*string);

        string++;
    }

    *result = aux;

    return string;
}

const char * stringBeginsWith(const char * string, const char * prefix) {

    if (string == NULL || prefix == NULL) {
        return NULL;
    }

    while(*prefix != '\0') {

        if(*prefix != *string){
            return NULL;
        }

        prefix++;
        string++;
    }

    return string;
}

Bool stringEquals(const char * string1, const char * string2) {

    if (string1 == NULL || string2 == NULL) {
        return FALSE;
    }

    while(*string1 != '\0' && *string2 != '\0') {

        if(*string1 != *string2){
            return FALSE;
        }

        string1++;
        string2++;
    }

    return (Bool) (*string1 == *string2);
}

Bool stringEqualsIgnoreWhiteChars(const char * string1, const char * string2) {

    if (string1 == NULL || string2 == NULL) {
        return FALSE;
    }

    while(*string1 != '\0' && *string2 != '\0') {

        if(isWhiteChar(*string1)){
            string1++;
            continue;
        }

        if(isWhiteChar(*string2)){
            string2++;
            continue;
        }

        if(*string1 != *string2){
            return FALSE;
        }

        string1++;
        string2++;
    }

    while(isWhiteChar(*string1)){
        string1++;
    }

    while(isWhiteChar(*string2)){
        string2++;
    }

    return (Bool) (*string1 == *string2);
}

void stringCopy(const char * origin, char * dest){

    while(*origin != '\0') {

        *dest = *origin;

        origin++;
        dest++;
    }

    *dest = '\0';
}

void stringCopyMax(const char * origin, char * dest, uint32_t maxLength){

    uint32_t count = 0;

    while(*origin != '\0' && count < maxLength) {

        *dest = *origin;

        origin++;
        dest++;
        count++;
    }

    *dest = '\0';
}

int i(float value){
    return (int) value;
}

int d(float value, int digits){

    if(value < 0){
        value = -value;
    }

    float reminder = value - i(value);

    int i;
    for(i = 0; i<digits; ++i){
        reminder *= 10;
    }

    return (int) reminder;
}

void bytesCopy(const char * origin, char * dest, int size){

    int i;
    for(i = 0; i<size; ++i){
        *dest = *origin;

        origin++;
        dest++;
    }

}