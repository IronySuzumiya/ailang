#include "../ailang.h"

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
    AiObject *stream = NULL;    /* for PRINT opcodes */
    AiObject **fastlocals, **freevars;
    AiObject *retval = NULL;            /* Return value */
    AiCodeObject *co;
    unsigned char *first_instr;
    AiObject *names;
    AiObject *consts;
    char *filename;
    AiThreadState *tstate = NULL; // for now

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
        }

        case BREAK_LOOP:
            why = WHY_BREAK;
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

        if (why != WHY_NOT && f->f_iblock > 0) {
            AiTryBlock *b = frame_pop_block(f);
            while (STACK_LEVEL() > b->b_level) {
                v = POP();
                XDEC_REFCNT(v);
            }
            if (b->b_type == SETUP_LOOP && why == WHY_BREAK) {
                why = WHY_NOT;
                JUMPTO(b->b_handler);
            }
        }
    }
}
