#include "../ailang.h"

static FILE *open_exclusive(char *filename, mode_t mode);
static void w_more(char c, AicFile *p);
static void w_long(long x, AicFile *p);
static void write_long_to_file(long x, FILE *fp);
static void w_string(const char *s, ssize_t n, AicFile *p);
static void w_pstring(const char *s, ssize_t n, AicFile *p);
static void write_string_to_file(const char *x, ssize_t n, FILE *fp);
static void w_object(AiObject *v, AicFile *p);
static void write_object_to_file(AiObject *x, FILE *fp);
static AiObject *write_object_to_string(AiObject *x);
static ssize_t r_string(char *s, ssize_t n, AicFile *p);
static long r_long(AicFile *p);
static long read_long_from_file(FILE *fp);
static AiObject *r_object(AicFile *p);
static AiObject *read_object_from_file(FILE *fp);
static AiObject *read_object_from_cstring(char *s, ssize_t len);

void write_compiled_module(AiCodeObject *co, char *path, struct stat *srcstat, time_t mtime) {
    FILE *fp;
    mode_t mode = srcstat->st_mode & S_IEXEC;
    mode |= _S_IWRITE;

    fp = open_exclusive(path, mode);
    if (!fp) {
        FATAL_ERROR("cannot create .aic file %s\n", path);
        return;
    }
    write_long_to_file(AI_MAGIC, fp);
    write_long_to_file((long)mtime, fp);
    write_object_to_file((AiObject *)co, fp);

    fflush(fp);
    fclose(fp);
}

FILE *open_exclusive(char *filename, mode_t mode) {
    int fd;
    _unlink(filename);
    fd = open(filename, O_EXCL | O_CREAT | O_WRONLY | O_TRUNC | O_BINARY);
    return fd < 0 ? NULL : _fdopen(fd, "wb");
}

void w_more(char c, AicFile *p) {
    ssize_t size, newsize;
    if (p->str == NULL)
        return;
    size = STRING_LEN(p->str);
    newsize = size + size + 1024;
    if (newsize > 32 * 1024 * 1024) {
        /* 12.5% overallocation */
        newsize = size + (size >> 3);
    }
    string_resize((AiStringObject **)&p->str, newsize);
    p->ptr = STRING_AS_CSTRING(p->str) + size;
    p->end = STRING_AS_CSTRING(p->str) + newsize;
    *p->ptr++ = c;
}

void w_long(long x, AicFile *p) {
    w_byte((char)(x & 0xff), p);
    w_byte((char)((x >> 8) & 0xff), p);
    w_byte((char)((x >> 16) & 0xff), p);
    w_byte((char)((x >> 24) & 0xff), p);
}

void write_long_to_file(long x, FILE *fp) {
    AicFile af;
    af.fp = fp;
    w_long(x, &af);
}

void w_string(const char *s, ssize_t n, AicFile *p) {
    if (p->fp) {
        fwrite(s, 1, (size_t)n, p->fp);
    }
    else {
        while (--n >= 0) {
            w_byte(*s, p);
            ++s;
        }
    }
}

void w_pstring(const char *s, ssize_t n, AicFile *p) {
    w_long((long)n, p);
    w_string(s, n, p);
}

void write_string_to_file(const char *x, ssize_t n, FILE *fp) {
    AicFile af;
    af.fp = fp;
    w_pstring(x, n, &af);
}

void w_object(AiObject *v, AicFile *p) {
    if (++p->depth > MAX_MARSHAL_STACK_DEPTH) {
        FATAL_ERROR("code recursion too deep");
        return;
    }

    if (v == NULL) {
        w_byte(TYPE_NULL, p);
    }
    else if (v == NONE) {
        w_byte(TYPE_NONE, p);
    }
    else if (v == AiFALSE) {
        w_byte(TYPE_FALSE, p);
    }
    else if (v == AiTRUE) {
        w_byte(TYPE_TRUE, p);
    }
    else if (CHECK_TYPE_INT(v)) {
        long x = INT_AS_CLONG(v);
        w_byte(TYPE_INT, p);
        w_long(x, p);
    }
    else if (CHECK_TYPE_STRING(v)) {
        if (p->strings && CHECK_STRING_INTERNED(v)) {
            AiObject *o = dict_getitem((AiDictObject *)p->strings, v);
            if (o) {
                long w = INT_AS_CLONG(o);
                w_byte(TYPE_STRINGREF, p);
                w_long(w, p);
            }
            else {
                o = int_from_long((long)DICT_SIZE(p->strings));
                dict_setitem((AiDictObject *)p->strings, v, o);
                XDEC_REFCNT(o);
                w_byte(TYPE_INTERNED, p);
                w_pstring(STRING_AS_CSTRING(v), STRING_LEN(v), p);
            }
        }
        else {
            w_byte(TYPE_STRING, p);
            w_pstring(STRING_AS_CSTRING(v), STRING_LEN(v), p);
        }
    }
    else if (CHECK_TYPE_LIST(v)) {
        ssize_t n;
        w_byte(TYPE_LIST, p);
        n = LIST_SIZE(v);
        w_long((long)n, p);
        for (ssize_t i = 0; i < LIST_SIZE(v); ++i) {
            w_object(list_getitem((AiListObject *)v, i), p);
        }
    }
    else if (CHECK_TYPE_DICT(v)) {
        AiDictObject *mp = (AiDictObject *)v;
        AiDictEntry *ep;
        ssize_t fill = mp->ma_fill;

        w_byte(TYPE_DICT, p);
        for (ep = mp->ma_table; fill > 0; ++ep) {
            if (ep->me_value) {
                --fill;
                w_object(ep->me_key, p);
                w_object(ep->me_value, p);
            }
        }
        w_object((AiObject *)NULL, p);
    }
    else if (CHECK_TYPE_CODE(v)) {
        AiCodeObject *co = (AiCodeObject *)v;
        w_byte(TYPE_CODE, p);
        w_long(co->co_argcount, p);
        w_long(co->co_nlocals, p);
        w_long(co->co_stacksize, p);
        w_long(co->co_flags, p);
        w_object(co->co_code, p);
        w_object(co->co_consts, p);
        w_object(co->co_names, p);
        w_object(co->co_varnames, p);
        w_object(co->co_freevars, p);
        w_object(co->co_cellvars, p);
        w_object(co->co_filename, p);
        w_object(co->co_name, p);
        w_long(co->co_firstlineno, p);
        w_object(co->co_lnotab, p);
    }
    else {
        w_byte(TYPE_UNKNOWN, p);
        p->error = AFERR_UNMARSHALLABLE;
    }
    --p->depth;
}

void write_object_to_file(AiObject *x, FILE *fp) {
    AicFile af;
    af.fp = fp;
    af.error = AFERR_OK;
    af.depth = 0;
    af.strings = dict_new();
    w_object(x, &af);
    DEC_REFCNT(af.strings);
}

AiObject *write_object_to_string(AiObject *x) {
    AicFile af;
    af.fp = NULL;
    af.str = string_from_cstring_with_size(NULL, 50);
    af.ptr = STRING_AS_CSTRING(af.str);
    af.end = af.ptr + STRING_LEN(af.str);
    af.error = AFERR_OK;
    af.depth = 0;
    af.strings = dict_new();

    w_object(x, &af);

    DEC_REFCNT(af.strings);

    string_resize((AiStringObject **)&af.str,
        (ssize_t)(af.ptr - STRING_AS_CSTRING(af.str)));
    return af.str;
}

ssize_t r_string(char *s, ssize_t n, AicFile *p) {
    if (p->fp)
        return fread(s, 1, (size_t)n, p->fp);
    else if (p->end - p->ptr < n)
        n = p->end - p->ptr;
    AiMEM_COPY(s, p->ptr, n);
    p->ptr += n;
    return n;
}

long r_long(AicFile *p) {
    long x;
    FILE *fp = p->fp;

    if (fp) {
        x = getc(p->fp);
        x |= (long)getc(fp) << 8;
        x |= (long)getc(fp) << 16;
        x |= (long)getc(fp) << 24;
    }
    else {
        x = rs_byte(p);
        x |= (long)rs_byte(p) << 8;
        x |= (long)rs_byte(p) << 16;
        x |= (long)rs_byte(p) << 24;
    }
    return x;
}

long read_long_from_file(FILE *fp) {
    AicFile af;
    af.fp = fp;
    af.strings = NULL;
    af.ptr = af.end = NULL;
    return r_long(&af);
}

AiObject *r_object(AicFile *p) {
    int type = r_byte(p);
    AiObject *v, *v2, *retval;
    ssize_t n;

    ++p->depth;

    switch (type) {
    case EOF:
        FATAL_ERROR("EOF read where object expected");
        retval = NULL;
        break;
    case TYPE_NULL:
        retval = NULL;
        break;
    case TYPE_NONE:
        INC_REFCNT(none);
        retval = NONE;
        break;
    case TYPE_FALSE:
        INC_REFCNT(aifalse);
        retval = AiFALSE;
        break;
    case TYPE_TRUE:
        INC_REFCNT(aitrue);
        retval = AiTRUE;
        break;
    case TYPE_INT:
        retval = int_from_long(r_long(p));
        break;
    case TYPE_INTERNED:
    case TYPE_STRING:
        n = r_long(p);
        v = string_from_cstring_with_size((char *)NULL, n);
        if (r_string(STRING_AS_CSTRING(v), n, p) != n) {
            DEC_REFCNT(v);
            FATAL_ERROR("EOF read where object expected");
            retval = NULL;
            break;
        }
        if (type == TYPE_INTERNED) {
            string_intern((AiStringObject **)&v);
            list_append((AiListObject *)p->strings, v);
        }
        retval = v;
        break;
    case TYPE_STRINGREF:
        n = r_long(p);
        v = list_getitem((AiListObject *)p->strings, n);
        INC_REFCNT(v);
        retval = v;
        break;
    case TYPE_LIST:
        n = r_long(p);
        v = list_new(n);
        for (ssize_t i = 0; i < n; ++i) {
            v2 = r_object(p);
            list_setitem((AiListObject *)v, i, v2);
        }
        retval = v;
        break;
    case TYPE_DICT:
        v = dict_new();
        for (;;) {
            AiObject *key, *value;
            key = r_object(p);
            if (!key) {
                break;
            }
            value = r_object(p);
            if (value) {
                dict_setitem((AiDictObject *)v, key, value);
            }
            DEC_REFCNT(key);
            XDEC_REFCNT(value);
        }
        retval = v;
        break;
    case TYPE_CODE:
        {
            int argcount;
            int nlocals;
            int stacksize;
            int flags;
            AiObject *code = NULL;
            AiObject *consts = NULL;
            AiObject *names = NULL;
            AiObject *varnames = NULL;
            AiObject *freevars = NULL;
            AiObject *cellvars = NULL;
            AiObject *filename = NULL;
            AiObject *name = NULL;
            int firstlineno;
            AiObject *lnotab = NULL;

            argcount = (int)r_long(p);
            nlocals = (int)r_long(p);
            stacksize = (int)r_long(p);
            flags = (int)r_long(p);
            code = r_object(p);
            firstlineno = (int)r_long(p);
            lnotab = r_object(p);

            v = code_new(argcount, nlocals, stacksize, flags,
                code, consts, names, varnames,
                freevars, cellvars, filename, name,
                firstlineno, lnotab);

            XDEC_REFCNT(code);
            XDEC_REFCNT(consts);
            XDEC_REFCNT(names);
            XDEC_REFCNT(varnames);
            XDEC_REFCNT(freevars);
            XDEC_REFCNT(cellvars);
            XDEC_REFCNT(filename);
            XDEC_REFCNT(name);
            XDEC_REFCNT(lnotab);
        }
        retval = v;
        break;
    default:
        FATAL_ERROR("bad marshal data (unknown type code)");
        retval = NULL;
        break;
    }
    --p->depth;
    return retval;
}

AiObject *read_object_from_file(FILE *fp) {
    AicFile af;
    AiObject *r;

    af.fp = fp;
    af.strings = list_new(0);
    af.depth = 0;
    af.ptr = af.end = NULL;
    r = r_object(&af);
    DEC_REFCNT(af.strings);

    return r;
}

AiObject *read_object_from_cstring(char *s, ssize_t len) {
    AicFile rf;
    AiObject *r;

    rf.fp = NULL;
    rf.ptr = s;
    rf.end = s + len;
    rf.strings = list_new(0);
    rf.depth = 0;
    r = r_object(&rf);
    DEC_REFCNT(rf.strings);

    return r;
}
