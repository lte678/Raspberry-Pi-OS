#ifndef TYPES_H
#define TYPES_H


/* Fixed-size types, underlying types depend on word size and compiler.  */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;

typedef uint8_t bool_t;
#define true  1
#define false 0


#endif // TYPES_H