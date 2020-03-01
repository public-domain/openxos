//*******************************************
// Functions to evaluate expressions for XMAC
//*******************************************
// Written by John Goltz
//*******************************************

//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <CTYPE.H>
#include <XOS.H>
#include "XMAC.H"
#include "XMACEXT.H"

// Allocate local data

char *sympnt;			// Pointer to symbuf
char  regntbl[] =		// Register name table
{   'A','L',' ',' ', RG_AL,  1,
    'A','H',' ',' ', RG_AH,  1,
    'B','L',' ',' ', RG_BL,  1,
    'B','H',' ',' ', RG_BH,  1,
    'C','L',' ',' ', RG_CL,  1,
    'C','H',' ',' ', RG_CH,  1,
    'D','L',' ',' ', RG_DL,  1,
    'D','H',' ',' ', RG_DH,  1,
    'A','X',' ',' ', RG_AX,  2,
    'B','X',' ',' ', RG_BX,  2,
    'C','X',' ',' ', RG_CX,  2,
    'D','X',' ',' ', RG_DX,  2,
    'S','I',' ',' ', RG_SI,  2,
    'D','I',' ',' ', RG_DI,  2,
    'B','P',' ',' ', RG_BP,  2,
    'S','P',' ',' ', RG_SP,  2,
    'E','A','X',' ', RG_EAX, 4,
    'E','B','X',' ', RG_EBX, 4,
    'E','C','X',' ', RG_ECX, 4,
    'E','D','X',' ', RG_EDX, 4,
    'E','S','I',' ', RG_ESI, 4,
    'E','D','I',' ', RG_EDI, 4,
    'E','B','P',' ', RG_EBP, 4,
    'E','S','P',' ', RG_ESP, 4,
    'C','S',' ',' ', RG_CS,  2,
    'D','S',' ',' ', RG_DS,  2,
    'S','S',' ',' ', RG_SS,  2,
    'E','S',' ',' ', RG_ES,  2,
    'F','S',' ',' ', RG_FS,  2,
    'G','S',' ',' ', RG_GS,  2,
    'P','C',' ',' ', RG_PC,  4,
    'F','R',' ',' ', RG_FR,  2,
    'L','D','T','R', RG_LDTR,2,
    'M','S','W',' ', RG_MSW, 2,
    'T','R',' ',' ', RG_TR,  2,
    'G','D','T','R', RG_GDTR,6,
    'I','D','T','R', RG_IDTR,6,
    'C','R','0',' ', RG_CR0, 4,
    'C','R','2',' ', RG_CR2, 4,
    'C','R','3',' ', RG_CR3, 4,
    'D','R','0',' ', RG_DR0, 4,
    'D','R','1',' ', RG_DR1, 4,
    'D','R','2',' ', RG_DR2, 4,
    'D','R','3',' ', RG_DR3, 4,
    'D','R','6',' ', RG_DR6, 4,
    'D','R','7',' ', RG_DR7, 4,
    'T','R','6',' ', RG_TR6, 4,
    'T','R','7',' ', RG_TR7, 4,
    'S','T',' ',' ', RG_ST0, 4,
    'S','T','0',' ', RG_ST0, 4,
    'S','T','1',' ', RG_ST1, 4,
    'S','T','2',' ', RG_ST2, 4,
    'S','T','3',' ', RG_ST3, 4,
    'S','T','4',' ', RG_ST4, 4,
    'S','T','5',' ', RG_ST5, 4,
    'S','T','6',' ', RG_ST6, 4,
    'S','T','7',' ', RG_ST7, 4
};
#define REGNUM ((sizeof(regntbl))/5)

//***************************************************
// Function: getabs - Evaluate general expression and
//	return absolute value only
// Returned: Nothing
//***************************************************

void getabs(void)

{
    exprsn();				// Evalulate expression
    if (val.vl_kind != VK_ABS)		// Simple absolute value?
    {
        errorx();			// No - complain
	val.vl_psect = 0;		// And return 0 as value
        val.vl_kind = VK_ABS;
	val.vl_val.vl_vl = 0;
	val.vl_type = VL_VAL;
    }
}

//***********************************************
// Function: exprsn - Evaluate general expression
// Returned: Nothing
//***********************************************

void exprsn(void)

{
    stopper = nxtnbc();
    level1(&val);
    if (stopper == '.')			// Stopped by period?
    {
        stopper = toupper(nxtchar());	// Yes - get next character
        if (val.vl_type == VL_VAL)	// If normal value
        {
            switch(stopper)
            {
            case 'B':			// Byte value
                val.vl_type = VL_BYTE;
                val.vl_size = 1;
                break;
            case 'W':			// Word value
                val.vl_type = VL_WORD;
                val.vl_size = 2;
                break;
            case 'L':			// Long value
                val.vl_type = VL_LONG;
                val.vl_size = 4;
                break;
            case 'S':			// Relative byte value
                val.vl_type = VL_BREL;
                val.vl_size = 1;
                break;
            case 'R':			// Relative word value
                val.vl_type = VL_WREL;
                val.vl_size = 2;
                break;
            case 'Q':			// Relative long value
                val.vl_type = VL_LREL;
                val.vl_size = 4;
                break;
            case 'V':			// Relative variable length
                val.vl_type = VL_VREL;	//   value
                break;
            default:			// Invalid modifier
                errorx();
                return;
            }
        }
        else				// Must be register value
        {
            errorx();
            return;
        }
        stopper = nxtnbc();		// Get stopper
    }
}

//*********************************************
// Function: level1 - Process level 1 operators
// Returned: Nothing
//*********************************************

void level1(
    struct vl *value)

{
    struct vl rval;

    level2(value);			// Evaluate left operand
    if (stopper == ':' && value->vl_kind != VK_REG)
    {
		stopper = nxtnbc();
		level2(&rval);			// Evaluate right operand
		if (value->vl_kind != VK_ABS ||
				(rval.vl_kind != VK_ABS && rval.vl_kind != VK_ABA))
			errorx();			// Both operands must be absolute
		value->vl_psect = value->vl_val.vl_vl;
		value->vl_val.vl_vl = rval.vl_val.vl_vl;
		value->vl_kind = VK_ABA;
		value->vl_undef |= rval.vl_undef;
	}
}

//*****************************
// Function: level2 - Process |
// Returned: Nothing
//*****************************

void level2(
    struct vl *value)

{
    struct vl rval;

    level3(value);			// Evaluate left operand
    while (stopper == '|' && value->vl_kind != VK_REG)
    {
	stopper = nxtnbc();
	level1(&rval);			// Evaluate right operand
	if (value->vl_kind != VK_ABS || rval.vl_kind != VK_ABS)
            errorx();			// Both operands must be absolute
	value->vl_val.vl_vl |= rval.vl_val.vl_vl; // Do inclusive or
        value->vl_undef |= rval.vl_undef;
    }
}

//*****************************
// Function: level3 - Process ^
// Returned: Nothing
//*****************************

void level3(
    struct vl *value)

{
    struct vl rval;

    level4(value);			// Evaluate left operand
    while (stopper == '^' && value->vl_kind != VK_REG)
    {
	stopper = nxtnbc();
	level4(&rval);			// Evaluate right operand
	if (value->vl_kind != VK_ABS || rval.vl_kind != VK_ABS)
            errorx();			// Both operands must be absolute
	value->vl_val.vl_vl ^= rval.vl_val.vl_vl; // Do exclusive or
        value->vl_undef |= rval.vl_undef;
    }
}

//*****************************
// Function: level4 - Process &
// Returned: Nothing
//*****************************

void level4(
    struct vl *value)

{
    struct vl rval;

    level5(value);			// Evaluate left operand
    while (stopper == '&' && value->vl_kind != VK_REG)
    {
	stopper = nxtnbc();
	level5(&rval);			// Evaluate right operand
	if (value->vl_kind != VK_ABS || rval.vl_kind != VK_ABS)
            errorx();			// Both operands must be absolute
	value->vl_val.vl_vl &= rval.vl_val.vl_vl; // Do and
        value->vl_undef |= rval.vl_undef;
    }
}

//***********************************
// Function: level5 - Process > and <
// Returned: Nothing
//***********************************

void level5(
    struct vl *value)

{
    struct vl rval;
    char   chr;

    level6(value);			// Evaluate left operand
    while (((chr=stopper) == '<' || chr == '>') && value->vl_kind != VK_REG)
    {
	stopper = nxtnbc();
	level6(&rval);			// Evaluate right operand
	if (value->vl_kind != VK_ABS || rval.vl_kind != VK_ABS)
            errorx();			// Both operands must be absolute
	if (chr == '<')
	    value->vl_val.vl_vl <<= rval.vl_val.vl_vl; // If left shift
	else
	    value->vl_val.vl_vl = ((unsigned long)(value->vl_val.vl_vl)) >>
                      rval.vl_val.vl_vl; // If right shift
        value->vl_undef |= rval.vl_undef;
    }
}

//***********************************
// Function: level6 - Process + and -
// Returned: Nothing
//***********************************

void level6(
    struct vl *value)

{
    struct vl rval;
    char   chr;

    level7(value);			// Evaluate left operand
    while (((chr=stopper) == '+' || chr == '-') && value->vl_kind != VK_REG)
    {
	stopper = nxtnbc();
	level7(&rval);			// Evaluate right operand
        if (value->vl_kind >= VK_REG || rval.vl_kind >= VK_REG)
        {				// Error if either operand is an
            errorx();			//   address space selector or a
            continue;			//   register
        }
	if (chr == '+')			// If add
	{
	    if (value->vl_kind == VK_ABS) // If left operand is absolute
	    {
		value->vl_psect = rval.vl_psect;
                value->vl_kind = rval.vl_kind;
		value->vl_pnt = rval.vl_pnt;
	    }
	    else			    // If left operand is relative
            {				    //   or absolute address or
	        if (rval.vl_kind != VK_ABS) //   external, right operand
		    errorx();		    //   must be absolute
	    }
	    value->vl_val.vl_vl += rval.vl_val.vl_vl;
	}
	else				// If subtract
	{
	    if (value->vl_kind == VK_ABS || value->vl_kind == VK_EXT)
	    {				// If left operand absolute or
					//   external
		if (rval.vl_kind != VK_ABS) // Right operand must be absolute
                    errorx();
	    }
	    else			// If left operand is address
	    {
		if (value->vl_kind == rval.vl_kind &&
                    value->vl_psect == rval.vl_psect)
                {			     // Right operand can be address
                    value->vl_psect = 0;     //   for same psect, difference
                    value->vl_kind = VK_ABS; //   is absolute
                }
		else if (rval.vl_kind == VK_ABS) // Right operand can be
		    ;			         //   absolute, difference is
		else			         //   an address
                    errorx();		 // Else bad expression
	    }
	    value->vl_val.vl_vl -= rval.vl_val.vl_vl;
	}
        value->vl_undef |= rval.vl_undef;
    }
}

//***********************************
// Function: level7 - Process * and /
// Returned: Nothing
//***********************************

void level7(
    struct vl *value)

{
    struct vl rval;
    char   chr;

    level8(value);			// Evaluate left operand
    while (((chr=stopper) == '*' || chr == '/') && value->vl_kind != VK_REG)
    {
	stopper = nxtnbc();
	level8(&rval);			// Evaluate right operand
	if (value->vl_kind != VK_ABS || rval.vl_kind != VK_ABS)
            errorx();			// Both operands must be absolute
	if (chr == '*')
	    value->vl_val.vl_vl *= rval.vl_val.vl_vl; // If multiply
	else
	{
	    if (rval.vl_val.vl_vl == 0)	// If divide
                errorx();
	    else
		value->vl_val.vl_vl /= rval.vl_val.vl_vl;
	}
        value->vl_undef |= rval.vl_undef;
    }
}

//**********************************
// Function: Process unary operators
// Returned: Nothing
//**********************************

void level8(
    struct vl *value)

{
    char chr;

    if ((chr=stopper) == '~' || chr == '-' || chr == '!')
    {
	stopper = nxtnbc();
	level8(value);			// Evaluate operand
        if (chr == '!')			// Want selector part?
        {				// Yes
            if (value->vl_kind == VK_ABA) // Absolute address?
            {				// Yes - just use value of selector
					//   part
                value->vl_val.vl_vl = (unsigned long)(value->vl_psect);
                value->vl_psect = 0;
                value->vl_kind = VK_ABS;
            }
            else if (value->vl_kind == VK_OFS) // Address?
            {
                value->vl_val.vl_vl = 0; // Yes
                value->vl_kind = VK_SEL;
                value->vl_psect = getsegnumps(value->vl_psect);
            }
            else if (value->vl_kind == VK_EXT) // External value?
            {
                value->vl_val.vl_vl = 0; // Yes
                value->vl_kind = VK_SLX;
            }
            else if (value->vl_kind == VK_MSC) // Msect value?
            {
                value->vl_val.vl_vl = 0; // Yes
                value->vl_kind = VK_SEL;
                value->vl_psect = getsegnumms(value->vl_psect);
            }
            else if (value->vl_kind == VK_SEG) // Segment value?
            {
                value->vl_val.vl_vl = 0; // Yes
                value->vl_kind = VK_SEL;
            }
            else
                errorna();
        }
        else				// Must be minus or 1's complement
        {
            if (value->vl_kind != VK_ABS)
                errorx();		// Error if not absolute
            if (chr == '~')
                value->vl_val.vl_vl = ~value->vl_val.vl_vl;
            else
                value->vl_val.vl_vl = -value->vl_val.vl_vl;
        }
    }
    else
	level9(value);
}

//**********************************
// Function: level9 - Process braces
// Returned: Nothing
//**********************************

void level9(
    struct vl *value)

{
    if (stopper == '{')			// Start of sub-expression?
    {
	stopper = nxtnbc();		// Yes
	level1(value);			// Evaluate sub-expression
	if (stopper == '}')		// Must end with right brace
	    stopper = nxtnbc();
	else
	    queerr();
    }
    else
    {
	primary(value);			// Get value of atom
	stopper = nxtnb0(stopper);
    }
}

//************************************
// Function: primary - Fetch next atom
// Returned: Nothing
//************************************

void primary(
    struct vl *value)

{
    char  *temp;
    char  *tpnt;
    struct sy *sym;
    int    extflag;
    int    cnt;
    char   chr;
    char   char0, char1, char2, char3;

    value->vl_type = VL_VAL;
    value->vl_kind = VK_ABS;
    value->vl_psect = 0;
    value->vl_size = 4;
    value->vl_undef = FALSE;
    chr = stopper;			// Get first character
    if (chr == '\'')			// Single quote?
    {
        cnt = 0;
        value->vl_val.vl_vl = 0;
        while ((chr=nxtchar()) != '\0' &&
                (chr != '\'' || (chr=nxtchar()) == '\''))
        {
            value->vl_val.vl_vl |= ((long)chr << cnt);
            cnt += 8;
        }
	stopper = nxtnb0(chr);
	return;
    }
    if (chr == '"')			// Double quote?
    {
	value->vl_val.vl_vc.lshc = nxtchar() & 0x7F; // Yes
	value->vl_val.vl_vc.lslc = nxtchar() & 0x7F;
	stopper = nxtnbc();
	return;
    }
    if (isdigit(chr))			// Is it a digit?
    {
	chr = getadg(chr);		// Collect digits
	chr = toupper(chr);
	switch(chr)
	{
         case '$':		// Local symbol
	    getals();			// Yes - collect local symbol
            if ((sym = looksym()) != NULL) // Find symbol table entry for symbol
            {
                if (sym->sy_flag & SYF_MULT)
                    errsub(errdmsg, ERR_D);
                value->vl_val.vl_vl = sym->sy_val.val;
                value->vl_psect = sym->sy_psect;
                value->vl_kind = (sym->sy_flag & SYF_ABS)? VK_ABA: VK_OFS;
            }
            else
            {
                errsub(errumsg, ERR_U);	// Undefined if not in symbol table
                value->vl_undef = TRUE;	// Remember that its undefined
                value->vl_val.vl_vl = 0;
            }
            stopper = nxtnbc();
	    if (stopper == '#')		// Was symbol stopped by #?
                errsub(erramsg, ERR_A);	// Yes - # following local symbol
            return;			//   is an error!

         case '.':		// Might be floating point number?
            chr = nxtchar();		// Get following character
            if (isdigit(chr))		// Floating point if digit
            {
                do		// CODE GOES HERE TO HANDLE FLOATING POINT!
                {

                } while (isdigit(chr=nxtchar()));
                value->vl_val.vl_vl = 0x55555L;
                errsub(errxmsg, ERR_X);
                break;
            }
            hldchar = chr;		// If not a floating point number
            value->vl_val.vl_vl = getanum((int)iradix); // Get value
            stopper = '.';
            return;

         case 'O':		// Octal number
         case 'Q':
            value->vl_val.vl_vl = getanum(8); // Yes - get value
	    break;

         case 'T':		// Decimal number
            value->vl_val.vl_vl = getanum(10); // Yes - get value
	    break;

         case 'H':		// Hex number
            value->vl_val.vl_vl = getanum(16); // Yes - get value
	    break;

         case 'W':		// Binary number
            value->vl_val.vl_vl = getanum(2); // Yes - get value
	    break;

         default:			// If no radix specified
            value->vl_val.vl_vl = getanum((int)iradix); // Get value
	    stopper = chr;
            return;
	}
        stopper = nxtnbc();
	return;
    }
    if (!getsym(chr))			// Try to collect symbol
    {
	value->vl_val.vl_vl = 0;	// Not symbol - return 0
	return;
    }
    if (symsize == 1 && symbuf.nsym[0] == '$') // Is this the current addr?
    {
        chkpsect();			// Yes - make sure have psect
	value->vl_val.vl_vl = curpsd->pd_ofs;
	value->vl_psect = curpsd->pd_psn;
        value->vl_kind = VK_OFS;
	return;
    }
    temp = NULL;
    if (prnflg)				// Using special register names?
    {
	if ((symsize>=3 && symsize<=5) && symbuf.nsym[0]=='.') // Yes
	    temp = &symbuf.nsym[1];
    }
    else
    {
	if (symsize>=2 && symsize<=4)	// No
	    temp = &symbuf.nsym[0];
    }
    if (temp)				// Might this be a register name?
    {
        char0 = toupper(temp[0]);
        char1 = toupper(temp[1]);
        char2 = toupper(temp[2]);
        char3 = toupper(temp[3]);
        if (char0 == '%')		// Want auto-size?
        {				// Yes
            if (cursgd && (segatrb & SA_32BIT)) // 32 bit?
                char0 = 'E';		// Yes
            else
            {
                char0 = char1;		// No
                char1 = char2;
                char2 = char3;
                char3 = ' ';
            }
        }
	for (tpnt=regntbl, cnt=REGNUM; --cnt >= 0;)
	{
	    if (tpnt[0]==char0 && tpnt[1]==char1 && tpnt[2]==char2
                    && tpnt[3] == char3)
	    {				// Found it - get value
		value->vl_val.vl_vl = (long)(tpnt[4]);
                value->vl_psect = 0;
                value->vl_size = tpnt[5];
		value->vl_type = VL_VAL;
                value->vl_kind = VK_REG;
		return;
	    }
	    tpnt += 6;			// Not this one - bump table pointer
	}
    }
    if (stopper == '#')			// Was symbol stopped by #?
    {
	stopper = nxtnbc();		// Yes - stopper is next character
        if (stopper == '#')
        {
            stopper = nxtnbc();
            extflag = 2;
        }
        else
            extflag = 1;
    }
    else
	extflag = 0;
    sym = findsym(FALSE);		// Search symbol table
    if (!prmflg)			// From parameter file?
        sym->sy_flag |= SYF_USED;	// No - indicate symbol has been used
    if (sym->sy_flag & SYF_MULT)	// Is it multiply defined?
        errsub(errdmsg, ERR_D);		// Yes - flag as D error
    if (sym->sy_flag & SYF_UNDEF)	// Undefined symbol?
    {
	if (extflag)			// Yes - followed by # or ##?
	{
	    sym->sy_psect = 0;		// Yes - make the symbol external
            sym->sy_flag |= (extflag == 1)? SYF_EXTERN: SYF_IMPORT;
	    sym->sy_flag &= ~SYF_UNDEF;
	    value->vl_pnt = sym;
	    value->vl_val.vl_vl = 0;
	    return;
	}
	else
	{
	    errsub(errumsg, ERR_U);	// Not followed by # - its undefined
            value->vl_undef = TRUE;	// Remember that its undefined
	    value->vl_val.vl_vl = 0;	// Return 0 as value
	    return;
	}
    }
    else				// If defined
    {
        if (extflag)
        {
            if (!(sym->sy_flag & (SYF_EXTERN|SYF_IMPORT)))
					// Error if flagged as external but
                errsub(errmmsg, ERR_M);	//   symbol is not external or
            else
            {
                if (extflag == 2)		  // Make sure imported if
                {				  //   flagged as imported
                    sym->sy_flag |= SYF_IMPORT;
                    sym->sy_flag &= ~SYF_EXTERN;    
                }
            }
        }
    }					//   imported!
    if (sym->sy_flag & (SYF_EXTERN|SYF_IMPORT)) // External or imported?
    {
        value->vl_kind = VK_EXT;	// Yes
	value->vl_pnt = sym;
	value->vl_val.vl_vl = 0;
        value->vl_psect = 0;
        value->vl_kind = VK_EXT;
    }
    else if (sym->sy_type == SYT_SYM)	// Really a symbol?
    {
        value->vl_val.vl_vl = sym->sy_val.val;
        value->vl_psect = sym->sy_psect;
        value->vl_kind = (sym->sy_flag & (SYF_EXTERN|SYF_IMPORT))? VK_EXT:
                !(sym->sy_flag & SYF_ADDR)? VK_ABS:
                (sym->sy_flag & SYF_ABS)? VK_ABA: VK_OFS;
        if (sym->sy_flag & SYF_REG)	// Is this value a register?
        {
            value->vl_kind = VK_REG;	// Yes
            if (extflag)		// Can't have # after register
                errsub(erramsg, ERR_A);
        }
    }
    else				// If not symbol (segment, msect, or
    {					//   psect
        value->vl_val.vl_vl = 0;
        value->vl_psect = sym->sy_psect;
        value->vl_kind = (sym->sy_type == SYT_SEG)? VK_SEG:
                (sym->sy_type == SYT_MSC)? VK_MSC: VK_OFS;
    }
}

//**************************************************
// Function: getanum -  Return value of number after
//	digits have been collected in digbuf
// Returned: Value of number
//**************************************************

long getanum(
    int  radix)

{
    long  value;
    long  dig;
    char *pnt;

    value = 0;
    pnt = digbuf;
    while (--digcnt >= 0)
    {
	if ((dig = *pnt++) >= radix)
	{
            errorx();
	    dig = radix - 1;
	}
	value = value * radix + dig;
    }
    return (value);
}

//***********************************************************
// Function: getsyq - Collect symbol in symbuf (also collects
//	local symbols)
// Returned: TRUE if symbol collected, FALSE if not a symbol
//***********************************************************

int getsyq(void)

{
    char chr;

    chr = nxtnbc();			// Get first character
    if (isdigit(chr))			// Is it a digit?
    {
        chr = getadg(chr);		// Yes - collect digits
        if (chr != '$')			// Stopped by dollar sign?
        {
            errorq();			// No - indicate Q error
            stopper = chr;
            return (FALSE);
        }
        else
        {
            getals();			// Yes - collect local symbol
            stopper = nxtnbc();
            return (TRUE);
        }
    }
    else
        return (getsym(chr));		// If not local symbol
}

//******************************************************************
// Function: getsym - Collect symbol in symbuf (use this function if
//	already have first character of symbol
// Returned: TRUE if symbol collected, FALSE if not a symbol
//******************************************************************

int getsym(
    char chr)

{
    int   cnt;
    char *pnt;

    chr = nxtnb0(chr);
    if (chr != '.' && !chksys(chr))	// Can this start a symbol?
    {
	stopper = chr;			// No - fail
	return (FALSE);
    }
    for (pnt = symbuf.nsym, cnt = SYMMAXSZ; --cnt >= 0;)
	*pnt++ = ' ';			// Clear symbuf

    symsize = 0;			// Clear symbol size
    sympnt = symbuf.nsym;
    do
    {   if (symsize < SYMMAXSZ)		// Is symbol full now?
	{
	    ++symsize;			// No
	    *sympnt++ = chr;		// Store character in symbol
	}
	chr = nxtchar();		// Get next character
    } while (chksym(chr));
    stopper = chr;			// Store stopper
    return (TRUE);			// Indicate have symbol
}

//***********************************************
// Function: persym - Collect remainder of symbol
//	which may contain a period
// Returned: Nothing
//***********************************************

void persym(void)

{
    char chr;

    if ((chr=stopper) == '.')
    {
	while (chr == '.' || chksym(chr))
	{
	    if (symsize < SYMMAXSZ)	// Is symbol full now?
	    {
		++symsize;		// No
		*sympnt++ = chr;	// Store character in symbol
	    }
	    chr = nxtchar();		// Get next character
	}
	stopper = nxtnb0(chr);
    }
}

//*************************************************************
// Function: chksum - Determine if a character can be part of a
//	symbol
// Returned: TRUE if character can be part of a symbol, FALSE
//	if not
//*************************************************************

int chksym(
    char chr)

{
    return(isdigit(chr)) ? TRUE : chksys(chr);
}

//*************************************************************
// Function: chksym - Determine if a character can be the first
//	character in a symbol
// Returned: TRUE if character can start a symbol, FALSE if not
//*************************************************************

int chksys(
    char chr)

{
    return (isalpha(chr) || chr == '@' || chr == '%' || chr == '_'
	    || chr == '$' || chr == '?');
}

//**********************************
// Function: getadg - Collect digits
// Returned: Stopper character
//**********************************

char getadg(
    char chr)

{
    char *pnt;
    int   cnt;

    pnt = digbuf;
    cnt = 0;
    do
    {
	if (cnt < DIGMAXSZ)
	{
	    if (chr >= 'A')		// Adjust value between A and F
		chr += 9;
	    *pnt++ = chr & 0xF;		// Store value in digit buffer
	    ++cnt;
	}
	else
            errorx();
	chr = nxtchar();		// Get next character
    } while (isxdigit(chr));
    digcnt = cnt;
    return (chr);
}

//****************************************
// Function: getals - Collect local symbol
// Returned: Nothing
//****************************************

void getals(void)

{
    symbuf.lsym.ls_flag = 0;		// Indicate local symbol
    symbuf.lsym.ls_blk = lsbnum;	// Store local symbol block number
    symbuf.lsym.ls_val = getanum(16);	// Store value of digits
    symsize = sizeof(struct locsym);
}
