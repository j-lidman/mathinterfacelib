#ifndef MATWLIB_CONST_HDR
#define MATWLIB_CONST_HDR

#define MATWLIB_USE_CPP0x_FUNC_CALLBACK
//#define MATWLIB_DEBUG
#define MATWLIB_PREFER_STATIC_ERRORS

namespace MatWLib {
   //Types
     //ARRAY = "(" ARRAY0 ... ARRAYN-1 ")" | MATRIX
     //MATRIX = "[" MATRIX00 ... MATRIX0N-1 ";" ... ";" MATRIXN-10 ... MATRIXN-1N-1 "]" | BASIC
     //BASIC = ? | INT | DOUBLE | STRING | CELL | STRUCT
     typedef enum {
         TYPE_UNKNOWN,

         TYPE_INT,
         TYPE_DOUBLE,
         TYPE_STRING,
         TYPE_CELL,
         TYPE_STRUCT,
     } TYPE;

     const char *toString(TYPE t);
};

#endif
