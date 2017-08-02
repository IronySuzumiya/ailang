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

AiObject *eval_frame(AiFrameObject *f) {
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
    AiThreadState *tstate = threadstate_get();

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
            break;
            
        case LOAD_CONST:
            x = TUPLE_GET_ITEM(consts, oparg);
            INC_REFCNT(x);
            PUSH(x);
            break;

        case STORE_NAME:
            w = TUPLE_GET_ITEM(names, oparg);
            v = POP();
            if (x = f->f_locals) {
                dict_setitem((AiDictObject *)x, w, v);
            }
            DEC_REFCNT(v);
            break;

        case BUILD_MAP:
            x = dict_new();
            PUSH(x);
            break;

        case BUILD_LIST:
            x = list_new(oparg);
            while (--oparg >= 0) {
                w = POP();
                list_setitem((AiListObject *)x, oparg, w);
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
            break;

        case ROT_TWO:
            x = v = POP();
            w = POP();
            PUSH(v);
            PUSH(w);
            break;

        case STORE_SUBSCR:
            x = w = POP();
            v = POP();
            u = POP();
            dict_setitem((AiDictObject *)v, w, u);
            DEC_REFCNT(u);
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            break;

        case LOAD_NAME:
            w = TUPLE_GET_ITEM(names, oparg);
            v = f->f_locals;
            x = dict_getitem((AiDictObject *)v, w);
            if (!x) {
                x = dict_getitem((AiDictObject *)f->f_globals, w);
                if (!x) {
                    x = dict_getitem((AiDictObject *)f->f_builtins, w);
                    if (!x) {
                        FATAL_ERROR("unknown name %s", STRING_AS_CSTRING(w));
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
            x = object_rich_compare_bool(v, w, oparg);
            DEC_REFCNT(w);
            DEC_REFCNT(v);
            PUSH(x);
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
            break;

        case SETUP_LOOP:
        case SETUP_EXCEPT:
        case SETUP_FINALLY:
            frame_setup_block(f, opcode, INSTR_OFFSET() + oparg, STACK_LEVEL());
            break;

        case GET_ITER:
            v = POP();
            x = object_getiter(v);
            if (x) {
                PUSH(x);
            }
            else {
                FATAL_ERROR("object cannot be iterated");
            }
            break;

        case FOR_ITER:
            v = TOP();
            if (!CHECK_TYPE_ITER(v)) {
                FATAL_ERROR("only iterator can be index of for-loop");
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
            break;

        case POP_BLOCK:
        {
            AiTryBlock *b = frame_pop_block(f);
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
            retval = int_from_long(oparg);
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
            else if (exceptionclass_check(v) || CHECK_TYPE_STRING(v)) {
                w = POP();
                u = POP();
                exception_restore(v, w, u);
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
            x = function_new(v, f->f_globals);
            DEC_REFCNT(v);
            // TODO
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

        case BINARY_ADD:
        {
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            numbermethods *nb;
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
            traceback_here(f);
        }
        if (why == WHY_RERAISE) {
            why = WHY_EXCEPTION;
        }
        while (why != WHY_NOT && f->f_iblock > 0) {
            AiTryBlock *b = frame_peek_block(f);
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
                    exception_fetch(&type, &val, &tb);
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
                    v = int_from_long((long)why);
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

enum why_code do_raise(AiObject *type, AiObject *value, AiObject *tb) {
    if (!type) {
        AiThreadState *tstate = threadstate_get();
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
        type = TUPLE_GET_ITEM(type, 0);
        INC_REFCNT(type);
        DEC_REFCNT(tmp);
    }
    exception_restore(type, value, tb);
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
    AiObject *x, *w;

    if (CHECK_TYPE_FUNCTION(func) && nk == 0) {
        // TODO
    }
}
