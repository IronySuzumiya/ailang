#pragma once
#ifndef MARSHAL_H
#define MARSHAL_H

#include "utils.h"

#define TYPE_NULL               '0'
#define TYPE_NONE               'N'
#define TYPE_FALSE              'F'
#define TYPE_TRUE               'T'
#define TYPE_STOPITER           'S'
#define TYPE_ELLIPSIS           '.'
#define TYPE_INT                'i'
#define TYPE_INT64              'I'
#define TYPE_FLOAT              'f'
#define TYPE_BINARY_FLOAT       'g'
#define TYPE_COMPLEX            'x'
#define TYPE_BINARY_COMPLEX     'y'
#define TYPE_LONG               'l'
#define TYPE_STRING             's'
#define TYPE_INTERNED           't'
#define TYPE_STRINGREF          'R'
#define TYPE_TUPLE              '('
#define TYPE_LIST               '['
#define TYPE_DICT               '{'
#define TYPE_CODE               'c'
#define TYPE_UNICODE            'u'
#define TYPE_UNKNOWN            '?'
#define TYPE_SET                '<'
#define TYPE_FROZENSET          '>'

#define AFERR_OK                0
#define AFERR_UNMARSHALLABLE    1
#define AFERR_NESTEDTOODEEP     2
#define AFERR_NOMEMORY          3

#define w_byte(c, p)                                    \
    if (((p)->fp)) putc((c), (p)->fp);                  \
    else if ((p)->ptr != (p)->end) *(p)->ptr++ = (c);   \
    else w_more(c, p)

#define rs_byte(p) (((p)->ptr < (p)->end) ? (unsigned char)*(p)->ptr++ : EOF)

#define r_byte(p) ((p)->fp ? getc((p)->fp) : rs_byte(p))

#define SAFE_DOWNCAST(VALUE, WIDE, NARROW)  \
    (assert((WIDE)(NARROW)(VALUE) == (VALUE)), (NARROW)(VALUE))

typedef unsigned short mode_t;
typedef __time64_t time_t;

#endif