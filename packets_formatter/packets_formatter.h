#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#define NELEMS(array) (sizeof (array) / sizeof (array [0]))

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

enum{
    BUF_SIZE = 256,
};

int recive(int network);
