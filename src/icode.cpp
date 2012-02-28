// Object oriented icode code for dcc
// (C) 1997 Mike Van Emmerik

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include "dcc.h"
#include "types.h"		// Common types like byte, etc
#include "ast.h"		// Some icode types depend on these
#include "icode.h"

#define ICODE_DELTA 25		// Amount to allocate for new chunk


CIcodeRec::CIcodeRec()
{
}

/* Copies the icode that is pointed to by pIcode to the icode array.
 * If there is need to allocate extra memory, it is done so, and
 * the alloc variable is adjusted.        */
ICODE * CIcodeRec::addIcode(ICODE *pIcode)
{
    push_back(*pIcode);
    back().loc_ip = size()-1;
    return &back();
}

void CIcodeRec::SetInBB(int start, int end, BB *pnewBB)
{
    for(ICODE &icode : *this)
        if((icode.loc_ip>=start) and (icode.loc_ip<=end))
            icode.inBB = pnewBB;
//    for (int i = start; i <= end; i++)
//        at(i).inBB = pnewBB;
}

/* labelSrchRepl - Searches the icodes for instruction with label = target, and
   replaces *pIndex with an icode index */
bool CIcodeRec::labelSrch(dword target, dword &pIndex)
{
    Int  i;
    iICODE location=labelSrch(target);
    if(end()==location)
	    return false;
    pIndex=location->loc_ip;
    return true;
}
CIcodeRec::iterator CIcodeRec::labelSrch(dword target)
{
    Int  i;
    return find_if(begin(),end(),[target](ICODE &l) -> bool {return l.ic.ll.label==target;});
}
ICODE * CIcodeRec::GetIcode(int ip)
{
    assert(ip>=0 && ip<size());
    iICODE res=begin();
    advance(res,ip);
    return &(*res);
}

extern char *indent(int level);
extern Int getNextLabel();
extern bundle cCode;
/* Checks the given icode to determine whether it has a label associated
 * to it.  If so, a goto is emitted to this label; otherwise, a new label
 * is created and a goto is also emitted.
 * Note: this procedure is to be used when the label is to be backpatched
 *       onto code in cCode.code */
void ICODE::emitGotoLabel (Int indLevel)
{
    if (! (ic.ll.flg & HLL_LABEL)) /* node hasn't got a lab */
    {
        /* Generate new label */
        ic.ll.hllLabNum = getNextLabel();
        ic.ll.flg |= HLL_LABEL;

        /* Node has been traversed already, so backpatch this label into
                 * the code */
        addLabelBundle (cCode.code, codeIdx, ic.ll.hllLabNum);
    }
    cCode.appendCode( "%sgoto L%ld;\n", indent(indLevel), ic.ll.hllLabNum);
    stats.numHLIcode++;
}

