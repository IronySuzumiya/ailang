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

AiObject *eval_frame(AiFrameObject *f) {
    AiObject **stack_pointer;  /* Next free slot in value stack */
    unsigned char *next_instr;
    int opcode;        /* Current opcode */
    int oparg;         /* Current opcode argument, if any */
    enum why_code why; /* Reason for block stack unwind */
    int err;           /* Error status -- nonzero if error */
    AiObject *x;       /* Result object -- NULL if error */
    AiObject *v;       /* Temporary objects popped off stack */
    AiObject *w;
    AiObject *u;
    AiObject *t;
    AiObject *stream = NULL;    /* for PRINT opcodes */
    AiObject **fastlocals, **freevars;
    AiObject *retval = NULL;            /* Return value */
    AiCodeObject *co;
    unsigned char *first_instr;
    AiObject *names;
    AiObject *consts;
    char *filename;
    AiThreadState *tstate = NULL; // for now

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
        AiObject *tmp = GETLOCAL(i);    \
        GETLOCAL(i) = value;            \
        XDEC_REFCNT(tmp);               \
    )

    if (!f) {
        return NULL;
    }

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
    err = 0;
    x = NONE;
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
            v = TOP();
            INC_REFCNT(v);
            PUSH(v);
            break;

        case ROT_TWO:
            v = POP();
            w = POP();
            PUSH(v);
            PUSH(w);
            break;

        case STORE_SUBSCR:
            w = POP();
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
            w = POP();
            if (w == AiFALSE) {
                JUMPBY(oparg);
            }
            break;

        case POP_JUMP_IF_TRUE:
            w = POP();
            if (w == AiTRUE) {
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
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
                x = NONE;
            }
            DEC_REFCNT(v);
            DEC_REFCNT(w);
            PUSH(x);
            break;
        }

        }
    }
}