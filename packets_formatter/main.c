#include "packets_formatter.h"

int unpack_type0(int n, uchar *buf);
int unpack_type1(int n, uchar *buf);
int unpack_type2(int n, uchar *buf);

int (*unpackfn[])(int, uchar *) = {
    unpack_type0,
    unpack_type1,
    unpack_type2,
};

int read_packet(int a, uchar *buf, size_t buf_size){
    return 1;
}

int pack(uchar *buf, char *fmt, ...){
    va_list va;
    char *p;
    uchar *bp;
    ushort s;
    ulong l;

    bp = buf;
    va_start(va, fmt);

    for (p = fmt; *p != '\0'; p++){
        switch (*p) {
            case 'c':{
                *bp++ = va_arg(va, int);
                break;
            }
            case 's':{
                s = va_arg(va, int);
                *bp++ = s >> 8;
                *bp++ = s;
                break;
            }
            case 'l':{
                l = va_arg(va, int);
                *bp++ = l >> 24;
                *bp++ = l >> 16;
                *bp++ = l >> 8;
                *bp++ = l;

                break;
            }
            default:{
                va_end(va);
                return -1;
            }
        }
    }

    va_end(va);
    return bp - buf;
}

int pack_type1(uchar *buf, ushort count, uchar val, ulong data){
    return pack(buf, "cscl", 0x01, count, val, data);
}

int unpack(uchar *buf, char *fmt, ...){
    va_list va;
    char *p;
    uchar *bp, *pc;
    ushort *ps;
    ulong *pl;

    bp = buf;
    va_start(va, fmt);

    for (p = fmt; *p != '\0'; p++){
        switch (*p) {
            case 'c':{
                pc = va_arg(va, uchar*);
                *pc = *bp++;
                break;
            }
            case 's':{
                ps = va_arg(va, ushort*);
                *ps = *bp++ << 8;
                *ps |= *bp++;
                break;
            }
            case 'l':{
                pl = va_arg(va, ulong*);
                *pl = *bp++ << 24;
                *pl |= *bp++ << 16;
                *pl |= *bp++ << 8;
                *pl |= *bp++;

                break;
            }
            default:{
                va_end(va);
                return -1;
            }
        }
    }

    va_end(va);
    return bp - buf;
}

int process_type2(ushort count, ulong dw1, ulong dw2){
    return 1;
}

int unpack_type2(int n, uchar *buf){
    uchar c;
    ushort count;
    ulong dw1, dw2;

    if ((unpack(buf, "csll", &c, &count, &dw1, &dw2)) != n){
        return -1;
    }

    assert(c == 0x02);
    return process_type2(count, dw1, dw2);
}

int recive(int network){
    uchar type, buf[BUF_SIZE];
    int n;

    while ((n = read_packet(network, buf, BUF_SIZE)) > 0){
        type = buf[0];
        if (type > NELEMS(unpackfn)){
            fprintf(stderr, "invalid packets type 0x%x", type);
            return -1;
        }

        if((*unpackfn[type])(n, buf) < 0){
            fprintf(stderr, "error unpacking packet type 0x%x", type);
            return -1;
        }
    }

    return 1;
}

