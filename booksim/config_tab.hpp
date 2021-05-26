#define STR 257
#define NUM 258
#define FNUM 259
typedef union {
  char         *name;
  unsigned int num;
  double       fnum;
} YYSTYPE;
extern YYSTYPE configlval;
