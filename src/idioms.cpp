/*****************************************************************************
 *          dcc project machine idiom recognition
 * (C) Cristina Cifuentes
 ****************************************************************************/

#include <cstring>
#include <deque>
#include "idiom.h"
#include "idiom1.h"
#include "epilogue_idioms.h"
#include "call_idioms.h"
#include "mov_idioms.h"
#include "xor_idioms.h"
#include "neg_idioms.h"
#include "shift_idioms.h"
#include "arith_idioms.h"
#include "dcc.h"
#include <boost/iterator/filter_iterator.hpp>
/*****************************************************************************
 * JmpInst - Returns TRUE if opcode is a conditional or unconditional jump
 ****************************************************************************/
bool JmpInst(llIcode opcode)
{
    switch (opcode) {
        case iJMP:  case iJMPF: case iJCXZ:
        case iLOOP: case iLOOPE:case iLOOPNE:
        case iJB:   case iJBE:  case iJAE:  case iJA:
        case iJL:   case iJLE:  case iJGE:  case iJG:
        case iJE:   case iJNE:  case iJS:   case iJNS:
        case iJO:   case iJNO:  case iJP:   case iJNP:
            return true;
    }
    return false;
}

/*****************************************************************************
 * findIdioms  - translates LOW_LEVEL icode idioms into HIGH_LEVEL icodes.
 ****************************************************************************/
void Function::findIdioms()
{
//    Int     ip;             /* Index to current icode                   */
    iICODE  pEnd, pIcode;   /* Pointers to end of BB and current icodes */
    int16   delta;

    pIcode = Icode.begin();
    pEnd = Icode.end();
    Idiom1  i01(this);
    Idiom2  i02(this);
    Idiom3  i03(this);
    Idiom4  i04(this);
    Idiom13 i13(this);
    Idiom14 i14(this);
    Idiom17 i17(this);
    // xor idioms
    Idiom21 i21(this);
    Idiom7 i07(this);
    // or idioms
    Idiom10 i10(this);
    // neg idiom
    Idiom11 i11(this);
    Idiom16 i16(this);
    // shift idioms
    Idiom8 i08(this);
    Idiom12 i12(this);
    Idiom15 i15(this);
    Idiom9 i09(this);
    //arithmetic idioms
    Idiom5 i05(this);
    Idiom6 i06(this);
    Idiom18 i18(this);
    Idiom19 i19(this);
    Idiom20 i20(this);
    struct is_valid
    {
        bool operator()(ICODE &z) { return not z.invalid;}
    };
    typedef boost::filter_iterator<is_valid,iICODE> ifICODE;
    while (pIcode != pEnd)
    {
        switch (pIcode->ic.ll.opcode)
        {
            case iDEC: case iINC:
            if (i18.match(pIcode))
                advance(pIcode,i18.action());
            else if (i19.match(pIcode))
                advance(pIcode,i19.action());
            else if (i20.match(pIcode))
                advance(pIcode,i20.action());
            else
                pIcode++;
            break;

        case iPUSH:
        {
            /* Idiom 1 */
            // todo add other push idioms.
                advance(pIcode,i01(pIcode));
                break;
        }

        case iMOV:
        {
            if (i02.match(pIcode)) /* Idiom 2 */
                advance(pIcode,i02.action());
            else if (i14.match(pIcode))  /* Idiom 14 */
                advance(pIcode,i14.action());
            else if (i13.match(pIcode))      /* Idiom 13 */
                advance(pIcode,i13.action());
            else
                pIcode++;
            break;
        }

            case iCALL:  case iCALLF:
                /* Check for library functions that return a long register.
                         * Propagate this result */
                if (pIcode->ic.ll.src.proc.proc != 0)
                    if ((pIcode->ic.ll.src.proc.proc->flg & PROC_ISLIB) &&
                        (pIcode->ic.ll.src.proc.proc->flg & PROC_IS_FUNC))
                    {
                        if ((pIcode->ic.ll.src.proc.proc->retVal.type==TYPE_LONG_SIGN)
                            || (pIcode->ic.ll.src.proc.proc->retVal.type == TYPE_LONG_UNSIGN))
                            localId.newLongReg(TYPE_LONG_SIGN, rDX, rAX, pIcode/*ip*/);
                    }

                /* Check for idioms */
                if (i03.match(pIcode))         /* idiom 3 */
                    advance(pIcode,i03.action());
                else if (i17.match(pIcode))  /* idiom 17 */
                    advance(pIcode,i17.action());
                else
                    pIcode++;
                break;

            case iRET:          /* Idiom 4 */
            case iRETF:
                advance(pIcode,i04(pIcode));
                break;

            case iADD:          /* Idiom 5 */
                advance(pIcode,i05(pIcode));
                break;

            case iSAR:          /* Idiom 8 */
                advance(pIcode,i08(pIcode));
                break;

            case iSHL:
                if (i15.match(pIcode))       /* idiom 15 */
                    advance(pIcode,i15.action());
                else if (i12.match(pIcode))        /* idiom 12 */
                    advance(pIcode,i12.action());
                else
                    pIcode++;
                break;

            case iSHR:          /* Idiom 9 */
                advance(pIcode,i09(pIcode));
                break;

            case iSUB:          /* Idiom 6 */
                advance(pIcode,i06(pIcode));
                break;

            case iOR:           /* Idiom 10 */
            advance(pIcode,i10(pIcode));
                break;

            case iNEG:          /* Idiom 11 */
                if (i11.match(pIcode))
                    advance(pIcode,i11.action());
                else if (i16.match(pIcode))
                    advance(pIcode,i16.action());
                else
                    pIcode++;
                break;

            case iNOP:
                (pIcode++)->invalidate();
                break;

            case iENTER:		/* ENTER is equivalent to init PUSH bp */
                if (pIcode == Icode.begin()) //ip == 0
                {
                    flg |= (PROC_HLL | PROC_IS_HLL);
                }
                pIcode++;
                break;

            case iXOR:          /* Idiom 7 */
                if (i21.match(pIcode))
                    advance(pIcode,i21.action());
                else if (i07.match(pIcode))
                    advance(pIcode,i07.action());
                else
                    ++pIcode;
                break;

            default:
                pIcode++;
        }
    }

    /* Check if number of parameter bytes match their calling convention */
    if ((flg & PROC_HLL) && (!args.sym.empty()))
    {
        args.m_minOff += (flg & PROC_FAR ? 4 : 2);
        delta = args.maxOff - args.m_minOff;
        if (cbParam != delta)
        {
            cbParam = delta;
            flg |= (CALL_MASK & CALL_UNKNOWN);
        }
    }
}


/* Sets up the TARGET flag for jump target addresses, and
 * binds jump target addresses to icode offsets.    */
void Function::bindIcodeOff()
{
    Int i;                 /* idx into icode array */
    iICODE pIcode;            /* ptr icode array      */
    dword *p;                 /* for case table       */

    if (Icode.empty())        /* No Icode */
        return;
    pIcode = Icode.begin();

    /* Flag all jump targets for BB construction and disassembly stage 2 */
    for(ICODE &c : Icode)
        {
        if ((c.ic.ll.flg & I) && JmpInst(c.ic.ll.opcode))
        {
            iICODE loc=Icode.labelSrch(c.ic.ll.src.op());
            if (loc!=Icode.end())
                loc->ic.ll.flg |= TARGET;
        }
    }
//    for (i = 0; i < Icode.size(); i++)
//        if ((pIcode[i].ic.ll.flg & I) && JmpInst(pIcode[i].ic.ll.opcode))
//        {
//            iICODE loc=Icode.labelSrch(pIcode[i].ic.ll.src.op());
//            if (loc!=Icode.end())
//                loc->ic.ll.flg |= TARGET;
//        }

    /* Finally bind jump targets to Icode offsets.  Jumps for which no label
     * is found (no code at dest. of jump) are simply left unlinked and
     * flagged as going nowhere.  */
    //for (pIcode = Icode.begin(); pIcode!= Icode.end(); pIcode++)
    for(ICODE &icode : Icode)
        {
        if (not JmpInst(icode.ic.ll.opcode))
            continue;
        if (icode.ic.ll.flg & I)
            {
                dword found;
            if (! Icode.labelSrch(icode.ic.ll.src.op(), found))
                icode.ic.ll.flg |= NO_LABEL;
                else
                icode.ic.ll.src.SetImmediateOp(found);

            }
        else if (icode.ic.ll.flg & SWITCH)
            {
            p = icode.ic.ll.caseTbl.entries;
            for (int j = 0; j < icode.ic.ll.caseTbl.numEntries; j++, p++)
                    Icode.labelSrch(*p, *p);
            }
        }
}

/** Performs idioms analysis, and propagates long operands, if any */
void Function::lowLevelAnalysis ()
{

    findIdioms(); // Idiom analysis - sets up some flags and creates some HIGH_LEVEL icodes
    propLong();   // Propagate HIGH_LEVEL idiom information for long operands
}
