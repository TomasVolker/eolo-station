#ifndef EOLO_STRING_UTILS_H
#define EOLO_STRING_UTILS_H

#define PARSE_ERROR (-1)

#include "cmsis.h"

uint32_t stringLength(const char * string);

Bool isEmpty(const char * string);

Bool isDigit(char c);

int8_t parseDigit(char c);

const char * parseInt(const char * string, int32_t* result);

//Devuelve el puntero despues del prefijo o NULL
const char * stringBeginsWith(const char * string, const char * prefix);

Bool stringEquals(const char * string1, const char * string2);

void stringCopy(const char * origin, char * dest);

void stringCopyMax(const char * origin, char * dest, uint32_t maxLength);

int i(float value);

int d(float value, int digits);

void bytesCopy(const char * origin, char * dest, int size);

Bool stringEqualsIgnoreWhiteChars(const char * string1, const char * string2);

Bool isWhiteChar(char c);


#endif //EOLO_STRING_UTILS_H
