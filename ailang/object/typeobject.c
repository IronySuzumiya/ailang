#include "../ailang.h"

#define TPSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    {NAME, offsetof(AiTypeObject, SLOT), (void *)(FUNCTION), WRAPPER, DOC}

#define FLSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC, FLAGS) \
    {NAME, offsetof(AiTypeObject, SLOT), (void *)(FUNCTION), WRAPPER, DOC, FLAGS}

#define ETSLOT(NAME, SLOT, FUNCTION, WRAPPER, DOC) \
    {NAME, offsetof(AiHeapTypeObject, SLOT), (void *)(FUNCTION), WRAPPER, DOC}

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
    TPSLOT("__str__", tp_print, NULL, NULL, ""),

    TPSLOT("__getattr__", tp_getattr, NULL, NULL, ""),

    TPSLOT("__setattr__", tp_setattr, NULL, NULL, ""),

    TPSLOT("__delattr__", tp_setattr, NULL, NULL, ""),
    /*
    TPSLOT("__cmp__", tp_compare, AiObject_SlotCompare, wrap_cmpfunc, "x.__cmp__(y) <==> cmp(x,y)"),
    
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

    SQSLOT("__len__", sq_length, slot_sq_length, wrap_lenfunc, "x.__len__() <==> len(x)"),

    SQSLOT("__add__", sq_concat, NULL, wrap_binaryfunc, "x.__add__(y) <==> x+y"),

    SQSLOT("__getitem__", sq_getitem, slot_getitem, wrap_sq_getitem, "x.__getitem__(y) <==> x[y]"),

    SQSLOT("__setitem__", sq_setitem, slot_setitem, wrap_sq_setitem, "x.__setitem__(i, y) <==> x[i]=y"),

    SQSLOT("__slice__", sq_slice, slot_sq_slice, wrap_ssizessizeargfunc, "x.__getslice__(i, j) <==> x[i:j]"),

    SQSLOT("__contains__", sq_contains, slot_sq_contains, wrap_objobjproc, "x.__contains__(y) <==> y in x"),

    MPSLOT("__len__", mp_length, slot_mp_length, wrap_lenfunc, "x.__len__() <==> len(x)"),

    MPSLOT("__getitem__", mp_getitem, slot_getitem, wrap_binaryfunc, "x.__getitem__(y) <==> x[y]"),

    MPSLOT("__setitem__", mp_setitem, slot_setitem, wrap_objobjargproc, "x.__setitem__(i, y) <==> x[i]=y"),

    MPSLOT("__delitem__", mp_delitem, slot_delitem, wrap_delitem, "x.__delitem__(y) <==> del x[y]"),
    */
    { NULL }
};

static void type_dealloc(AiTypeObject *type);
static void type_print(AiTypeObject *ob, FILE *stream);

AiTypeObject type_typeobject = {
    INIT_AiVarObject_HEAD(&type_typeobject, 0)
    "type",                             /* tp_name */
    sizeof(AiHeapTypeObject),           /* tp_basicsize */
    0,//sizeof(AiMemberDef),            /* tp_itemsize */
    (destructor)type_dealloc,           /* tp_dealloc */
    (printfunc)type_print,              /* tp_print */
    0,                                  /* tp_compare */

    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */

    (hashfunc)pointer_hash,             /* tp_hash */
    0,//(ternaryfunc)type_call,             /* tp_call */
    0,                                  /* tp_str */

    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,//(getattrofunc)type_getattro,        /* tp_getattro */
    0,//(setattrofunc)type_setattro,        /* tp_setattro */

    SUBCLASS_TYPE | BASE_TYPE,          /* tp_flags */

    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */

    0,//type_methods,                       /* tp_methods */
    0,//type_members,                       /* tp_members */
    0,//type_getset,                        /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    offsetof(AiTypeObject, tp_dict),    /* tp_dictoffset */
    0,//type_init,                          /* tp_init */
    0,                                  /* tp_alloc */
    0,//type_new,                           /* tp_new */
    AiObject_GC_DEL,                    /* tp_free */
    0,//tp_is_gc,                           /* tp_is_gc */
};

int AiType_Ready(AiTypeObject *type) {
    AiObject *dict, *bases;
    AiTypeObject *base;

    base = type->tp_base;
    if (!base && type != &type_baseobject) {
        base = type->tp_base = &type_baseobject;
        INC_REFCNT(base);
    }
    if (base && !base->tp_dict) {
        AiType_Ready(base);
    }
    if (!OB_TYPE(type) && base) {
        OB_TYPE(type) = OB_TYPE(base);
    }
    bases = type->tp_bases;
    if (!bases) {
        if (!base) {
            bases = tuple_new(0);
        }
        else {
            bases = tuple_pack(1, base);
        }
        type->tp_bases = bases;
    }
    dict = type->tp_dict;
    if (!dict) {
        dict = dict_new();
        type->tp_dict = dict;
    }

    // TODO
}

void type_dealloc(AiTypeObject *type) {
    AiHeapTypeObject *et;

    assert(type->tp_flags & HEAP_TYPE);
    et = (AiHeapTypeObject *)type;
    XDEC_REFCNT(type->tp_base);
    XDEC_REFCNT(type->tp_dict);
    XDEC_REFCNT(type->tp_bases);
    XDEC_REFCNT(type->tp_mro);
    XDEC_REFCNT(type->tp_cache);
    XDEC_REFCNT(type->tp_subclasses);
    XDEC_REFCNT(et->ht_name);
    XDEC_REFCNT(et->ht_slots);
    OB_FREE(type);
}

void type_print(AiTypeObject *ob, FILE *stream) {
    fputs("<type 'type'>", stream);
}

int half_compare(AiObject *self, AiObject *other) {
    AiObject *func, *args, *res;
    static AiObject *cmp_str;
    ssize_t c;

    func = lookup_method(self, "__cmp__", &cmp_str);
    if(func) {
        args = tuple_pack(1, other);
        if (!args) {
            res = NULL;
        }
        else {
            res = AiObject_Call(func, args, NULL);
            DEC_REFCNT(args);
        }
        DEC_REFCNT(func);
        if (res != AiNotImplemented) {
            if (!res)
                return -2;
            c = int_as_clong(res);
            DEC_REFCNT(res);
            if (c == -1 && EXCEPTION_OCCURRED())
                return -2;
            return c < 0 ? -1 : c > 0 ? 1 : 0;
        }
        DEC_REFCNT(res);
    }
    return 2;
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
        (void *)self > (void *)other ? 1 : 0;
}
