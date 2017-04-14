/* $CORTO_GENERATED
 *
 * vm.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <corto/vm/vm.h>

/* $header() */

#include "corto/vm/vm.h"
#include "corto/vm/vm_operands.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label"
#pragma GCC diagnostic ignored "-pedantic"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#include "signal.h"

static corto_threadKey corto_stringConcatCacheKey;
static corto_threadKey corto_currentProgramKey;
typedef struct corto_vm_context corto_vm_context;

int CORTO_PROCEDURE_VM;

/* TLS structure for corto_currentProgramKey, used for debugging */
typedef struct corto_currentProgramData {
    vm_program stack[64];
    corto_vm_context *c[64];
    corto_uint32 sp;
}corto_currentProgramData;

/* TLS structure for corto_stringConcatCacheKey */
typedef struct corto_stringConcatCacheNode {
    vm_program program;
    corto_string str;
    corto_uint32 length;
}corto_stringConcatCacheNode;

typedef struct corto_stringConcatCache {
    corto_stringConcatCacheNode staged[256]; /* Limit the amount of allocations required for a
                                             stringconcatenation involving multiple elements
                                             with a factor 256 */
    corto_uint32 count;
    corto_uint32 length;
}corto_stringConcatCache;

/* Translation */
#define toJump(in) (((intptr_t)&&in))

/* Program control */
#define go() goto *(void*)(c.pc->op);
#define next() c.pc++; go()
#define jump(r) c.pc = (vm_op*)r; go()
#define fetchIc() c.ic = c.pc->ic
#define fetchLo() c.lo = c.pc->lo
#define fetchHi() c.hi = c.pc->hi
#define fetchDbl() c.dbl = *(corto_int64*)&c.pc->lo

/* Instruction implementation templates */
#define fetchOp1(op,code)\
        fetch_##code;\

#define fetchOp2(op,code)\
        fetch_##code;\
        fetch1_##code;\
        fetch2_##code;\

#define fetchOp3(op,code)\
        fetch_##code;\
        fetch1_##code;\
        fetch2_##code;\
        fetch3_##code;\


/* ---- Instruction implementations */
/* Set */
#define SET(type, code)\
    SET_##code:\
        fetchOp2(SET,code);\
        op1_##code = op2_##code;\
        next();

#define SETREF(type, code)\
    SETREF_##code:{\
        fetchOp2(SETREF,code);\
        corto_setref((corto_object*)&op1_##code, (corto_object)op2_##code);\
    }\
    next();

#define SETSTR(type, code)\
    SETSTR_##code:\
        fetchOp2(SETSTR,code);\
        if (op1_##code) corto_dealloc((corto_string)op1_##code);\
        op1_##code = op2_##code;\
        next();

#define SETSTRDUP(type, code)\
    SETSTRDUP_##code:\
        fetchOp2(SETSTRDUP,code);\
        if (op1_##code) corto_dealloc((corto_string)op1_##code);\
        if (op2_##code) {\
            op1_##code = (W_t)corto_strdup((corto_string)op2_##code);\
        } else {\
            op1_##code = 0;\
        }\
        next();

#define SETX(type, code)\
    SETX_##code:\
        fetchOp2(SETX, code)\
        op1_##code = (W_t)&op2_##code;\
        next();

#define ZERO(type, code)\
    ZERO_##code:\
        fetchOp2(ZERO, code)\
        memset(&op1_WRV,0,op2_WRV);\
        next();

#define INIT(type, code)\
    INIT_##code: {\
        fetchOp2(INIT, code)\
        corto_value v = corto_value_value((corto_type)op2_##code, &op1_##code);\
        corto_initv(&v);\
        next();\
    }

#define DEINIT(type, code)\
    DEINIT_##code: {\
        fetchOp2(DEINIT, code)\
        corto_value v = corto_value_value((corto_type)op2_##code, &op1_##code);\
        corto_deinitv(&v);\
        next();\
    }

/* Inc & dec */
#define INC(type,code)\
    INC_##code:\
        fetchOp1(INC,code);\
        ++ op1_##code;\
        next();\

#define DEC(type,code)\
    DEC_##code:\
        fetchOp1(DEC,code);\
        -- op1_##code;\
        next();\

/* Integer arithmetic */
#define ADDI(type,code)\
    ADDI_##code:\
        fetchOp2(ADDI,code);\
        op1_##code += op2_##code;\
        next();\

#define SUBI(type,code)\
    SUBI_##code:\
        fetchOp2(SUBI,code);\
        op1_##code -= op2_##code;\
        next();\

#define MULI(type,code)\
    MULI_##code:\
        fetchOp2(MULI,code);\
        op1_##code *= op2_##code;\
        next();\

#define DIVI(type,code)\
    DIVI_##code:\
        fetchOp2(DIVI,code);\
        op1_##code /= op2_##code;\
        next();\

#define MODI(type,code)\
    MODI_##code:\
        fetchOp2(MODI,code);\
        op1_##code %= op2_##code;\
        next();\

/* Floating point arithmetic */
typedef float Lf_t;
typedef double Df_t;

/* Union for converting between int-operand and float */
typedef union Li2f_t {
    L_t _i;
    Lf_t _f;
}Li2f_t;

typedef union Di2f_t {
    D_t _i;
    Df_t _f;
}Di2f_t;

#define ADDF(type,code)\
    ADDF_##code: {\
        type##i2f_t u;\
        fetchOp2(ADDF,code);\
        u._i = op2_##code;\
        *(type##f_t*)&op1_##code += u._f;\
        next();\
    }\

#define SUBF(type,code)\
    SUBF_##code: {\
        type##i2f_t u;\
        fetchOp2(SUBF,code);\
        u._i = op2_##code;\
        *(type##f_t*)&op1_##code -= u._f;\
        next();\
    }\

#define MULF(type,code)\
    MULF_##code: {\
        type##i2f_t u;\
        fetchOp2(MULF,code);\
        u._i = op2_##code;\
        *(type##f_t*)&op1_##code *= u._f;\
        next();\
}\

#define DIVF(type,code)\
    DIVF_##code: {\
        type##i2f_t u;\
        fetchOp2(DIVF,code);\
        u._i = op2_##code;\
        *(type##f_t*)&op1_##code /= u._f;\
        next();\
    }\

/* Bitwise operators */
#define AND(type,code)\
    AND_##code:\
        fetchOp2(AND,code);\
        op1_##code &= op2_##code;\
        next();\

#define XOR(type,code)\
    XOR_##code:\
        fetchOp2(XOR,code);\
        op1_##code ^= op2_##code;\
        next();\

#define OR(type,code)\
    OR_##code:\
        fetchOp2(OR,code);\
        op1_##code |= op2_##code;\
        next();\

#define NOT(type,code)\
    NOT_##code:\
        fetchOp1(NOT,code);\
        op1_##code = ~op1_##code;\
        next();\

#define SHIFT_LEFT(type, code)\
    SHIFT_LEFT_##code:\
        fetchOp2(SHIFT_LEFT, code);\
        op1_##code <<= op2_##code;\
        next();\

#define SHIFT_RIGHT(type, code)\
    SHIFT_RIGHT_##code:\
        fetchOp2(SHIFT_RIGHT, code);\
        op1_##code >>= op2_##code;\
        next();\

/* Staging */
#define STAGE1(type,code)\
    STAGE1_##code:\
        fetchOp1(STAGE1,code);\
        stage1_##type = op1_##code;\
        next();\

#define STAGE2(type,code)\
    STAGE2_##code: {\
        fetchOp2(STAGE2,code);\
        type##_t tmp1 = op1_##code;\
        stage2_##type = op2_##code;\
        stage1_##type = tmp1;\
        next();\
    }\


/* Expand compare operators for all stagetypes */
#define COND_OP_STAGETYPE(op, type, code, stageType, operator, cast)\
    op##_##code: \
        fetchOp1(op,code);\
        op1_##code = (cast)stage1_##stageType operator (cast)stage2_##stageType;\
        next();\

#define COND_OP_LD(op,type,code,operator,sign)\
    COND_OP_STAGETYPE(op##L,type,code,L,operator,L##sign##_t)\
    COND_OP_STAGETYPE(op##D,type,code,D,operator,D##sign##_t)

#define COND_OP(op,type,code,operator,sign)\
    COND_OP_STAGETYPE(op##B,type,code,B,operator,B##sign##_t)\
    COND_OP_STAGETYPE(op##S,type,code,S,operator,S##sign##_t)\
    COND_OP_LD(op,type,code,operator,sign)\


#define COND_OP1_STAGETYPE(op, type, code, stageType, operator)\
    op##_##code:\
        fetchOp1(op,code);\
        op1_##code = operator stage1_##stageType;\
        next();\

#define COND_OP1(op,type,code,operator)\
    COND_OP1_STAGETYPE(op##B,type,code,B,operator)\
    COND_OP1_STAGETYPE(op##S,type,code,S,operator)\
    COND_OP1_STAGETYPE(op##L,type,code,L,operator)\
    COND_OP1_STAGETYPE(op##D,type,code,D,operator)

/* Compare operators */
#define CAND(type,code) COND_OP(CAND,type,code,&&,)
#define COR(type,code) COND_OP(COR,type,code,||,)
#define CNOT(type,code) COND_OP1(CNOT,type,code,!)
#define CEQ(type,code) COND_OP(CEQ,type,code,==,)
#define CNEQ(type,code) COND_OP(CNEQ,type,code,!=,)

/* Signed comparisons */
#define CGTI(type,code) COND_OP(CGTI,type,code,>,s)
#define CLTI(type,code) COND_OP(CLTI,type,code,<,s)
#define CGTEQI(type,code) COND_OP(CGTEQI,type,code,>=,s)
#define CLTEQI(type,code) COND_OP(CLTEQI,type,code,<=,s)

/* Unsigned comparisons */
#define CGTU(type,code) COND_OP(CGTU,type,code,>,)
#define CLTU(type,code) COND_OP(CLTU,type,code,<,)
#define CGTEQU(type,code) COND_OP(CGTEQU,type,code,>=,)
#define CLTEQU(type,code) COND_OP(CLTEQU,type,code,<=,)

/* Floating point comparisons */
#define CGTF(type,code) COND_OP_LD(CGTF,type,code,>,f)
#define CLTF(type,code) COND_OP_LD(CLTF,type,code,<,f)
#define CGTEQF(type,code) COND_OP_LD(CGTEQF,type,code,>=,f)
#define CLTEQF(type,code) COND_OP_LD(CLTEQF,type,code,<=,f)

#define CEQSTR(type,code)\
    CEQSTR_##code:\
        fetchOp1(CEQSTR,code);\
        if (stage1_W && stage2_W) {\
            op1_##code = !strcmp((corto_string)stage1_W, (corto_string)stage2_W);\
        } else {\
            op1_##code = stage1_W == stage2_W;\
        }\
        next();

#define CNEQSTR(type,code)\
    CNEQSTR_##code:\
        fetchOp1(CNEQSTR,code);\
        if (stage1_W && stage2_W) {\
            op1_##code = strcmp((corto_string)stage1_W, (corto_string)stage2_W) != 0;\
        } else {\
            op1_##code = stage1_W != stage2_W;\
        }\
        next();

#define JEQ(type,code)\
    JEQ_##code:\
        fetch_##code;\
        fetch1_##code;\
        if (op1_##code) {\
            fetch2_##code;\
            jump(op2_##code);\
        }\
        next();\

#define JNEQ(type,code)\
    JNEQ_##code:\
        fetch_##code;\
        fetch1_##code;\
        if (op1_##code) {\
            next();\
        }\
        fetch2_##code;\
        jump(op2_##code);\

#define PUSH(type,code)\
    PUSH_##code:\
        fetchOp1(PUSH,code);\
        *(type##_t*)c.sp = op1_##code;\
        c.sp = CORTO_OFFSET(c.sp, sizeof(type##_t));\
        next();\

#define PUSHX(type,code)\
    PUSHX_##code:\
        fetchOp1(PUSHX,code);\
        *(corto_word*)c.sp = (corto_word)&op1_##code;\
        c.sp = CORTO_OFFSET(c.sp, sizeof(corto_word));\
        next();\

#define _PUSHANY(opx,_type,code,deref,_pc,v)\
    PUSHANY##opx##_##code:\
    fetchOp2(PUSHANY,code);\
    ((corto_any*)c.sp)->type = (corto_type)op2_##code;\
    ((corto_any*)c.sp)->value = (void*)deref _pc op1##v##_##code;\
    ((corto_any*)c.sp)->owner = FALSE;\
    c.sp = CORTO_OFFSET(c.sp, sizeof(corto_any));\
    next();\

#define PUSHANYX(_type,code)  _PUSHANY(X,_type,code,&,,)
#define PUSHANYV(_type,code)  _PUSHANY(V,_type,code,&,c.pc->,x)
#define PUSHANY(_type,code)   _PUSHANY(,_type,code,(corto_word),,)

#define CALL(type,code)\
    CALL_##code:\
        fetchOp1(CALL,code);\
        fetchHi();\
        corto_callb((corto_function)c.hi.w, &op1_##code, c.stack);\
        c.sp = c.stack; /* Reset stack pointer */\
        next();\

#define CALLVM(type,code)\
    CALLVM_##code:\
        fetchOp1(CALLVM,code);\
        fetchHi();\
        corto_vm_run_w_storage((vm_program)((corto_function)c.hi.w)->fptr, c.stack, &op1_##code);\
        c.sp = c.stack; /* Reset stack pointer */\
        next();\

#define CALLVOID()\
    CALLVOID:\
        fetchHi();\
        corto_callb((corto_function)c.hi.w, NULL, c.stack);\
        c.sp = c.stack; /* Reset stack pointer */\
        next();\

#define CALLVMVOID()\
    CALLVMVOID:\
        fetchHi();\
        corto_vm_run_w_storage((vm_program)((corto_function)c.hi.w)->fptr, c.stack, NULL);\
        c.sp = c.stack; /* Reset stack pointer */\
        next();\

#define CALLPTR(type,code)\
    CALLPTR_##code: {\
        fetchOp2(CALLPTR,code);\
        corto_delegatedata *ptr = (corto_delegatedata*)&op2_##code;\
        void *stackptr = c.stack;\
        if (!ptr->instance) {\
            stackptr = CORTO_OFFSET(stackptr, sizeof(corto_word));\
        }\
        corto_callb((corto_function)ptr->procedure, &op1_##code, stackptr);\
        c.sp = c.stack; /* Reset stack pointer */ \
        next();\
    }\

#define RET(type,code)\
    RET_##code:\
        fetchOp1(RET,code);\
        *(type##_t*)result = op1_##code;\
        goto STOP;

#define RETCPY(type,code)\
    RETCPY_##code:\
        fetchOp1(RETCPY,code);\
        fetchHi();\
        memcpy((type##_t*)result, &op1_##code, c.hi.w);\
        goto STOP;

#define CAST(type,code)\
    CAST_##code:\
        fetchOp2(CAST,code)\
        if (op1_##code) {\
            if (!corto_instanceof((corto_type)op2_##code, (corto_object)op1_##code)) {\
                printf("Exception: invalid cast from type '%s' to '%s'\n", \
                    corto_fullpath(NULL, (corto_object)op2_##code), \
                    corto_fullpath(NULL, corto_typeof((corto_object)op1_##code)));\
                    goto STOP;\
            }\
        }\
        next();\

#define PCAST(type,code)\
    PCAST_##code: {\
        fetchOp2(PCAST,code)\
        corto_type fromType = (corto_type)stage1_W;\
        if (fromType->reference) {\
            fromType = (corto_type)corto_word_o;\
        }\
        corto_convert((corto_primitive)fromType, &op2_##code, (corto_primitive)stage2_W, &op1_##code);\
        next();\
    }

#define STRCAT(type,code)\
    STRCAT_##code:\
    {\
        corto_string str1,str2;\
        fetchOp2(STRCAT,code)\
        if ((str1 = (corto_string)op1_##code)) {\
            c.strcache->length += c.strcache->staged[c.strcache->count].length = strlen(str1);\
            c.strcache->staged[c.strcache->count].str = str1;\
            c.strcache->count++;\
        }\
        if ((str2 = (corto_string)op2_##code)) {\
            c.strcache->length += c.strcache->staged[c.strcache->count].length = strlen(str2);\
            c.strcache->staged[c.strcache->count].str = str2;\
            c.strcache->count++;\
        }\
        next();\
    }\

#define STRCPY(type,code)\
    STRCPY_##code:\
    {\
        corto_string result, ptr, str;\
        corto_uint32 i;\
        fetchOp2(STRCPY,code);\
        corto_uint32 length=0;\
        if ((str = (corto_string)op2_##code)) {\
            c.strcache->length += c.strcache->staged[c.strcache->count].length = strlen(str);\
            c.strcache->staged[c.strcache->count].str = str;\
            c.strcache->count++;\
        }\
        result = corto_alloc(c.strcache->length + 1);\
        ptr = result;\
        for(i=0; i<c.strcache->count; i++) {\
            length = c.strcache->staged[i].length;\
            memcpy(ptr, c.strcache->staged[i].str, length);\
            ptr += length;\
        }\
        *ptr = '\0';\
        c.strcache->count = 0;\
        c.strcache->length = 0;\
        op1_##code = (W_t)result;\
        next();\
    }\

#define NEW(type,code)\
    NEW_##code:\
        fetchOp2(NEW,code);\
        op1_##code = (corto_word)corto_declare((corto_object)op2_##code);\
        next();\

#define DEALLOC(type,code)\
    DEALLOC_##code:\
        fetchOp1(DEALLOC,code);\
        corto_dealloc((void*)op1_##code);\
        next();\

#define KEEP(type,code)\
    KEEP_##code:\
        fetchOp1(KEEP,code);\
        if (op1_##code) {\
            corto_claim((corto_object)op1_##code);\
        }\
        next();\

#define FREE(type,code)\
    FREE_##code:\
        fetchOp1(FREE,code);\
        if (op1_##code) {\
            corto_release((corto_object)op1_##code);\
        }\
        next();\

#define DEFINE(type,code)\
    DEFINE_##code:\
        fetchOp1(DEFINE,code);\
        if (!op1_##code) {\
            corto_error("Exception: null dereference in define");\
            goto STOP;\
        }\
        if (corto_define((corto_object)op1_##code)) {\
            exception = -1;\
            goto STOP;\
        };\
        next();\

#define UPDATE(type,code)\
    UPDATE_##code:\
        fetchOp1(UPDATE,code);\
        if (!op1_##code) {\
            corto_error("Exception: null dereference in updateFrom");\
            goto STOP;\
        }\
        if (corto_update((corto_object)op1_##code)) {\
            corto_error("Exception: %s", corto_lasterr());\
            goto error;\
        }\
        next();\

#define UPDATEBEGIN(type,code)\
    UPDATEBEGIN_##code:\
        fetchOp1(UPDATEBEGIN,code);\
        if (!op1_##code) {\
            corto_error("Exception: null dereference in updateBegin");\
            goto STOP;\
        }\
        if (corto_updateBegin((corto_object)op1_##code)) {\
            corto_error("Exception: %s", corto_lasterr());\
            goto error;\
        }\
        next();\

#define UPDATEEND(type,code)\
    UPDATEEND_##code:\
        fetchOp1(UPDATEEND,code);\
        if (!op1_##code) {\
            corto_error("Exception: null dereference in updateEnd");\
            goto STOP;\
        }\
        corto_updateEnd((corto_object)op1_##code);\
        next();\

#define UPDATEFROM(type,code)\
    UPDATEFROM_##code:\
        fetchOp2(UPDATEFROM, code);\
        if (!op1_##code) {\
            printf("Exception: null dereference in updateFrom\n");\
            goto STOP;\
        }\
        {\
            corto_object prev = corto_setOwner((corto_object)op2_##code);\
            corto_update((corto_object)op1_##code);\
            corto_setOwner(prev);\
        }\
        next();\

#define UPDATEENDFROM(type,code)\
    UPDATEENDFROM_##code:\
        fetchOp2(UPDATEENDFROM,code);\
        if (!op1_##code) {\
            printf("Exception: null dereference in updateEndFrom\n");\
            abort();\
            goto STOP;\
        }\
        {\
            corto_object prev = corto_setOwner((corto_object)op2_##code);\
            corto_updateEnd((corto_object)op1_##code);\
            corto_setOwner(prev);\
        }\
        next();\

#define UPDATECANCEL(type,code)\
    UPDATECANCEL_##code:\
        fetchOp1(UPDATECANCEL,code);\
        if (!op1_##code) {\
            printf("Exception: null dereference in updateCancel\n");\
            abort();\
            goto STOP;\
        }\
        corto_updateCancel((corto_object)op1_##code);\
        next();\

#ifdef CORTO_VM_BOUNDSCHECK
#define CHECK_BOUNDS(size, index)\
    if ((int)size <= (int)index) {\
        printf("Exception: element [%" PRIdPTR "] is out of bounds (collection size is %" PRId32 ")\n", index, size);\
        abort();\
        goto STOP;\
    }
#else
#define CHECK_BOUNDS(size, index)
#endif

#define ELEMA(type,code)\
    ELEMA_##code:\
        fetchOp3(ELEMA,code##V);\
        op1_##code##V += (L_t)op2_##code##V * op3_##code##V;\
        next();\

#define ELEMS(type,code)\
    ELEMS_##code:\
        fetchOp3(ELEMS,code##V);\
        {\
            corto_objectseq* seq = (corto_objectseq*)op1_##code##V;\
            CHECK_BOUNDS(seq->length, op2_##code##V);\
            op1_##code##V = (W_t)CORTO_OFFSET(seq->buffer, (L_t)op2_##code##V * op3_##code##V);\
        }\
        next();\

#define ELEML(type,code)\
    ELEML_##code:\
        fetchOp2(ELEML, code);\
        CHECK_BOUNDS(corto_llSize(*(corto_ll*)op1_##code), op2_##code)\
        op1_##code = (W_t)corto_llGet(*(corto_ll*)op1_##code, op2_##code);\
        next();

#define ELEMLX(type,code)\
    ELEMLX_##code:\
        fetchOp2(ELEMLX, code);\
        CHECK_BOUNDS(corto_llSize(*(corto_ll*)op1_##code), op2_##code)\
        op1_##code = (W_t)corto_llGetPtr(*(corto_ll*)op1_##code, op2_##code);\
        next();\

#define ELEMM(type,code)\
    ELEMM_##code:\
        fetchOp2(ELEMM, code);\
        op1_##code = (W_t)corto_rbtreeGet(*(corto_rbtree*)op1_##code, (void*)&op2_##code);\
        next();\

#define ELEMMX(type,code)\
    ELEMMX_##code:\
        fetchOp2(ELEMMX, code);\
        op1_##code = (W_t)corto_rbtreeGetPtr(*(corto_rbtree*)op1_##code, (void*)&op2_##code);\
        next();\

#define ITER_SET(type,code)\
    ITER_SET_##code:\
        fetchOp3(ITER_SET, code##V);\
        /* TODO */\
        next();\

/* op1 is hasNext, op2 is the result of next and op3 is the iterator */
#define ITER_NEXT(type,code)\
    ITER_NEXT_##code:\
        fetchOp2(ITER_NEXT, code);\
        /* TODO */\
        next();\

#define JUMP(type, code)\
    JUMP_##code:\
        fetchOp1(JUMP, code);\
        jump(op1_##code);

#define MEMBER()\
    MEMBER:\
        fetch_LRR;\
        fetch1_LRR;\
        fetch2_LRR;\
        fetchLo();\
        if (!op2_LRR) {\
            printf("Error: dereferencing null\n");\
        }\
        op1_WRR = op2_WRR + c.lo.w;\
        next();

#define STOP()\
    STOP:\
        corto_vm_popSignalHandler();\
        return exception;\

/* ---- */

/* ---- Instruction expansion macro's */
#define INSTR0(arg)\
    arg()

#define INSTR1(type, arg, op1)\
    arg(type, type##op1)

#define INSTR1_COND(type, arg, op1)\
    arg(type, type##op1)

#define INSTR1_COND_LD(type, arg, op1)\
    arg(type, type##op1)

#define INSTR2(type, arg, op1, op2)\
    arg(type, type##op1##op2)

#define INSTR3(type, arg, op1, op2, op3)\
    arg(type, type##op1##op2##op3)
/* ---- */

/* ---- Jump expansion macro's */
#define JUMP0(arg)\
    case CORTO_VM_##arg: p[i].op = toJump(arg); break;

#define JUMP1(type, arg, op1)\
    case CORTO_VM_##arg##_##type##op1: p[i].op = toJump(arg##_##type##op1); break;

#define JUMP1_COND(type, arg, op1)\
    case CORTO_VM_##arg##B_##type##op1: p[i].op = toJump(arg##B_##type##op1); break;\
    case CORTO_VM_##arg##S_##type##op1: p[i].op = toJump(arg##S_##type##op1); break;\
    case CORTO_VM_##arg##L_##type##op1: p[i].op = toJump(arg##L_##type##op1); break;\
    case CORTO_VM_##arg##D_##type##op1: p[i].op = toJump(arg##D_##type##op1); break;

#define JUMP1_COND_LD(type, arg, op1)\
    case CORTO_VM_##arg##L_##type##op1: p[i].op = toJump(arg##L_##type##op1); break;\
    case CORTO_VM_##arg##D_##type##op1: p[i].op = toJump(arg##D_##type##op1); break;

#define JUMP2(type, arg, op1, op2)\
    case CORTO_VM_##arg##_##type##op1##op2: p[i].op = toJump(arg##_##type##op1##op2); break;

#define JUMP3(type, arg, op1, op2, op3)\
    case CORTO_VM_##arg##_##type##op1##op2##op3: p[i].op = toJump(arg##_##type##op1##op2##op3); break;
/* ---- */

/* ---- Jump expansion macro's */
#define STRING0(arg)\
    case CORTO_VM_##arg: result = vm_opToString(result, &p[i], #arg, "", "", "", ""); break;\

#define STRING1(type, arg, op1)\
    case CORTO_VM_##arg##_##type##op1: result = vm_opToString(result, &p[i], #arg, #type, #op1, "", ""); break;\

#define STRING1_COND(type, arg, op1)\
    case CORTO_VM_##arg##B_##type##op1: result = vm_opToString(result, &p[i], #arg "B", #type, #op1, "", ""); break;\
    case CORTO_VM_##arg##S_##type##op1: result = vm_opToString(result, &p[i], #arg "S", #type, #op1, "", ""); break;\
    case CORTO_VM_##arg##L_##type##op1: result = vm_opToString(result, &p[i], #arg "L", #type, #op1, "", ""); break;\
    case CORTO_VM_##arg##D_##type##op1: result = vm_opToString(result, &p[i], #arg "D", #type, #op1, "", ""); break;\

#define STRING1_COND_LD(type, arg, op1)\
    case CORTO_VM_##arg##L_##type##op1: result = vm_opToString(result, &p[i], #arg "L", #type, #op1, "", ""); break;\
    case CORTO_VM_##arg##D_##type##op1: result = vm_opToString(result, &p[i], #arg "D", #type, #op1, "", ""); break;\

#define STRING2(type, arg, op1, op2)\
    case CORTO_VM_##arg##_##type##op1##op2: result = vm_opToString(result, &p[i], #arg, #type, #op1, #op2, ""); break;\

#define STRING3(type, arg, op1, op2, op3)\
    case CORTO_VM_##arg##_##type##op1##op2##op3: result = vm_opToString(result, &p[i], #arg, #type, #op1, #op2, #op3); break;\
/* ---- */

struct corto_vm_context {
    vm_op *pc; /* Instruction counter */
    vm_parameter16 ic; /* First parameter */
    vm_parameter lo; /* Lo parameter */
    vm_parameter hi; /* Hi parameter */
    corto_uint64 dbl; /* Double parameter */
    corto_uint64 dbl2;
    vm_parameter stage1;
    vm_parameter stage2;
    void *stack, *sp;
    corto_stringConcatCache *strcache;

    /* Reserved space for interrupt program in case of SIGINT */
    vm_op interrupt[2];
};

#ifdef CORTO_VM_DEBUG
typedef void (*sigfunc)(int sig);
static sigfunc prevSegfaultHandler;
static sigfunc prevAbortHandler;
static sigfunc prevInterruptHandler;

sigfunc safe_signal (int sig, sigfunc h) {
    struct sigaction sa;
    struct sigaction osa;
    sa.sa_handler = h;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(sig, &sa, &osa) < 0) {
        return SIG_ERR;
    }
    return osa.sa_handler;
}

static void corto_vm_sigHardAbort(int sig) {
    CORTO_UNUSED(sig);
    exit(-1);
}

/* The VM signal handler */
static void corto_vm_sig(int sig) {
    corto_int32 sp;

    /* Unblock all signals */
    sigset_t mask_set, old_set;
    sigfillset(&mask_set);
    sigprocmask(SIG_UNBLOCK, &mask_set, &old_set);

    /* If any signal occurs again, do a hard abort */
    int i;
    for (i = 1; i < 35; i++) {
        signal(i, corto_vm_sigHardAbort);
    }

    corto_currentProgramData *programData = corto_threadTlsGet(corto_currentProgramKey);

    if ((sig == SIGSEGV) || (sig == SIGBUS)) {
        printf("Access violation (%d)\n", sig);
    }
    if (sig == SIGABRT) {
        printf("Abort\n");
    }
    if (sig == SIGINT) {
        for(sp = programData->sp-1; sp>=0; sp--) {
            vm_program program = programData->stack[sp];
            programData->c[sp]->interrupt[0].op = program->program[program->size-1].op;
            programData->c[sp]->interrupt[1].op = program->program[program->size-1].op;
            programData->c[sp]->pc = programData->c[sp]->interrupt;
        }
        return;
    }

    /* Walk the stack, print frames */
    for(sp = programData->sp-1; sp>=0; sp--) {
        corto_id file;
        vm_program program = programData->stack[sp];

        corto_uint32 line = program->debugInfo[((corto_word)programData->c[sp]->pc - (corto_word)program->program)/sizeof(vm_op)].line;

        if (program->filename) {
            sprintf(file, "%s:", program->filename);
        } else {
            *file = '\0';
        }
        if (program->function) {
            printf("[%d] %s (%s%d)\n", sp+1, corto_fullpath(NULL, program->function), file, line);
        } else {
            printf("[%d] <main> (%s%d)\n", sp+1, file, line);
        }

        /* Print program with location of crash */
#ifdef CORTO_IC_TRACING
        if(sp == (corto_int32)programData->sp-1) {
            corto_string str = vm_programToString(program, programData->c[sp]->pc);
            printf("\n%s\n", str);
            corto_dealloc(str);
        }
#endif
    }

    printf("\n");
    exit(-1);
}

/* Push a program to the exception stack (see pushSignalHandler) */
static void corto_vm_pushCurrentProgram(vm_program program, corto_vm_context *c) {
    corto_currentProgramData *data = NULL;
    if (!corto_currentProgramKey) {
        corto_threadTlsKey(&corto_currentProgramKey, NULL);
    }
    data = corto_threadTlsGet(corto_currentProgramKey);
    if (!data) {
        data = corto_alloc(sizeof(corto_currentProgramData));
        data->sp = 0;
        corto_threadTlsSet(corto_currentProgramKey, data);
    }
    data->stack[data->sp] = program;
    data->c[data->sp] = c;
    data->sp++;
}

/* Pop a program from the exception stack */
static void corto_vm_popCurrentProgram(void) {
    corto_currentProgramData *data = corto_threadTlsGet(corto_currentProgramKey);
    data->sp--;
}

/* Push a program to the signal handler stack. This will allow backtracing the
 * stack when an error occurs. */
static void corto_vm_pushSignalHandler(vm_program program, corto_vm_context *c) {
    sigfunc result = safe_signal(SIGSEGV, corto_vm_sig);
    if (result == SIG_ERR) {
        corto_error("failed to install signal handler for SIGSEGV");
    } else {
        prevSegfaultHandler = result;
    }

    result = safe_signal(SIGBUS, corto_vm_sig);
    if (result == SIG_ERR) {
        corto_error("failed to install signal handler for SIGSEGV");
    } else {
        prevSegfaultHandler = result;
    }

    result = safe_signal(SIGABRT, corto_vm_sig);
    if (result == SIG_ERR) {
        corto_error("failed to install signal handler for SIGABRT");
    } else {
        prevAbortHandler = result;
    }

    result = safe_signal(SIGINT, corto_vm_sig);
    if (result == SIG_ERR) {
        corto_error("failed to install signal handler for SIGINT");
    } else {
        prevInterruptHandler = result;
    }

    /* Store current program in TLS */
    corto_vm_pushCurrentProgram(program, c);
}

/* Pop a program from the signal handler stack */
static void corto_vm_popSignalHandler(void) {
    if (safe_signal(SIGSEGV, prevSegfaultHandler) == SIG_ERR) {
        corto_error("failed to uninstall signal handler for SIGSEGV");
    } else {
        prevSegfaultHandler = NULL;
    }
    if (safe_signal(SIGABRT, prevAbortHandler) == SIG_ERR) {
        corto_error("failed to uninstall signal handler for SIGABRT");
    } else {
        prevAbortHandler = NULL;
    }
    if (safe_signal(SIGINT, prevInterruptHandler) == SIG_ERR) {
        corto_error("failed to uninstall signal handler for SIGINT");
    } else {
        prevInterruptHandler = NULL;
    }

    corto_vm_popCurrentProgram();
}
#else
#define corto_vm_pushSignalHandler(p,c)
#define corto_vm_popSignalHandler()
#endif

static int32_t corto_vm_run_w_storage(vm_program program, void* reg, void *result) {
    corto_vm_context c;
    c.strcache = corto_threadTlsGet(corto_stringConcatCacheKey);
    int exception = 0;

    /* The signal handler will catch any exceptions and report when (and where)
     * an error is occurring */
    corto_vm_pushSignalHandler(program, &c);

    /* Translate program if required
     * This will translate from the VM instruction codes (the constants from
     * the corto_vm_opKind enumeration) to the actual addresses of the
     * implementations. This allows the execution of code to jump directly
     * from one instruction to the next, thereby skipping the overhead of
     * an evaluation-then-jump construction like a switch statement. */
    if (!program->translated)  {
        uint32_t size = program->size;
        vm_op *p = program->program;
        uint32_t i;
        for(i=0; i<size;i++) {
#ifdef CORTO_VM_DEBUG
            p[i].opKind = p[i].op; /* Cache actual opKind for debugging purposes */
#endif
            switch(p[i].op) {
                /* ---- Expand jump macro's */
                OPS_EXP(JUMP)
                default:
                    corto_assert(0, "invalid instruction in sequence %d @ %d", p[i].op, i);
                    break;
            }
        }
        program->translated = TRUE;
    }

    /* Prepare context */
    c.stack = alloca(program->stack);
    c.sp = c.stack;
    c.pc = program->program;

    /* Run program */
    go();

    /* ---- Expand instruction macro's */
    OPS_EXP(INSTR);

error:
    return -1;
}

/* Delete a string concatenation cache (cleanup function for thread
 * specific memory) */
static void corto_stringConcatCacheClean(void *data) {
    corto_dealloc(data);
}

/* Create a string concatenation cache */
static void corto_stringConcatCacheCreate(void) {
    corto_stringConcatCache *concatCache;
    if (!corto_stringConcatCacheKey) {
        corto_threadTlsKey(&corto_stringConcatCacheKey, corto_stringConcatCacheClean);
    }

    concatCache = corto_threadTlsGet(corto_stringConcatCacheKey);
    if (!concatCache) {
        concatCache = corto_alloc(sizeof(corto_stringConcatCache));
        memset(concatCache, 0, sizeof(corto_stringConcatCache));
        corto_threadTlsSet(corto_stringConcatCacheKey, concatCache);
    }
}

/* Execute a program */
int32_t vm_run(vm_program program, corto_stringSeq argv, void *result) {
    void *storage = NULL;
    if (program->storage) {
        storage = alloca(program->storage);
    }
    *(void**)storage = &argv;
    corto_stringConcatCacheCreate();
    return corto_vm_run_w_storage(program, storage, result);
}

/* This function converts a single instruction to a string */
#ifdef CORTO_IC_TRACING
char * vm_opToString(
    char * string, vm_op *instr, const char *op, const char *type, const char *lvalue, const char *rvalue, const char* fetch) {
    char *result = string;

    if (fetch && strlen(fetch)) {
        result = strappend(
            result,
            "%s_%s%s%s_%s %u %u %u %u\n",
            op, type, lvalue, rvalue, fetch, instr->ic.b._1, instr->ic.b._2, instr->lo.w, instr->hi.w);
    } else {
        result = strappend(
            result,
            "%s_%s%s%s %hu %hu %u %u\n",
            op, type, lvalue, rvalue, instr->ic.b._1, instr->ic.b._2, instr->lo.w, instr->hi.w);
    }

    return result;
}
#endif

/* Convert an instruction sequence to a string */
char * vm_programToString(vm_program program, vm_op *addr) {
    char * result = NULL;
    corto_int32 shown = 4;
    CORTO_UNUSED(program);

/* Since these strings can occupy a lot of space, they're only compiled in
 * when these two macros are enabled */
#ifdef CORTO_IC_TRACING
    vm_op *p = program->program;
    uint32_t i;

#ifndef CORTO_VM_DEBUG
    if (!program->translated) {
        printf("cannot convert active program to string with non-debug version\n");
        abort();
    }
#endif
    /* Prefix '...' to string if the code section is not at the start */
    if (addr && ((addr - p) > shown)) {
        result = strappend(result, "  ...\n");
    }

    /* Loop instructions, prefix address */
    for(i=0; i<program->size;i++) {
        corto_int32 diff = addr - &p[i];
        if (!addr || ((diff <= shown) && (diff >= -shown))) {
            vm_opKind kind;

            if (addr) {
                if (addr == &p[i]) {
                    result = strappend(result, "> %u: ", &p[i]);
                } else {
                    result = strappend(result, "  %u: ", &p[i]);
                }
            } else {
                result = strappend(result, "%u: ", &p[i]);
            }
    #ifdef CORTO_VM_DEBUG
            if (program->translated) {
                kind = p[i].opKind;
            } else {
                kind = p[i].op;
            }
    #else
            kind = p[i].op;
    #endif
            switch(kind) {
                /* ---- Expand string conversion macro's */
                OPS_EXP(STRING)
                default:
                    corto_assert(0, "invalid instruction %d in sequence @ %d", p[i].op, i);
                    break;
            }
        }
    }
    /* Append '...' to string if the code section is not at the end */
    if (addr && ((&p[program->size-1] - addr) > 5)) {
        result = strappend(result, "  ...\n");
    }
#endif
    return result;
}

#pragma GCC diagnostic pop

/* Create a new VM program */
vm_program vm_programNew(char *filename, corto_object function) {
    vm_program result;

    result = corto_alloc(sizeof(vm_program_s));
    result->program = NULL;
    result->debugInfo = NULL;
    result->filename = filename ? corto_strdup(filename) : NULL;
    result->function = function;
    result->size = 0;
    result->maxSize = 0;
    result->stack = 0;
    result->storage = 0;
    result->translated = FALSE;

    return result;
}

/* Free a VM program */
void vm_programFree(vm_program program) {
    if (program) {
        if (program->program) {
            corto_dealloc(program->program);
        }
        corto_dealloc(program);
    }
}

/* Add new instruction to a VM program */
vm_op *vm_programAddOp(vm_program program, uint32_t line) {
    /* Try to be smart with memory allocations, don't allocate new memory
     * every time an instruction is added. */
    if (!program->size) {
        program->size = 1;
        program->maxSize = 8;
    } else {
        program->size ++;
        if (program->size > program->maxSize) {
            program->maxSize *= 2;
        }
    }
    program->program = corto_realloc(program->program, program->maxSize * sizeof(vm_op));
    program->debugInfo = corto_realloc(program->debugInfo, program->maxSize * sizeof(vm_debugInfo));

    /* Initialize instruction and debug data to zero */
    memset(&program->program[program->size-1], 0, sizeof(vm_op));
    memset(&program->debugInfo[program->size-1], 0, sizeof(vm_debugInfo));
    program->debugInfo[program->size-1].line = line;

    /* Return potentially reallocd program */
    return &program->program[program->size-1];
}

/* Language binding function that calls a VM function */
static void vm_call(void *fdata, void *fptr, void* result, void** args) {
    vm_program program;
    void *storage = NULL;
    corto_function f = fdata;

    /* Obtain instruction sequence */
    program = (vm_program)fptr;

    /* Allocate a storage for a program. This memory will
     * store all local variables, and space required to
     * prepare a stack for calling functions */
    storage = alloca(program->storage);

    char *ptr = storage;
    corto_int32 i;
    for (i = 0; i < f->parameters.length; i++) {
        corto_type type = f->parameters.buffer[i].type;
        corto_int32 size = type->reference ? sizeof(corto_word) : type->size;
        memcpy(ptr, args[i], size);
        ptr += size;
    }

    /* Thread specific cache that speeds up string concatenations */
    corto_stringConcatCacheCreate();

    /* Execute the instructions */
    corto_vm_run_w_storage(program, storage, result);
}

/* Language binding function that frees a VM function */
void vm_callDestruct(corto_function f) {
    vm_programFree((vm_program)f->fptr);
}

corto_int16 vm_initFunction(corto_function this) {
    this->impl = (corto_word)vm_call;
    this->fdata = (corto_word)this;
    return 0;
}

void vm_deinitFunction(corto_function this) {
    CORTO_UNUSED(this);
}
/* $end */

int vmMain(int argc, char *argv[]) {
/* $begin(main) */
    CORTO_UNUSED(argc);
    CORTO_UNUSED(argv);
    CORTO_PROCEDURE_VM = corto_callRegister(vm_initFunction, vm_deinitFunction);
    return 0;
/* $end */
}
