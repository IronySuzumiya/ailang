#include "../ailang.h"

enum why_code {
    WHY_NOT = 0x0001,       /* No error */
    WHY_EXCEPTION = 0x0002, /* Exception occurred */
    WHY_RERAISE = 0x0004,   /* Exception re-raised by 'finally' */
    WHY_RETURN = 0x0008,    /* 'return' statement */
    WHY_BREAK = 0x0010,     /* 'break' statement */
    WHY_CONTINUE = 0x0020,  /* 'continue' statement */
    WHY_YIELD = 0x0040      /* 'yield' operator */
};

static enum why_code do_raise(AiObject *type, AiObject *value, AiObject *tb);
static AiObject *call_function(AiObject ***pp_stack, int oparg);
static AiObject *fast_function(AiObject *func, AiObject ***pp_stack, int n, int na, int nk);
static AiObject *build_class(AiObject *methods, AiObject *base, AiObject *name);

AiObject *Eval_Frame(AiFrameObject *f) {
    AiObject **stack_pointer;
    unsigned char *next_instr;
    int opcode;
    int oparg;
    enum why_code why;
    AiObject *x;
    AiObject *v;
    AiObject *w;
    AiObject *u;
    AiObject **fastlocals, **freevars;
    AiObject *retval = NULL;
    AiCodeObject *co;
    unsigned char *first_instr;
    AiObject *names;
    AiObject *consts;
    char *filename;
    AiThreadState *tstate = AiThreadState_Get();

    if (!f) {
        return NULL;
    }

#define INSTR_OFFSET()      ((int)(next_instr - first_instr))
#define NEXTOP()            (*next_instr++)
#define NEXTARG()           (next_instr += 2, (next_instr[-1]<<8) + next_instr[-2])
#define PEEKARG()           ((next_instr[2]<<8) + next_instr[1])
#define JUMPTO(x)           (next_instr = first_instr + (x))
#define JUMPBY(x)           (next_instr += (x))

#define STACK_LEVEL()       ((int)(stack_pointer - f->f_valuestack))
#define EMPTY()             (STACK_LEVEL() == 0)
#define TOP()               (stack_pointer[-1])
#define SECOND()            (stack_pointer[-2])
#define THIRD()             (stack_pointer[-3])
#define FOURTH()            (stack_pointer[-4])
#define PEEK(n)             (stack_pointer[-(n)])
#define SET_TOP(v)          (stack_pointer[-1] = (v))
#define SET_SECOND(v)       (stack_pointer[-2] = (v))
#define SET_THIRD(v)        (stack_pointer[-3] = (v))
#define SET_FOURTH(v)       (stack_pointer[-4] = (v))
#define SET_VALUE(n, v)     (stack_pointer[-(n)] = (v))
#define BASIC_STACKADJ(n)   (stack_pointer += n)
#define BASIC_PUSH(v)       (*stack_pointer++ = (v))
#define BASIC_POP()         (*--stack_pointer)
#define PUSH                BASIC_PUSH
#define POP                 BASIC_POP
#define STACKADJ            BASIC_STACKADJ

#define GETLOCAL(i)         (fastlocals[i])
#define SETLOCAL(i, value)              \
    WRAP(                               \
        XDEC_REFCNT(GETLOCAL(i));       \
        GETLOCAL(i) = value;            \
    )

    tstate->frame = f;
    co = f->f_code;
    names = co->co_names;
    consts = co->co_consts;
    fastlocals = f->f_localsplus;
    freevars = f->f_localsplus + co->co_nlocals;
    first_instr = (unsigned char *)STRING_AS_CSTRING(co->co_code);
    next_instr = first_instr + f->f_lasti + 1;
    stack_pointer = f->f_stacktop;

    assert(stack_pointer);

    f->f_stacktop = NULL;
    filename = STRING_AS_CSTRING(co->co_filename);

    why = WHY_NOT;
    x = GET_NONE();
    w = NULL;

    for (;;) {
        f->f_lasti = INSTR_OFFSET();
        opcode = NEXTOP();
        oparg = HAS_ARG(opcode) ? NEXTARG() : 0;

        switch (opcode) {

        case NOP:
            continue;
            
        case LOAD_CONST:
            x = TUPLE_GETITEM(consts, oparg);
            INC_REFCNT(x);
            PUSH(x);
            continue;

        case STORE_NAME:
            w = TUPLE_GETITEM(names, oparg);
            v = POP();
            if (x = f->f_locals) {
                AiDict_SetItem((AiDictObject *)x, w, v);
            }
            DEC_REFCNT(v);
            break;

        case BUILD_MAP:
            x = AiDict_New();
            PUSH(x);
            continue;

        case BUILD_LIST:
            x = AiList_New(oparg);
            while (--oparg >= 0) {
                w = POP();
                AiList_SetItem((AiListObject *)x, oparg, w);
            }
            PUSH(x);
            break;

        case RETURN_VALUE:
            retval = POP();
            why = WHY_RETURN;
            break;

        case DUP_TOP:
            x = TOP();
            INC_REFCNT(x);
            PUSH(x);
            continue;

        case ROT_TWO:
            x = v = POP();
            w = POP();
            PUSH(v);
            PUSH(w);
            continue;

        case STORE_SUBSCR:
            x = w = POP();
            v = POP();
            u = POP();
            AiDict_SetItem((AiDictObject *)v, w, u);
            DEC_REFCNT(u);
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            break;

        case LOAD_NAME:
            w = TUPLE_GETITEM(names, oparg);
            v = f->f_locals;
            x = AiDict_GetItem((AiDictObject *)v, w);
            if (!x) {
                x = AiDict_GetItem((AiDictObject *)f->f_globals, w);
                if (!x) {
                    x = AiDict_GetItem((AiDictObject *)f->f_builtins, w);
                    if (!x) {
                        RUNTIME_EXCEPTION("unknown name %s", STRING_AS_CSTRING(w));
                        break;
                    }
                }
            }
            INC_REFCNT(x);
            PUSH(x);
            break;

        case COMPARE_OP:
            w = POP();
            v = POP();
            x = AiObject_Rich_Compare_Bool(v, w, oparg);
            DEC_REFCNT(w);
            DEC_REFCNT(v);
            if (x) {
                PUSH(x);
            }
            else {
                RUNTIME_EXCEPTION("bad compare");
            }
            break;

        case POP_JUMP_IF_FALSE:
            x = POP();
            if (x == AiFALSE) {
                JUMPBY(oparg);
            }
            break;

        case POP_JUMP_IF_TRUE:
            x = POP();
            if (x == AiTRUE) {
                JUMPBY(oparg);
            }
            break;
            
        case JUMP_FORWARD:
            JUMPBY(oparg);
            continue;

        case SETUP_LOOP:
        case SETUP_EXCEPT:
        case SETUP_FINALLY:
            AiFrame_Setup_Block(f, opcode, INSTR_OFFSET() + oparg, STACK_LEVEL());
            continue;

        case GET_ITER:
            v = POP();
            x = AiObject_Generic_Getiter(v);
            if (x) {
                PUSH(x);
            }
            else {
                RUNTIME_EXCEPTION("object cannot be iterated");
            }
            break;

        case FOR_ITER:
            v = TOP();
            if (!CHECK_TYPE_ITER(v)) {
                RUNTIME_EXCEPTION("only iterator can be index of for-loop");
            }
            else if (x = v->ob_type->tp_iternext(v)) {
                PUSH(x);
            }
            else {
                x = v = POP();
                DEC_REFCNT(v);
                JUMPBY(oparg);
            }
            break;

        case JUMP_ABSOLUTE:
            JUMPTO(oparg);
            continue;

        case POP_BLOCK:
        {
            AiTryBlock *b = AiFrame_Pop_Block(f);
            while (STACK_LEVEL() > b->b_level) {
                v = POP();
                DEC_REFCNT(v);
            }
            continue;
        }

        case BREAK_LOOP:
            why = WHY_BREAK;
            break;

        case CONTINUE_LOOP:
            retval = AiInt_From_Long(oparg);
            why = WHY_CONTINUE;
            break;

        case RAISE_VARARGS:
            u = v = w = NULL;
            switch (oparg) {
            case 3:
                u = POP();
            case 2:
                v = POP();
            case 1:
                w = POP();
            case 0:
                why = do_raise(w, v, u);
                break;
            default:
                RUNTIME_EXCEPTION("bad RAISE_VARARGS oparg");
                why = WHY_EXCEPTION;
                break;
            }
            break;

        case END_FINALLY:
            v = POP();
            if (CHECK_TYPE_INT(v)) {
                why = INT_AS_CLONG(v);
                if (why == WHY_RETURN
                    || why == WHY_CONTINUE) {
                    retval = POP();
                }
                DEC_REFCNT(v);
            }
            else if (AiExceptionClass_Check(v) || CHECK_TYPE_STRING(v)) {
                w = POP();
                u = POP();
                AiException_Restore(v, w, u);
                why = WHY_RERAISE;
            }
            else if (v != NONE) {
                RUNTIME_EXCEPTION("'finally' pops bad exception");
                why = WHY_EXCEPTION;
                DEC_REFCNT(v);
            }
            break;

        case MAKE_FUNCTION:
            v = POP();
            x = AiFunction_New(v, f->f_globals);
            DEC_REFCNT(v);
            if (x && oparg > 0) {
                v = AiTuple_New(oparg);
                while (--oparg >= 0) {
                    w = POP();
                    TUPLE_SETITEM(v, oparg, w);
                }
                AiFunction_SetDefaults((AiFunctionObject *)x, v);
                DEC_REFCNT(v);
            }
            PUSH(x);
            break;

        case CALL_FUNCTION:
        {
            AiObject **sp;
            sp = stack_pointer;
            x = call_function(&sp, oparg);
            stack_pointer = sp;
            PUSH(x);
            break;
        }

        case LOAD_FAST:
            x = GETLOCAL(oparg);
            if (x) {
                INC_REFCNT(x);
                PUSH(x);
            }
            else {
                FATAL_ERROR("unbound local visited");
            }
            break;

        case STORE_FAST:
            x = POP();
            SETLOCAL(oparg, x);
            continue;

        case STORE_DEREF:
            w = POP();
            x = freevars[oparg];
            AiCell_Set((AiCellObject *)x, w);
            DEC_REFCNT(w);
            break;

        case LOAD_DEREF:
            x = freevars[oparg];
            w = AiCell_Get((AiCellObject *)x);
            if (w) {
                PUSH(w);
            }
            else {
                if (oparg < TUPLE_SIZE(co->co_cellvars)) {
                    v = TUPLE_GETITEM(co->co_cellvars, oparg);
                    RUNTIME_EXCEPTION("unbound local visited");
                }
                else {
                    v = TUPLE_GETITEM(co->co_freevars, oparg - TUPLE_SIZE(co->co_cellvars));
                    RUNTIME_EXCEPTION("unbound freevar visited");
                }
            }
            break;

        case LOAD_CLOSURE:
            x = freevars[oparg];
            INC_REFCNT(x);
            PUSH(x);
            break;

        case MAKE_CLOSURE:
            v = POP();
            x = AiFunction_New(v, f->f_globals);
            DEC_REFCNT(v);
            if (x) {
                v = POP();
                AiFunction_SetClosure((AiFunctionObject *)x, v);
                DEC_REFCNT(v);
                if (oparg > 0) {
                    v = AiTuple_New(oparg);
                    while (--oparg >= 0) {
                        w = POP();
                        TUPLE_SETITEM(v, oparg, w);
                    }
                    AiFunction_SetDefaults((AiFunctionObject *)x, v);
                    DEC_REFCNT(v);
                }
            }
            PUSH(x);
            break;

        case LOAD_LOCALS:
            if (x = f->f_locals) {
                INC_REFCNT(x);
                PUSH(x);
                continue;
            }
            else {
                RUNTIME_EXCEPTION("no locals");
                break;
            }

        case BUILD_CLASS:
            u = POP();
            v = POP();
            w = POP();
            x = build_class(u, v, w);
            PUSH(x);
            DEC_REFCNT(u);
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            break;

        case BINARY_ADD:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_add) {
                x = nb->nb_add(v, w);
            }
            else {
                UNSUPPORTED_ADD(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case BINARY_SUBTRACT:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_subtract) {
                x = nb->nb_subtract(v, w);
            }
            else {
                UNSUPPORTED_SUB(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case BINARY_MULTIPLY:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_multiply) {
                x = nb->nb_multiply(v, w);
            }
            else {
                UNSUPPORTED_MUL(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case BINARY_DIVIDE:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_divide) {
                x = nb->nb_divide(v, w);
            }
            else {
                UNSUPPORTED_DIV(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case BINARY_POWER:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_power) {
                x = nb->nb_power(v, w);
            }
            else {
                UNSUPPORTED_POW(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case BINARY_MODULO:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_modulo) {
                x = nb->nb_modulo(v, w);
            }
            else {
                UNSUPPORTED_MOD(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case UNARY_POSITIVE:
        {
            AiNumberMethods *nb;
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_positive) {
                x = nb->nb_positive(v);
            }
            else {
                UNSUPPORTED_POS(OB_TYPENAME(v));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            PUSH(x);
            break;
        }

        case UNARY_NEGATIVE:
        {
            AiNumberMethods *nb;
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_negative) {
                x = nb->nb_negative(v);
            }
            else {
                UNSUPPORTED_NEG(OB_TYPENAME(v));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            PUSH(x);
            break;
        }

        case BINARY_LSHIFT:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_shl) {
                x = nb->nb_shl(v, w);
            }
            else {
                UNSUPPORTED_SHL(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case BINARY_RSHIFT:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_shr) {
                x = nb->nb_shr(v, w);
            }
            else {
                UNSUPPORTED_SHR(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case BINARY_AND:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_and) {
                x = nb->nb_and(v, w);
            }
            else {
                UNSUPPORTED_AND(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case BINARY_OR:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_or) {
                x = nb->nb_or(v, w);
            }
            else {
                UNSUPPORTED_OR(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        case UNARY_NOT:
        {
            AiNumberMethods *nb;
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_not) {
                x = nb->nb_not(v);
            }
            else {
                UNSUPPORTED_NOT(OB_TYPENAME(v));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            PUSH(x);
            break;
        }

        case BINARY_XOR:
        {
            AiNumberMethods *nb;
            w = POP();
            v = POP();
            nb = v->ob_type->tp_as_number;
            if (nb && nb->nb_xor) {
                x = nb->nb_xor(v, w);
            }
            else {
                UNSUPPORTED_XOR(OB_TYPENAME(v), OB_TYPENAME(w));
                x = GET_NONE();
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        }

        if (why == WHY_NOT) {
            if (!x || EXCEPTION_OCCURRED()) {
                why = WHY_EXCEPTION;
                x = GET_NONE();
            }
            else {
                continue;
            }
        }
        if (why == WHY_EXCEPTION) {
            AiTraceback_Here(f);
        }
        if (why == WHY_RERAISE) {
            why = WHY_EXCEPTION;
        }
        while (why != WHY_NOT && f->f_iblock > 0) {
            AiTryBlock *b = AiFrame_Peek_Block(f);
            if (b->b_type == SETUP_LOOP && why == WHY_CONTINUE) {
                why = WHY_NOT;
                JUMPTO(INT_AS_CLONG(retval));
                DEC_REFCNT(retval);
                break;
            }
            --f->f_iblock;
            while (STACK_LEVEL() > b->b_level) {
                v = POP();
                XDEC_REFCNT(v);
            }
            if (b->b_type == SETUP_LOOP && why == WHY_BREAK) {
                why = WHY_NOT;
                JUMPTO(b->b_handler);
                break;
            }
            if (b->b_type == SETUP_FINALLY
                || (b->b_type == SETUP_EXCEPT && why == WHY_EXCEPTION)) {
                if (why == WHY_EXCEPTION) {
                    AiObject *type, *val, *tb;
                    AiException_Fetch(&type, &val, &tb);
                    if (!val) {
                        val = GET_NONE();
                    }
                    if (!tb) {
                        tb = GET_NONE();
                    }
                    PUSH(tb);
                    PUSH(val);
                    PUSH(type);
                }
                else {
                    if (why & (WHY_RETURN | WHY_CONTINUE)) {
                        PUSH(retval);
                    }
                    v = AiInt_From_Long((long)why);
                    PUSH(v);
                }
                why = WHY_NOT;
                JUMPTO(b->b_handler);
                break;
            }
        }
        if (why != WHY_NOT) {
            break;
        }
    }
    while (!EMPTY()) {
        v = POP();
        XDEC_REFCNT(v);
    }
    if (why != WHY_RETURN) {
        retval = NULL;
    }
    tstate->frame = f->f_back;

    return retval;
}

AiObject *Eval_Code(AiCodeObject *co, AiObject *globals, AiObject *locals,
    AiObject **args, int argcount,
    AiObject **kws, int kwcount,
    AiObject **defs, int defcount,
    AiObject *closure) {
    AiFrameObject *f;
    AiObject *retval = NULL;
    AiObject **fastlocals, **freevars;
    AiThreadState *tstate = AiThreadState_Get();

    f = AiFrame_New(tstate, co, globals, locals);
    fastlocals = f->f_localsplus;
    freevars = f->f_localsplus + co->co_nlocals;
    if (co->co_argcount > 0 || co->co_flags & (CO_VARARGS | CO_VARKEYWORDS)) {
        int nposargs = argcount;
        AiObject *lst = NULL;
        AiObject *kwdict = NULL;

        if (argcount > co->co_argcount) {
            if (!(co->co_flags & CO_VARARGS)) {
                TYPE_ERROR("function '%s' takes %s %d argument%s (%d given)",
                    STRING_AS_CSTRING(co->co_name),
                    defcount ? "atmost" : "exactly",
                    co->co_argcount,
                    co->co_argcount == 1 ? "" : "s",
                    argcount + kwcount);
                goto fail;
            }
            else {
                nposargs = co->co_argcount;
            }
        }
        
        if (co->co_flags & CO_VARARGS) {
            lst = AiTuple_New(argcount - nposargs);
            SETLOCAL(co->co_argcount, lst);
        }
        if (co->co_flags & CO_VARKEYWORDS) {
            kwdict = AiDict_New();
            SETLOCAL(co->co_flags & CO_VARARGS ?
                co->co_argcount + 1 : co->co_argcount, kwdict);
        }
        for (int i = 0; i < nposargs; ++i) {
            INC_REFCNT(args[i]);
            SETLOCAL(i, args[i]);
        }
        if (co->co_flags & CO_VARARGS) {
            for (int i = nposargs; i < argcount; ++i) {
                INC_REFCNT(args[i]);
                TUPLE_SETITEM(lst, i - nposargs, args[i]);
            }
        }
        AiObject *keyword;
        AiObject *value;
        int kw_found;
        for (int i = 0; i < kwcount; ++i) {
            keyword = kws[2 * i];
            value = kws[2 * i + 1];
            kw_found = 0;
            if (!keyword || !CHECK_TYPE_STRING(keyword)) {
                TYPE_ERROR("function '%s' keywords must be strings",
                    STRING_AS_CSTRING(co->co_name));
                goto fail;
            }
            for (int j = 0; j < co->co_argcount; ++i) {
                kw_found = AiObject_Rich_Compare(
                    TUPLE_GETITEM(co->co_varnames, j), keyword, CMP_EQ);
                if (kw_found > 0) {
                    INC_REFCNT(value);
                    SETLOCAL(j, value);
                    break;
                }
                else if (kw_found < 0) {
                    RUNTIME_EXCEPTION("bad keyword matching");
                    goto fail;
                }
            }
            if(!kw_found) {
                AiDict_SetItem((AiDictObject *)kwdict, keyword, value);
            }
        }
        if (argcount < co->co_argcount) {
            int nnondefs = co->co_argcount - defcount;
            for (int i = argcount; i < nnondefs; ++i) {
                if (!GETLOCAL(i)) {
                    int given = 0;
                    for (int j = 0; j < co->co_argcount; ++j) {
                        if (GETLOCAL(j)) {
                            ++given;
                        }
                    }
                    TYPE_ERROR("function '%s' takes %s %d argument%s (%d given)",
                        STRING_AS_CSTRING(co->co_name),
                        ((co->co_flags & CO_VARARGS) || defcount) ?
                        "at least" : "exactly",
                        nnondefs, nnondefs == 1 ? "" : "s", given);
                    goto fail;
                }
            }
            for (int i = max(nposargs - nnondefs, 0); i < defcount; ++i) {
                if (!GETLOCAL(nnondefs + i)) {
                    INC_REFCNT(defs[i]);
                    SETLOCAL(nnondefs + i, defs[i]);
                }
            }
        }
    }
    else if (argcount > 0 || kwcount > 0) {
        TYPE_ERROR("function '%s' takes no arguments (%d given)",
            STRING_AS_CSTRING(co->co_name), argcount + kwcount);
        goto fail;
    }

    if (TUPLE_SIZE(co->co_cellvars)) {
        int nargs = co->co_argcount;
        int found;

        if (co->co_flags & CO_VARARGS) {
            ++nargs;
        }
        if (co->co_flags & CO_VARKEYWORDS) {
            ++nargs;
        }
        for (int i = 0; i < TUPLE_SIZE(co->co_cellvars); ++i) {
            found = 0;
            for (int j = 0; j < nargs; ++j) {
                if (!strcmp(STRING_AS_CSTRING(TUPLE_GETITEM(co->co_cellvars, i)),
                    STRING_AS_CSTRING(TUPLE_GETITEM(co->co_varnames, j)))) {
                    SETLOCAL(co->co_nlocals + i, AiCell_New(GETLOCAL(j)));
                    found = 1;
                    break;
                }
            }
            if (!found) {
                SETLOCAL(co->co_nlocals + i, AiCell_New(NULL));
            }
        }
    }
    if (TUPLE_SIZE(co->co_freevars)) {
        AiObject *o;
        for (int i = 0; i < TUPLE_SIZE(co->co_freevars); ++i) {
            o = TUPLE_GETITEM(closure, i);
            INC_REFCNT(o);
            freevars[TUPLE_SIZE(co->co_cellvars) + i] = o;
        }
    }

    retval = Eval_Frame(f);

fail:
    DEC_REFCNT(f);
    return retval;
}

enum why_code do_raise(AiObject *type, AiObject *value, AiObject *tb) {
    if (!type) {
        AiThreadState *tstate = AiThreadState_Get();
        type = tstate->exc_type ? tstate->exc_type : NONE;
        value = tstate->exc_value;
        tb = tstate->exc_traceback;
        XINC_REFCNT(type);
        XINC_REFCNT(value);
        XINC_REFCNT(tb);
    }

    /* We support the following forms of raise:
    raise <class>, <classinstance>
    raise <class>, <argument tuple>
    raise <class>, None
    raise <class>, <argument>
    raise <classinstance>, None
    raise <string>, <object>
    raise <string>, None

    An omitted second argument is the same as None.

    In addition, raise <tuple>, <anything> is the same as
    raising the tuple's first item (and it better have one!);
    this rule is applied recursively.

    Finally, an optional third argument can be supplied, which
    gives the traceback to be substituted (useful when
    re-raising an exception after examining it).  */

    if (tb == NONE) {
        tb = NULL;
    }
    else if (tb && !CHECK_TYPE_TRACEBACK(tb)) {
        RUNTIME_EXCEPTION("raise: arg 3 must be a traceback or None");
        XDEC_REFCNT(value);
        XDEC_REFCNT(type);
        XDEC_REFCNT(tb);
        return WHY_EXCEPTION;
    }
    if (!value) {
        value = GET_NONE();
    }
    while (CHECK_TYPE_TUPLE(type) && TUPLE_SIZE(type) > 0) {
        AiObject *tmp = type;
        type = TUPLE_GETITEM(type, 0);
        INC_REFCNT(type);
        DEC_REFCNT(tmp);
    }
    AiException_Restore(type, value, tb);
    if (!tb) {
        return WHY_EXCEPTION;
    }
    else {
        return WHY_RERAISE;
    }

    XDEC_REFCNT(value);
    XDEC_REFCNT(type);
    XDEC_REFCNT(tb);
    return WHY_EXCEPTION;
}

AiObject *call_function(AiObject ***pp_stack, int oparg) {
    int na = oparg & 0xff;
    int nk = (oparg >> 8) & 0xff;
    int n = na + 2 * nk;
    AiObject **pfunc = (*pp_stack) - n - 1;
    AiObject *func = *pfunc;

    if (CHECK_TYPE_FUNCTION(func) && nk == 0) {
        return fast_function(func, pp_stack, n, na, nk);
    }
    else {
        // TODO
        return NULL;
    }
}

AiObject *fast_function(AiObject *func, AiObject ***pp_stack, int n, int na, int nk) {
    AiCodeObject *co = (AiCodeObject *)FUNCTION_GETCODE(func);
    AiObject *globals = FUNCTION_GETGLOBALS(func);
    AiObject *argdefs = FUNCTION_GETDEFAULTS(func);
    AiObject **d = NULL;
    int nd = 0;

    if (!argdefs && co->co_argcount == n && nk == 0
        && co->co_flags == (CO_OPTIMIZED | CO_NEWLOCALS | CO_NOFREE)) {
        AiFrameObject *f;
        AiObject *retval;
        AiThreadState *tstate = AiThreadState_Get();
        AiObject **fastlocals, **stack;

        f = AiFrame_New(tstate, co, globals, NULL);
        fastlocals = f->f_localsplus;
        stack = (*pp_stack) - n;
        for (ssize_t i = 0; i < n; ++i) {
            fastlocals[i] = *stack++;
        }
        retval = Eval_Frame(f);

        return retval;
    }
    else if (argdefs) {
        if (!CHECK_TYPE_TUPLE(argdefs)) {
            FATAL_ERROR("argdefaults should be given as a tuple");
        }
        d = &TUPLE_GETITEM(argdefs, 0);
        nd = (int)TUPLE_SIZE(argdefs);
    }
    return Eval_Code(co, globals,
        NULL, (*pp_stack) - n, na,
        (*pp_stack) - 2 * nk, nk, d, nd,
        FUNCTION_GETCLOSURE(func));
}

AiObject *build_class(AiObject *methods, AiObject *base, AiObject *name) {
    AiObject *result, *args;
    args = AiTuple_Pack(3, name, base, methods);
    result = AiObject_Call((AiObject *)&AiType_Type, args, NULL);
    DEC_REFCNT(args);
    return result;
}
