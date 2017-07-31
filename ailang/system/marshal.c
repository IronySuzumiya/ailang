#include "../ailang.h"

typedef struct _aicfile {
    FILE *fp;
    int error;
    int depth;
    AiObject *str;
    char *ptr;
    char *end;
    /* dict on marshal, list on unmarshal */
    AiObject *strings;
}
AicFile;

static FILE *open_exclusive(char *filename, mode_t mode) {
#if defined(O_EXCL)&&defined(O_CREAT)&&defined(O_WRONLY)&&defined(O_TRUNC)
    int fd;
    _unlink(filename);
    fd = open(filename, O_EXCL | O_CREAT | O_WRONLY | O_TRUNC
#ifdef O_BINARY
        | O_BINARY
#endif
    );
    if (fd < 0) {
        return NULL;
    }
    else {
        return _fdopen(fd, "wb");
    }
#endif
}

static void w_more(int c, AicFile *p) {
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
    *p->ptr++ = SAFE_DOWNCAST(c, int, char);
}

static void w_long(long x, AicFile *p) {
    w_byte((char)(x & 0xff), p);
    w_byte((char)((x >> 8) & 0xff), p);
    w_byte((char)((x >> 16) & 0xff), p);
    w_byte((char)((x >> 24) & 0xff), p);
}

static void write_long_to_file(long x, FILE *fp) {
    AicFile af;
    af.fp = fp;
    af.error = AFERR_OK;
    af.depth = 0;
    af.strings = NULL;
    w_long(x, &af);
}

static void w_string(const char *s, ssize_t n, AicFile *p) {
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

static void w_pstring(const char *s, ssize_t n, AicFile *p) {
    w_long((long)n, p);
    w_string(s, n, p);
}

static void w_object(AiObject *v, AicFile *p) {
    ++p->depth;

    if (v == NULL) {
        w_byte(TYPE_NULL, p);
    }
    else if (v == NONE) {
        w_byte(TYPE_NONE, p);
    }
    else if (v == AI_FALSE) {
        w_byte(TYPE_FALSE, p);
    }
    else if (v == AI_TRUE) {
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
            }
        }
        else {
            w_byte(TYPE_STRING, p);
        }
        w_pstring(STRING_AS_CSTRING(v), STRING_LEN(v), p);
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
        ssize_t pos = 0;
        AiDictObject *mp = (AiDictObject *)v;
        AiObject *key, *value;
        AiDictEntry *ep;
        ssize_t fill = mp->ma_fill;

        w_byte(TYPE_DICT, p);
        for (ep = mp->ma_table; fill > 0; ++ep) {
            if (ep->me_key) {
                --fill;
                w_object(key, p);
                w_object(value, p);
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

static void write_object_to_file(AiObject *x, FILE *fp) {
    AicFile af;
    af.fp = fp;
    af.error = AFERR_OK;
    af.depth = 0;
    af.strings = dict_new();
    w_object(x, &af);
    DEC_REFCNT(af.strings);
}

static void write_compiled_module(AiCodeObject *co, char *path, struct stat *srcstat, time_t mtime) {
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

static ssize_t r_string(char *s, ssize_t n, AicFile *p) {
    if (p->fp)
        return fread(s, 1, (size_t)n, p->fp);
    else if (p->end - p->ptr < n)
        n = p->end - p->ptr;
    AiMEM_COPY(s, p->ptr, n);
    p->ptr += n;
    return n;
}

static long r_long(AicFile *p) {
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

static AiObject *r_object(AicFile *p) {
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
        retval = AI_FALSE;
        break;
    case TYPE_TRUE:
        INC_REFCNT(aitrue);
        retval = AI_TRUE;
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

            v = NULL;

            argcount = (int)r_long(p);
            nlocals = (int)r_long(p);
            stacksize = (int)r_long(p);
            flags = (int)r_long(p);
            code = r_object(p);
            firstlineno = (int)r_long(p);
            lnotab = r_object(p);

            v = (AiObject *)code_new(
                argcount, nlocals, stacksize, flags,
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
