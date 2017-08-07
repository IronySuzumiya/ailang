#include "../ailang.h"

static int add_operators(AiTypeObject *type);
static int add_members(AiTypeObject *type, AiMemberDef *mem);
static int add_methods(AiTypeObject *type, AiMethodDef *meth);
static int add_subclass(AiTypeObject *base, AiTypeObject *type);
static void inherit_special(AiTypeObject *type, AiTypeObject *base);

static AiObject *lookup_method(AiObject *self, char *attrstr, AiObject **attrobj);

static void init_slotdefs(void);
static void **slotptr(AiTypeObject *type, int offset);
static int AiObject_SlotCompare(AiObject *self, AiObject *other);
static int half_compare(AiObject *self, AiObject *other);

static AiObject *wrap_cmpfunc(AiObject *self, AiObject *args, void *wrapped);

static int check_num_args(AiObject *ob, int n);
static void subtype_dealloc(AiObject *self);

static void type_dealloc(AiTypeObject *type);
static void type_print(AiTypeObject *ob, FILE *stream);
static AiObject *type_call(AiTypeObject *type, AiObject *args, AiObject *kwds);
static AiObject *type_new(AiTypeObject *metatype, AiObject *args, AiObject *kwds);

#define TPSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    { NAME, offsetof(AiTypeObject, SLOT), (void *)(FUNCTION), WRAPPER, DOC }

#define FLSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC, FLAGS) \
    { NAME, offsetof(AiTypeObject, SLOT), (void *)(FUNCTION), WRAPPER, DOC, FLAGS }

#define ETSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    { NAME, offsetof(AiHeapTypeObject, SLOT), (void *)(FUNCTION), WRAPPER, DOC }

#define SQSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    ETSLOT(NAME, as_sequence.SLOT, FUNCTION, WRAPPER, DOC)

#define MPSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    ETSLOT(NAME, as_mapping.SLOT, FUNCTION, WRAPPER, DOC)

#define NBSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    ETSLOT(NAME, as_number.SLOT, FUNCTION, WRAPPER, DOC)

#define UNSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    ETSLOT(NAME, as_number.SLOT, FUNCTION, WRAPPER, "x." NAME "() <==> " DOC)

#define IBSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    ETSLOT(NAME, as_number.SLOT, FUNCTION, WRAPPER, "x." NAME "(y) <==> x" DOC "y")

#define BINSLOT(NAME, SLOT, FUNCTION, DOC) \
    ETSLOT(NAME, as_number.SLOT, FUNCTION, wrap_binaryfunc_l, "x." NAME "(y) <==> x" DOC "y")

#define RBINSLOT(NAME, SLOT, FUNCTION, DOC) \
    ETSLOT(NAME, as_number.SLOT, FUNCTION, wrap_binaryfunc_r, "x." NAME "(y) <==> y" DOC "x")

#define BINSLOTNOTINFIX(NAME, SLOT, FUNCTION, DOC) \
    ETSLOT(NAME, as_number.SLOT, FUNCTION, wrap_binaryfunc_l, "x." NAME "(y) <==> " DOC)

#define RBINSLOTNOTINFIX(NAME, SLOT, FUNCTION, DOC) \
    ETSLOT(NAME, as_number.SLOT, FUNCTION, wrap_binaryfunc_r, "x." NAME "(y) <==> " DOC)

typedef struct wrapperbase slotdef;

static slotdef slotdefs[] = {
    TPSLOT("__cmp__", tp_compare, AiObject_SlotCompare, wrap_cmpfunc, "x.__cmp__(y) <==> cmp(x,y)"),
    /*
    TPSLOT("__hash__", tp_hash, slot_tp_hash, wrap_hashfunc, "x.__hash__() <==> hash(x)"),

    FLSLOT("__call__", tp_call, slot_tp_call, (wrapperfunc)wrap_call, "x.__call__(...) <==> x(...)", WRAPPER_FLAG_KEYWORDS),

    TPSLOT("__str__", tp_str, slot_tp_str, wrap_unaryfunc, "x.__str__() <==> str(x)"),

    TPSLOT("__getattr__", tp_getattro, slot_tp_getattr_hook, NULL, ""),

    TPSLOT("__setattr__", tp_setattro, slot_tp_setattro, wrap_setattr, "x.__setattr__('name', value) <==> x.name = value"),

    TPSLOT("__delattr__", tp_setattro, slot_tp_setattro, wrap_delattr, "x.__delattr__('name') <==> del x.name"),

    TPSLOT("__lt__", tp_richcompare, slot_tp_richcompare, richcmp_lt, "x.__lt__(y) <==> x<y"),

    TPSLOT("__le__", tp_richcompare, slot_tp_richcompare, richcmp_le, "x.__le__(y) <==> x<=y"),
    
    TPSLOT("__eq__", tp_richcompare, slot_tp_richcompare, richcmp_eq, "x.__eq__(y) <==> x==y"),

    TPSLOT("__ne__", tp_richcompare, slot_tp_richcompare, richcmp_ne, "x.__ne__(y) <==> x!=y"),

    TPSLOT("__gt__", tp_richcompare, slot_tp_richcompare, richcmp_gt, "x.__gt__(y) <==> x>y"),

    TPSLOT("__ge__", tp_richcompare, slot_tp_richcompare, richcmp_ge, "x.__ge__(y) <==> x>=y"),

    TPSLOT("__iter__", tp_iter, slot_tp_iter, wrap_unaryfunc, "x.__iter__() <==> iter(x)"),

    TPSLOT("next", tp_iternext, slot_tp_iternext, wrap_next, "x.next() -> the next value"),

    TPSLOT("__get__", tp_descr_get, slot_tp_descr_get, wrap_descr_get, "descr.__get__(obj[, type]) -> value"),

    TPSLOT("__set__", tp_descr_set, slot_tp_descr_set, wrap_descr_set, "descr.__set__(obj, value)"),

    TPSLOT("__delete__", tp_descr_set, slot_tp_descr_set, wrap_descr_delete, "descr.__delete__(obj)"),

    FLSLOT("__init__", tp_init, slot_tp_init, (wrapperfunc)wrap_init, "x.__init__(...) initializes x", WRAPPER_FLAG_KEYWORDS),

    TPSLOT("__new__", tp_new, slot_tp_new, NULL, ""),

    TPSLOT("__del__", tp_del, slot_tp_del, NULL, ""),

    BINSLOT("__add__", nb_add, slot_nb_add, "+"),

    RBINSLOT("__radd__", nb_add, slot_nb_add, "+"),

    BINSLOT("__sub__", nb_subtract, slot_nb_subtract, "-"),

    RBINSLOT("__rsub__", nb_subtract, slot_nb_subtract, "-"),

    BINSLOT("__mul__", nb_multiply, slot_nb_multiply, "*"),

    RBINSLOT("__rmul__", nb_multiply, slot_nb_multiply, "*"),

    BINSLOT("__div__", nb_divide, slot_nb_divide, "/"),

    RBINSLOT("__rdiv__", nb_divide, slot_nb_divide, "/"),

    BINSLOT("__mod__", nb_remainder, slot_nb_remainder, "%"),

    RBINSLOT("__rmod__", nb_remainder, slot_nb_remainder, "%"),

    NBSLOT("__pow__", nb_power, slot_nb_power, wrap_ternaryfunc, "x.__pow__(y[, z]) <==> pow(x, y[, z])"),

    NBSLOT("__rpow__", nb_power, slot_nb_power, wrap_ternaryfunc_r, "y.__rpow__(x[, z]) <==> pow(x, y[, z])"),

    UNSLOT("__neg__", nb_negative, slot_nb_negative, wrap_unaryfunc, "-x"),

    UNSLOT("__pos__", nb_positive, slot_nb_positive, wrap_unaryfunc, "+x"),

    UNSLOT("__abs__", nb_absolute, slot_nb_absolute, wrap_unaryfunc, "abs(x)"),

    UNSLOT("__nonzero__", nb_nonzero, slot_nb_nonzero, wrap_inquirypred, "x != 0"),

    BINSLOT("__lshift__", nb_lshift, slot_nb_lshift, "<<"),

    RBINSLOT("__rlshift__", nb_lshift, slot_nb_lshift, "<<"),

    BINSLOT("__rshift__", nb_rshift, slot_nb_rshift, ">>"),

    RBINSLOT("__rrshift__", nb_rshift, slot_nb_rshift, ">>"),

    BINSLOT("__and__", nb_and, slot_nb_and, "&"),

    RBINSLOT("__rand__", nb_and, slot_nb_and, "&"),

    BINSLOT("__xor__", nb_xor, slot_nb_xor, "^"),

    RBINSLOT("__rxor__", nb_xor, slot_nb_xor, "^"),

    BINSLOT("__or__", nb_or, slot_nb_or, "|"),

    RBINSLOT("__ror__", nb_or, slot_nb_or, "|"),

    UNSLOT("__not__", nb_not, slot_nb_not, wrap_unaryfunc, "!"),

    UNSLOT("__invert__", nb_invert, slot_nb_invert, wrap_unaryfunc, "~"),

    UNSLOT("__int__", nb_int, slot_nb_int, wrap_unaryfunc, "int(x)"),

    UNSLOT("__float__", nb_float, slot_nb_float, wrap_unaryfunc, "float(x)"),

    MPSLOT("__len__", mp_length, slot_mp_length, wrap_lenfunc, "x.__len__() <==> len(x)"),

    MPSLOT("__getitem__", mp_getitem, slot_getitem, wrap_binaryfunc, "x.__getitem__(y) <==> x[y]"),

    MPSLOT("__setitem__", mp_setitem, slot_setitem, wrap_objobjargproc, "x.__setitem__(i, y) <==> x[i]=y"),

    MPSLOT("__delitem__", mp_delitem, slot_delitem, wrap_delitem, "x.__delitem__(y) <==> del x[y]"),

    SQSLOT("__len__", sq_length, slot_sq_length, wrap_lenfunc, "x.__len__() <==> len(x)"),

    SQSLOT("__add__", sq_concat, NULL, wrap_binaryfunc, "x.__add__(y) <==> x+y"),

    SQSLOT("__getitem__", sq_getitem, slot_getitem, wrap_sq_getitem, "x.__getitem__(y) <==> x[y]"),

    SQSLOT("__setitem__", sq_setitem, slot_setitem, wrap_sq_setitem, "x.__setitem__(i, y) <==> x[i]=y"),

    SQSLOT("__slice__", sq_slice, slot_sq_slice, wrap_ssizessizeargfunc, "x.__getslice__(i, j) <==> x[i:j]"),

    SQSLOT("__contains__", sq_contains, slot_sq_contains, wrap_objobjproc, "x.__contains__(y) <==> y in x"),
    */
    { NULL }
};

AiTypeObject AiType_Type = {
    AiVarObject_HEAD_INIT(&AiType_Type, 0)
    "type",                             /* tp_name */
    sizeof(AiHeapTypeObject),           /* tp_basicsize */
    sizeof(AiMemberDef),                /* tp_itemsize */
    (destructor)type_dealloc,           /* tp_dealloc */
    (printfunc)type_print,              /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    (hashfunc)Pointer_Hash,             /* tp_hash */
    (ternaryfunc)type_call,             /* tp_call */
    0,                                  /* tp_str */

    0,//(getattrofunc)type_getattro,        /* tp_getattro */
    0,//(setattrofunc)type_setattro,        /* tp_setattro */

    SUBCLASS_TYPE | BASE_TYPE,          /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,//type_methods,                       /* tp_methods */
    0,//type_members,                       /* tp_members */

    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,//type_init,                          /* tp_init */
    0,                                  /* tp_alloc */
    type_new,                           /* tp_new */
    AiObject_GC_Del,                    /* tp_free */
};

int AiType_Ready(AiTypeObject *type) {
    if (!type->tp_base && type != &AiType_BaseObject) {
        type->tp_base = &AiType_BaseObject;
        INC_REFCNT(type->tp_base);
    }
    if (type->tp_base && !type->tp_base->tp_dict) {
        AiType_Ready(type->tp_base);
    }
    if (!OB_TYPE(type) && type->tp_base) {
        OB_TYPE(type) = OB_TYPE(type->tp_base);
    }
    if (!type->tp_dict) {
        type->tp_dict = AiDict_New();
    }
    add_operators(type);
    if (type->tp_methods) {
        add_methods(type, type->tp_methods);
    }
    if (type->tp_members) {
        add_members(type, type->tp_members);
    }
    if (type->tp_base) {
        inherit_special(type, type->tp_base);
    }
    if (type->tp_base) {
        if (!type->tp_as_number) {
            type->tp_as_number = type->tp_base->tp_as_number;
        }
        if (!type->tp_as_sequence) {
            type->tp_as_sequence = type->tp_base->tp_as_sequence;
        }
        if (!type->tp_as_mapping) {
            type->tp_as_mapping = type->tp_base->tp_as_mapping;
        }
        add_subclass(type->tp_base, type);
    }
    return 0;
}

AiObject *AiType_Generic_Alloc(AiTypeObject *type, ssize_t nitems) {
    AiObject *obj;
    ssize_t size = _AiVarObject_SIZE(type, nitems + 1);

    obj = (AiObject *)AiObject_Malloc(size);
    AiMem_Set(obj, 0, size);
    if (type->tp_flags & HEAP_TYPE) {
        INC_REFCNT(type);
    }
    if (type->tp_itemsize) {
        AiVarObject_INIT(obj, type, nitems);
    }
    else {
        AiObject_INIT(obj, type);
    }
    return obj;
}

int AiType_IsSubclass(AiTypeObject *type, AiTypeObject *base) {
    do {
        if (type == base) {
            return 1;
        }
        else {
            type = type->tp_base;
        }
    } while (type);
    return base == &AiType_BaseObject;
}

AiObject *_AiType_Lookup(AiTypeObject *type, AiObject *name) {
    AiObject *res;

    if (res = AiDict_GetItem((AiDictObject *)type->tp_dict, name)) {
        return res;
    }
    else {
        return AiDict_GetItem((AiDictObject *)type->tp_base->tp_dict, name);
    }
}

int add_operators(AiTypeObject *type) {
    AiDictObject *dict = (AiDictObject *)type->tp_dict;
    AiObject *descr;
    void **ptr;

    init_slotdefs();
    for (slotdef *p = slotdefs; p->name; ++p) {
        if (p->wrapper) {
            ptr = slotptr(type, p->offset);
            if (ptr && *ptr && !AiDict_GetItem(dict, p->name_strobj)) {
                if (*ptr == AiObject_Unhashable) {
                    AiDict_SetItem(dict, p->name_strobj, NONE);
                }
                else {
                    descr = AiDescr_NewWrapper(type, p, *ptr);
                    AiDict_SetItem(dict, p->name_strobj, descr);
                    DEC_REFCNT(descr);
                }
            }
        }
    }
    return 0;
}

int add_members(AiTypeObject *type, AiMemberDef *mem) {
    AiDictObject *dict = (AiDictObject *)type->tp_dict;

    for (; mem->name; ++mem) {
        AiObject *descr;
        AiObject *namestr = AiString_From_String(mem->name);
        if (!AiDict_GetItem(dict, namestr)) {
            AiString_Intern((AiStringObject **)&namestr);
            descr = AiDescr_NewMember(type, mem);
            AiDict_SetItem(dict, namestr, descr);
            DEC_REFCNT(descr);
        }
        DEC_REFCNT(namestr);
    }
    return 0;
}

int add_methods(AiTypeObject *type, AiMethodDef *meth) {
    AiDictObject *dict = (AiDictObject *)type->tp_dict;

    for (; meth->ml_name; ++meth) {
        AiObject *descr;
        AiObject *namestr = AiString_From_String(meth->ml_name);
        if (!AiDict_GetItem(dict, namestr)) {
            AiString_Intern((AiStringObject **)&namestr);
            descr = AiDescr_NewMethod(type, meth);
            AiDict_SetItem(dict, namestr, descr);
            DEC_REFCNT(descr);
        }
        DEC_REFCNT(namestr);
    }
    return 0;
}

int add_subclass(AiTypeObject *base, AiTypeObject *type) {
    AiObject *list;

    // implement weakref in the future
    list = base->tp_subclasses;
    if (!list) {
        base->tp_subclasses = list = AiList_New(0);
    }
    list_append((AiListObject *)list, (AiObject *)type);
    return 0;
}

void inherit_special(AiTypeObject *type, AiTypeObject *base) {
    if (!type->tp_basicsize) {
        type->tp_basicsize = base->tp_basicsize;
    }
    if (base != &AiType_BaseObject || (type->tp_flags & HEAP_TYPE)) {
        if (!type->tp_new) {
            type->tp_new = base->tp_new;
        }
    }

#define COPYVAL(SLOT)   \
    if(!type->SLOT) type->SLOT = base->SLOT

    COPYVAL(tp_itemsize);
    if (AiType_IsSubclass(base, &AiType_BaseException)) {
        type->tp_flags |= SUBCLASS_BASEEXC;
    }/*
    else if (AiType_IsSubclass(base, &AiType_Type)) {
        type->tp_flags |= SUBCLASS_TYPE;
    }*/
    else if (AiType_IsSubclass(base, &AiType_Int)) {
        type->tp_flags |= SUBCLASS_INT;
    }/*
    else if (AiType_IsSubclass(base, &AiType_Float)) {
        type->tp_flags |= SUBCLASS_FLOAT;
    }*/
    else if (AiType_IsSubclass(base, &AiType_String)) {
        type->tp_flags |= SUBCLASS_STRING;
    }
    else if (AiType_IsSubclass(base, &AiType_Tuple)) {
        type->tp_flags |= SUBCLASS_TUPLE;
    }
    else if (AiType_IsSubclass(base, &AiType_List)) {
        type->tp_flags |= SUBCLASS_LIST;
    }
    else if (AiType_IsSubclass(base, &AiType_Dict)) {
        type->tp_flags |= SUBCLASS_DICT;
    }
}

AiObject *lookup_method(AiObject *self, char *attrstr, AiObject **attrobj) {
    AiObject *res;

    if (!*attrobj) {
        AiObject *str = AiString_From_String(attrstr);
        AiString_Intern((AiStringObject **)&str);
        *attrobj = str;
    }
    res = _AiType_Lookup(OB_TYPE(self), *attrobj);
    if (res) {
        descrgetfunc f;
        if (!(f = OB_TYPE(res)->tp_descr_get)) {
            INC_REFCNT(res);
        }
        else {
            res = f(res, self, (AiObject *)(OB_TYPE(self)));
        }
    }
    else {
        RUNTIME_EXCEPTION("no such attribute");
    }
    return res;
}

void init_slotdefs() {
    static int initialized = 0;
    if (initialized)
        return;

    for (slotdef *p = slotdefs; p->name; ++p) {
        AiObject *str = AiString_From_String(p->name);
        AiString_Intern((AiStringObject **)&str);
        p->name_strobj = str;
    }

    initialized = 1;
}

void **slotptr(AiTypeObject *type, int offset) {
    char *ptr;

    if ((size_t)offset >= offsetof(AiHeapTypeObject, as_sequence)) {
        ptr = (char *)type->tp_as_sequence;
        offset -= offsetof(AiHeapTypeObject, as_sequence);
    }
    else if ((size_t)offset >= offsetof(AiHeapTypeObject, as_mapping)) {
        ptr = (char *)type->tp_as_mapping;
        offset -= offsetof(AiHeapTypeObject, as_mapping);
    }
    else if ((size_t)offset >= offsetof(AiHeapTypeObject, as_number)) {
        ptr = (char *)type->tp_as_number;
        offset -= offsetof(AiHeapTypeObject, as_number);
    }
    else {
        ptr = (char *)type;
    }
    if (ptr) {
        ptr += offset;
    }
    return (void **)ptr;
}

int AiObject_SlotCompare(AiObject *self, AiObject *other) {
    int c;

    if (OB_TYPE(self)->tp_compare == AiObject_SlotCompare) {
        c = half_compare(self, other);
        if (c <= 1)
            return c;
    }
    if (OB_TYPE(other)->tp_compare == AiObject_SlotCompare) {
        c = half_compare(other, self);
        if (c < -1)
            return -2;
        if (c <= 1)
            return -c;
    }
    return (void *)self < (void *)other ? -1 :
        (void *)self >(void *)other ? 1 : 0;
}

int half_compare(AiObject *self, AiObject *other) {
    AiObject *func, *args, *res;
    static AiObject *cmp_str;
    ssize_t c;

    func = lookup_method(self, "__cmp__", &cmp_str);
    if (func) {
        args = AiTuple_Pack(1, other);
        res = AiObject_Call(func, args, NULL);
        DEC_REFCNT(args);
        DEC_REFCNT(func);
        if (res != AiNotImplemented) {
            if (!res)
                return -2;
            c = INT_AS_CLONG(res);
            DEC_REFCNT(res);
            if (c == -1 && EXCEPTION_OCCURRED())
                return -2;
            return c < 0 ? -1 : c > 0 ? 1 : 0;
        }
        DEC_REFCNT(res);
    }
    return 2;
}

AiObject *wrap_cmpfunc(AiObject *self, AiObject *args, void *wrapped) {
    cmpfunc func = (cmpfunc)wrapped;
    int res;
    AiObject *other;
    if (!check_num_args(args, 1)) {
        return NULL;
    }
    else {
        other = TUPLE_GETITEM(args, 0);
        if (OB_TYPE(other)->tp_compare != func &&
            !AiType_IsSubclass(OB_TYPE(other), OB_TYPE(self))) {
            RUNTIME_EXCEPTION("%s.__cmp__(x,y) requires y to be a '%s', not a '%s'",
                OB_TYPE(self)->tp_name, OB_TYPE(self)->tp_name,
                OB_TYPE(other)->tp_name);
            return NULL;
        }
        else {
            res = (*func)(self, other);
            return AiInt_From_Long((long)res);
        }
    }
}

int check_num_args(AiObject *ob, int n) {
    if (CHECK_EXACT_TYPE_TUPLE(ob)) {
        if (n == TUPLE_SIZE(ob)) {
            return 1;
        }
        else {
            RUNTIME_EXCEPTION("expected %d arguments, got %d", n, TUPLE_SIZE(ob));
            return 0;
        }
    }
    else {
        RUNTIME_EXCEPTION("argument list is not a tuple");
        return 0;
    }
}

void subtype_dealloc(AiObject *self) {
    // TODO
}

void type_dealloc(AiTypeObject *type) {
    AiHeapTypeObject *et;

    assert(type->tp_flags & HEAP_TYPE);
    et = (AiHeapTypeObject *)type;
    XDEC_REFCNT(type->tp_base);
    XDEC_REFCNT(type->tp_dict);
    XDEC_REFCNT(type->tp_subclasses);
    XDEC_REFCNT(et->ht_name);
    XDEC_REFCNT(et->ht_slots);
    OB_FREE(type);
}

void type_print(AiTypeObject *ob, FILE *stream) {
    fputs("<type 'type'>", stream);
}

AiObject *type_call(AiTypeObject *type, AiObject *args, AiObject *kwds) {
    AiObject *obj;
    if (!type->tp_new) {
        RUNTIME_EXCEPTION("cannot create '%s' instances", type->tp_name);
        return NULL;
    }
    else {
        obj = type->tp_new(type, args, kwds);
        if (obj) {
            if (type == &AiType_Type && CHECK_TYPE_TUPLE(args) && TUPLE_SIZE(args) == 1
                && (!kwds || (CHECK_TYPE_DICT(kwds) && DICT_SIZE(kwds) == 0))) {
                return obj;
            }
            else if (!AiType_IsSubclass(obj->ob_type, type)) {
                return obj;
            }
            else {
                type = obj->ob_type;
                if (type->tp_init) {
                    type->tp_init(obj, args, kwds);
                    DEC_REFCNT(obj);
                }
            }
        }
        return obj;
    }
}

AiObject *type_new(AiTypeObject *metatype, AiObject *args, AiObject *kwds) {
    /* Special case: type(x) should return x->ob_type */
    ssize_t nargs = TUPLE_SIZE(args);
    ssize_t nkwds = kwds ? DICT_SIZE(kwds) : 0;
    AiObject *name;
    AiObject *slots = NULL;
    AiDictObject *dict;
    AiTypeObject *type, *base;
    AiHeapTypeObject *et;

    if (!CHECK_EXACT_TYPE_TYPE(metatype)) {
        FATAL_ERROR("user defined metaclass not supported now");
        return NULL;
    }

    if (nargs == 1 && nkwds == 0) {
        AiObject *x = TUPLE_GETITEM(args, 0);
        INC_REFCNT(OB_TYPE(x));
        return (AiObject *)OB_TYPE(x);
    }
    else if (nargs != 3) {
        if (nargs + nkwds == 3) {
            TYPE_ERROR("calling type() with keywords not supported now");
        }
        else {
            TYPE_ERROR("type() takes 1 or 3 arguments");
        }
        return NULL;
    }
    else {
        // name <- args[0] | kwds['name']
        // base <- args[1] | kwds['base']
        // dict <- args[2] | kwds['dict']
        // slots <- dict['__slots__']
        name = TUPLE_GETITEM(args, 0);
        base = (AiTypeObject *)TUPLE_GETITEM(args, 1);
        dict = (AiDictObject *)TUPLE_GETITEM(args, 2);

        /* do something with slots */

        // so many so many problems here
        type = (AiTypeObject *)metatype->tp_alloc(metatype, 0);
        et = (AiHeapTypeObject *)type;
        INC_REFCNT(name);
        et->ht_name = name;
        et->ht_slots = slots;
        type->tp_flags = HEAP_TYPE | BASE_TYPE;
        type->tp_as_number = &et->as_number;
        type->tp_as_sequence = &et->as_sequence;
        type->tp_as_mapping = &et->as_mapping;
        type->tp_name = STRING_AS_CSTRING(name);
        type->tp_base = base;
        INC_REFCNT(base);
        type->tp_dict = AiDict_Copy(dict);
        type->tp_basicsize = base->tp_basicsize;
        type->tp_itemsize = base->tp_itemsize;
        type->tp_members = AiHeapType_GET_MEMBERS(et);
        type->tp_dealloc = subtype_dealloc;
        type->tp_alloc = AiType_Generic_Alloc;
        type->tp_free = AiObject_GC_Del;

        AiType_Ready(type);
        return (AiObject *)type;
    }
}
