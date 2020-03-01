//******************************************
// Functions to handle opcode input for XMAC
//******************************************
// Written by John Goltz
//******************************************

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
#include <XOS.H>
#include "XMAC.H"
#include "XMACEXT.H"

#define LX_BB  0
#define LX_W   1
#define LX_L   2

extern char regindx[];
extern char regbits[];
extern char regseg[];
int    adrerr();

// Allocate local data

struct op opd1;			// Data for first operand
struct op opd2;			// Data for second operand
struct op opd3;			// Data for third operand

// Dispatch table for type of first operand for MOV instruction

void (*movdsp1[])(struct optbl *) =
{   movreg1,				// OP_REG
    (void (*)(struct optbl *))adrerr,	// OP_IMM
    movmem1,				// OP_MEM
    movmem1,				// OP_CM1
    movmem1				// OP_CM2
};

// Function to handle MOV instruction

void imovins(opt)
struct optbl *opt;

{
    if (getopr1(&opd1, opt, RB_DAT|RB_SEG|RB_CNTL) // Get operands
	    && getopr(&opd2, opt, RB_DAT|RB_SEG|RB_CNTL))
        (*movdsp1[opd1.op_type])(opt);	// Dispatch on type of first operand
}

// Dispatch table for second operand of move instruction when first operand
//   is a register

void (*movregdsp[])(struct optbl *) =
{   movregreg,		// OP_REG
    movregimm,		// OP_IMM
    movregmem,		// OP_MEM
    movregmem,		// OP_CM1
    movregmem		// OP_CM2
};

// Function to handle MOV instruction when first (destination) operand is a
//   register

void movreg1(opt)
struct optbl *opt;

{
    (*movregdsp[opd2.op_type])(opt);	// Dispatch on type of second operand
}

// Function to handle MOV instruction when first (destination) operand is a
//   register and second (source) operand is also a register

void movregreg(opt)
struct optbl *opt;

{
    if (opd1.op_val.vl_val.vl_vi<=RG_ESP && opd2.op_val.vl_val.vl_vi<=RG_ESP)
    {					// If have two data registers
        opcdout(opt->ot_val1 | 0x02, &opd2, opt); // Output opcode byte
        absbyte(opd2.op_modrm + (regindx[(int)(opd1.op_val.vl_val.vl_vl)]<<3));
    }					// Followed by mod-r/m byte
    else if (opd1.op_val.vl_val.vl_vi <= RG_ESP) // Is destination a data
    {						 //   register?
        if (opd2.op_val.vl_val.vl_vi <= RG_GS)	// Yes - is source a segment
        {					//   register?
            opcdout(0x8C, &opd1, opt);	// Yes - output opcode byte
            oprdout(regindx[(int)(opd2.op_val.vl_val.vl_vl)], &opd1);
        }
        else				// Source must be a control register
            cntlreg(0x00, opd2.op_val.vl_val.vl_vi, opd1.op_val.vl_val.vl_vi);
    }
    else if (opd2.op_val.vl_val.vl_vi <= RG_ESP) // Is source a data
    {						 //   register?
        if (opd1.op_val.vl_val.vl_vi <= RG_GS)	// Yes - is destination a
        {					//   segment register?
            opcdout(0x8E, &opd1, opt);	// Yes - output opcode byte
            oprdout(regindx[(int)(opd2.op_val.vl_val.vl_vl)], &opd1);
        }
        else				// Destination must be a control
					//   register
            cntlreg(0x02, opd1.op_val.vl_val.vl_vi, opd2.op_val.vl_val.vl_vi);
    }
    else
        adrerr();
}

// Function to handle transfers to or from control register for MOV
//   instruction

void cntlreg(dbit, creg, dreg)
int dbit;
int creg;
int dreg;

{
    if (creg > RG_DR7 || ptype < P_80386) // Processor must be 386 or 486
    {
        adrerr();
        return;
    }
    absbyte(0x0F);
    if (creg <= RG_CR3)
        absbyte(0x20 | dbit);
    else if (creg <= RG_DR7)
        absbyte(0x21 | dbit);
    else
        absbyte(0x24 | dbit);
    absbyte(0xC0 | regindx[creg] | regindx[dreg]);
}

// Function to handle MOV instruction when first (destination) operand is a
//   register and second (source) operand is immediate

void movregimm(opt)
struct optbl *opt;

{
    if (opd1.op_val.vl_val.vl_vi <= RG_ESP) // Data register?
    {
        opcdout(opt->ot_val3 | regindx[(int)(opd1.op_val.vl_val.vl_vl)],
                &opd1, opt);
        valueout(&opd2, opt);
    }
    else
        adrerr();			// No - error
}

// Function to handle MOV instruction when first (destination) operand is a
//   register and second (source) operand is a memory address

void movregmem(opt)
struct optbl *opt;

{
    movrmmr(0x02, &opd1, &opd2, opt);
}

// Dispatch table for second operand of MOV instruction when first operand
//   is a memory address

void (*movmemdsp[])(struct optbl *) =
{   movmemreg,				// OP_REG
    movmemimm,				// OP_IMM
    (void (*)(struct optbl *))adrerr,	// OP_MEM
    (void (*)(struct optbl *))adrerr,	// OP_CM1
    (void (*)(struct optbl *))adrerr	// OP_CM2
};

// Function to handle MOV instruction when first (destination) operand is a
//   memory reference

void movmem1(opt)
struct optbl *opt;

{
    (*movmemdsp[opd2.op_type])(opt);	// Dispatch on type of second operand
}

// Function to handle MOV instruction when first (destination) operand is a
//   memory address and second (source) operand is a register

void movmemreg(opt)
struct optbl *opt;

{
    movrmmr(0x00, &opd2, &opd1, opt);
}

// Function to handle MOV instruction when first (destination) operand is a
//   memory address and second (source) operand is immediate

void movmemimm(opt)
struct optbl *opt;

{
    opcdout(opt->ot_val2, &opd1, opt);
    oprdout(0x00, &opd1);
    valueout(&opd2, opt);
}

// Function to handle MOV instruction for reg-mem or mem-reg transfer

void movrmmr(dbit, ropd, mopd, opt)
int    dbit;
struct op    *ropd;
struct op    *mopd;
struct optbl *opt;

{
    if (ropd->op_val.vl_val.vl_vi <= RG_ESP) // Data register?
    {
        if (mopd->op_type == OP_MEM && (ropd->op_val.vl_val.vl_vi == RG_AL ||
                ropd->op_val.vl_val.vl_vi == RG_AX ||
                ropd->op_val.vl_val.vl_vi == RG_EAX))
        {
            opcdout(opt->ot_val4 ^ dbit, mopd, opt);
            if (mopd->op_nab != 4)
                putword(U_VALUE, &mopd->op_val);
            else
                putlong(U_VALUE, &mopd->op_val);
        }
        else
        {
            opcdout(opt->ot_val1 | dbit, mopd, opt); // Output opcode byte
            oprdout(regindx[(int)(ropd->op_val.vl_val.vl_vl)]<<3, mopd);
        }				// Followed by operand byte(s)
    }
    else if (ropd->op_val.vl_val.vl_vi <= RG_GS) // Segment register?
    {
        if (ropd->op_val.vl_val.vl_vi > RG_ES && ptype < P_80386)
            adrerr();
        else
        {
            opcdout(0x8C | dbit, mopd, opt);
            oprdout(regindx[(int)(ropd->op_val.vl_val.vl_vl)], mopd);
        }
    }
    else
        adrerr();
}

// Function to handle single operand floating point instructions which expect
//   a floating point register as operand  - operand must be a single floating
//   point register (ST or ST0 - ST7)

void ioneoprf(opt)
struct optbl *opt;

{
    stopper = nxtnb0(stopper);		// Make sure have non-blank character
    if (stopper == '\0' || stopper == '}' || stopper == ';') // Have operand?
    {
        absbyte(opt->ot_val1);		// No - operand - default is ST0
        absbyte(opt->ot_val2);
        return;
    }
    if (getopr(&opd1, opt, RB_FLOAT) && opd1.op_type == OP_REG)
    {
        absbyte(opt->ot_val1);
        absbyte(opt->ot_val2 + regindx[(int)(opd1.op_val.vl_val.vl_vl)]);
        return;
    }
    adrerr();
}

// Function to handle general single operand floating point instructions with
//   an initial wait

void ioneoprw(opt)
struct optbl *opt;

{
    absbyte(0x9B);			// Output WAIT opcode
    ioneopr(opt);			// Then just like normal instruction
}

// Function to handle FNSTSW and FSTSW instructions

void ifstsw(
    struct optbl *opt)

{
    if (getopr(&opd1, opt, RB_DAT))
    {
        if (opd1.op_type == OP_REG && opd1.op_val.vl_val.vl_vi == RG_AX)
        {
            if (opt->ot_val1)
                absbyte(0x9B);
            absbyte(0xDF);
            absbyte(0xE0);
        }
        else if (opd1.op_type >= OP_MEM)
        {
            if (opt->ot_val1)
                absbyte(0x9b);
            opcdout(0xDD, &opd1, opt);
            oprdout(0x38, &opd1);
        }
        else
            adrerr();
    }

}

// Function to handle general single operand instructions

void ioneopr(opt)
struct optbl *opt;

{
    if (getopr(&opd1, opt, RB_DAT))
    {
        if ((opd1.op_type == OP_REG && (opt->ot_flag & OF_RS)) ||
                (opd1.op_type >= OP_MEM && (opt->ot_flag & OF_MS)))
        {
            opcdout(opt->ot_val1, &opd1, opt);
            oprdout(opt->ot_val4, &opd1);
        }
        else
            adrerr();
    }
}

// Dispatch table for type of first operand for general two operand
//   instructions

void (*op2dsp1[])(struct optbl *) =
{   op2reg1,				// OP_REG
    (void (*)(struct optbl *))adrerr,	// OP_IMM
    op2mem1,				// OP_MEM
    op2mem1,				// OP_CM1
    op2mem1				// OP_CM2
};

// Function to handle two operand floating point instructions - these
//   instructions take zero, one, or two floating point registers as operands.
//   If two operands are given, one of them must be ST0.  If one operand is
//   given, it can be any floating point register (this is equivilent to
//   ST0, STn).  If no operand is given, this is equivilent to ST0, ST1. Also
//   if no operand is given, the no-push form of the opcode means the push
//   form most of the time! (FADD is the same as FADDP ST0, ST1, not FADD ST0,
//   ST1, but FCOM is the same as FCOM ST0, ST1!!!!)  If the instruction uses
//   the direction bit, either operand can be ST0.  The direction bit is set
//   if the first operand is not ST0.  If the instruction does not use the
//   direction bit, the first operand must be ST0 if two operands are given.

// Use of the opcode table:
//	ot_val1 = First byte value
//	ot_val2 = Second byte value
//	ot_val3 = Alternate first byte (used if no operands)
//	ot_val4 = Not used

void itwooprf(opt)
struct optbl *opt;

{
    int reg1;

    stopper = nxtnb0(stopper);		// Make sure have non-blank character
    if (stopper == '\0' || stopper == '}' || stopper == ';') // Have operand?
    {
        absbyte(opt->ot_val3);		// No - operand - default is ST0, ST1
        absbyte(opt->ot_val2 + 1);
        return;
    }
    if (getopr(&opd1, opt, RB_FLOAT) && opd1.op_type == OP_REG)
    {
        if (stopper != ',')		// Have second operand?
        {
            absbyte(opt->ot_val1);	// No
            absbyte(opt->ot_val2 + regindx[(int)(opd1.op_val.vl_val.vl_vl)]);
            return;
        }
        reg1 = (int)opd1.op_val.vl_val.vl_vl; // Yes - save first register
        if (getopr(&opd1, opt, RB_FLOAT) && opd1.op_type == OP_REG)
        {
            if (reg1 == RG_ST0)		// If first operand is ST0, then
            {				//   second operand can be any
                absbyte(opt->ot_val1);	//   FPU register
                absbyte(opt->ot_val2 +
                        regindx[(int)(opd1.op_val.vl_val.vl_vl)]);
                return;
            }
            if (opd1.op_val.vl_val.vl_vl == RG_ST0 && (opt->ot_flag & OF_DB))
            {				// If first operand not ST0, second
					//   operand must be ST0 and
					//   instruction must use direction
					//   bit
                absbyte(opt->ot_val1 + 4);
                absbyte(opt->ot_val2 + regindx[reg1]);
                return;
            }
        }
    }
    adrerr();
}

// Function to handle general two operand instructions

void itwoopr(opt)
struct optbl *opt;

{
    if (getopr1(&opd1, opt, RB_DAT)	// Get first operand
	    && getopr(&opd2, opt, RB_DAT)) // Get second operand
        (*op2dsp1[opd1.op_type])(opt);	// Dispatch on type of first operand
}

// Dispatch table for second operand of two operand instruction when first
//   operand is a register

void (*op2regdsp[])(struct optbl *) =
{   op2regreg,		// OP_REG
    op2regimm,		// OP_IMM
    op2regmem,		// OP_MEM
    op2regmem,		// OP_CM1
    op2regmem		// OP_CM2
};

// Function to handle two operand instruction when first (destination) operand
//   is a register

void op2reg1(opt)
struct optbl *opt;

{
    if (opt->ot_flag & OF_RD)
        (*op2regdsp[opd2.op_type])(opt); // Dispatch on type of 2nd operand
    else
        adrerr();
}

// Function to handle two operand instruction when first (destination) operand
//   is a register and second (source) operand is also a register

void op2regreg(opt)
struct optbl *opt;

{
    if (!(opt->ot_flag & OF_RS))	// Can the source be a register?
        adrerr();			// No - fail
    else
    {
        if (opt->ot_flag & OF_RF)	// Is the first operand the register?
        {				// Yes
            opcdout(opt->ot_val1, &opd1, opt); // Output opcode byte
            absbyte(opd2.op_modrm +
                    (regindx[(int)(opd1.op_val.vl_val.vl_vl)]<<3));
        }
        else
        {
            if (opt->ot_flag & OF_DB)	// Does it use a direction bit?
            {				// Yes - output it with the direction
					//   bit set
                opcdout(opt->ot_val1 | 0x02, &opd1, opt); // Output opcode
                absbyte(opd2.op_modrm +	// Followed by mod-r/m byte
                        (regindx[(int)(opd1.op_val.vl_val.vl_vl)]<<3));
            }
            else			// If does not use direction bit
            {
                opcdout(opt->ot_val1, &opd2, opt); // Output opcode byte
                absbyte(opd1.op_modrm +	// Followed by mod-r/m byte
                        (regindx[(int)(opd2.op_val.vl_val.vl_vl)]<<3));
            }
        }
    }
}

// Function to handle two operand instruction when first (destination) operand
//   is a register and second (source) operand is immediate

void op2regimm(opt)
struct optbl *opt;

{
    if (opt->ot_val4 && (opd1.op_val.vl_val.vl_vi == RG_AL ||
            (((opd1.op_val.vl_val.vl_vi == RG_AX) ||
            (opd1.op_val.vl_val.vl_vi == RG_EAX)) && 
            (!(opt->ot_flag & OF_SB) || (opd2.op_val.vl_size > 1)))))
					// Is destination AL, AX, or EAX and
					//   we have a special form for this
					//   and we want to use it? (we do
					//   not use the special form for
					//   ADDL EAX, #n.B since it is
    {					//   longer than the normal form!
        opcdout(opt->ot_val4, &opd1, opt); // Yes
        valueout(&opd2, opt);
    }
    else
        op2memimm(opt);			// No - just like memory immediate
}

// Function to handle two operand instruction when first (destination) operand
//   is a register and second (source) operand is a memory address

void op2regmem(opt)
struct optbl *opt;

{
    if (!(opt->ot_flag & OF_MS))	// Is a memory source legal here?
    {
        adrerr();			// No - error
        return;
    }
    opcdout(opt->ot_val1 | ((opt->ot_flag & OF_DB)? 2: 0), &opd2, opt);
					// Yes - output opcode byte
    oprdout(regindx[(int)(opd1.op_val.vl_val.vl_vl)]<<3, &opd2);
					// Followed by operand byte(s)
}

// Dispatch table for second operand of two operand instruction when first
//   operand is a memory address

void (*op2memdsp[])(struct optbl *) =
{   op2memreg,				// OP_REG
    op2memimm,				// OP_IMM
    (void (*)(struct optbl *))adrerr,	// OP_MEM
    (void (*)(struct optbl *))adrerr,	// OP_CM1
    (void (*)(struct optbl *))adrerr	// OP_CM2
};

// Function to handle two operand instruction when first (destination) operand
//   is a memory reference

void op2mem1(opt)
struct optbl *opt;

{
    if (!(opt->ot_flag & OF_MD))	// Can destination be memory?
    {
        adrerr();			// No - fail
        return;
    }
    (*op2memdsp[(int)(opd2.op_type)])(opt); // Dispatch on type of second
}					    //   operand

// Function to handle two operand instruction when first (destination) operand
//   is a memory address and second (source) operand is a register

void op2memreg(opt)
struct optbl *opt;

{
    if (!(opt->ot_flag & OF_MD))	// Is memory destination legal here?
    {
        adrerr();			// No - error
        return;
    }
    opcdout(opt->ot_val1, &opd1, opt);	// Yes - output opcode byte
    oprdout(regindx[(int)(opd2.op_val.vl_val.vl_vl)]<<3, &opd1);
}					// Followed by operand byte(s)

// Function to handle two operand instruction when first (destination) operand
//   is a memory address and second (source) operand is immediate

void op2memimm(opt)
struct optbl *opt;

{
    if (!opt->ot_val2)			// Is immediate mode value legal?
    {
        adrerr();			// No - error
        return;
    }
    if (opt->ot_flag & OF_BI)		// Does this instruction always get a
    {					//   single byte immediate value?
        opcdout(opt->ot_val2, &opd1, opt); // Yes
        oprdout(opt->ot_val3, &opd1);
        putbyte(U_VALUE, &opd2.op_val);
    }
    else if (opt->ot_flag & OF_SB)	// Does this instruction use the
    {					//   size bit?
        opcdout(opt->ot_val2 | ((opd2.op_val.vl_size == 1)? 2: 0), // Yes
                &opd1, opt);
        oprdout(opt->ot_val3, &opd1);
        valuesout(&opd2, opt);
    }
    else				// Does not use size bit
    {
        opcdout(opt->ot_val2, &opd1, opt);
        oprdout(opt->ot_val3, &opd1);
        valueout(&opd2, opt);
    }
}

// Function to handle string instructions - if this is a repeated string
//   instruction, the repeat prefix is output first so this can be used
//   together with a segment prefix on processors which don't handle multiple
//   prefixes quite right!

void istrins(opt)
struct optbl *opt;

{
    char source;
    char destination;
    char both;
    char wide;

    source = ((opt->ot_flag & OF_MS) != 0);
    destination = ((opt->ot_flag & OF_MD) != 0);
    both = source & destination;
    opd1.op_seg = 0;
    opd2.op_seg = 0;
    if (both? (getopr1(&opd1, opt, 0) && getopr(&opd2, opt, 0)):
            getopr(destination? &opd1: &opd2, opt, 0))
    {
        if (both)
        {
            wide = (opd1.op_asize == 4);
            if (wide != (opd2.op_asize == 4))
            {
                adrerr();
                return;
            }
        }
        else
            wide = ((source? opd2.op_asize: opd1.op_asize) == 4);
        if ((source && opd2.op_modrm != ((wide)?6:4)) ||
                (destination && (opd1.op_modrm != ((wide)?7:5)) || opd1.op_seg))
        {
            adrerr();
            return;
        }
        if (opt->ot_val2)		// Need repeat prefix byte?
            absbyte(opt->ot_val2);	// Yes
        if (wide ^ ((segatrb & SA_32BIT) != 0))
					// Need address size prefix byte?
            absbyte(0x67);		// Yes
        if (opt->ot_size > 1)		// Might this instruction need size
					//   prefix?
        {				// Yes - does it really need one?
            if ((opt->ot_size == 2) ^ ((segatrb & SA_32BIT) == 0))
                absbyte(0x66);		// Yes
        }
        if (opd2.op_seg)		// Need segment prefix byte?
            absbyte(opd2.op_seg);	// Yes
        absbyte(opt->ot_val1);		// Finally, output opcode byte!
    }
}

// Function to handle the ENTER instruction

void ientrins(opt)
struct optbl *opt;

{
    if (getopr(&opd1, opt, 0))		// Get first operand
    {
        if (opd1.op_type == OP_MEM)
        {
            if (stopper == ',')		// Have second operand?
            {
                if (getopr(&opd2, opt, 0)) // Yes - get it
                {
                    if (opd2.op_type != OP_MEM)
                    {
                        adrerr();
                        return;
                    }
                }
                else
                    return;
            }
            else
                opd2.op_val.vl_val.vl_vi = 0;
            if (stack32 ^ ((segatrb & SA_32BIT) != 0))
					// Need operand size prefix?
                absbyte(0x66);		// Yes - output it
            absbyte(0xC8);
            putword(U_VALUE, &opd1.op_val);
            putbyte(U_VALUE, &opd2.op_val);
        }
        else
            adrerr();
    }
}

// Function to handle the IMUL instruction

void iimul(opt)
struct optbl *opt;

{
    if (!getopr(&opd1, opt, RB_DAT))	// Get first operand
        return;
    if (stopper != ',')			// Single operand?
    {					// Yes
        opcdout(opt->ot_val1, &opd1, opt);
        oprdout(0x20, &opd1);
    }
    else				// More than one operand - cannot be
    {					//   byte and must be for 80286 or
        if (opt->ot_size == 1 ||	//   80386
                opd1.op_type != OP_REG || ptype < P_80286)
        {
            adrerr();
            return;
        }
        if (!getopr(&opd2, opt, RB_DAT)) // Get second operand
            return;
        if (stopper!= ',')		// Only two operands?
        {
            if (opd2.op_type == OP_IMM)	// Yes - is second immediate?
                iimul3op(&opd1, &opd1, &opd2, opt); // Really a version of
						   //   the 3 operand form
            else
            {
                if (opd1.op_type != OP_REG || ptype < P_80386)
                {			// General 2 operand form is only
                    adrerr();		//   available on the 80386
                    return;
                }
                opcdout(0x0F, &opd2, opt); // Output 1st opcode byte
                absbyte(0xAF);		// Output 2nd opcode byte
                oprdout(regindx[(int)(opd1.op_val.vl_val.vl_vl)]<<3, &opd2);
					// Followed by operand byte(s)
            }
        }
        else				// If have three operands
        {
            if (!getopr(&opd3, opt, 0))	// Get third operand
                return;
            if (opd3.op_type != OP_IMM)	// Must be immediate
            {
                adrerr();
                return;
            }
            iimul3op(&opd1, &opd2, &opd3, opt);
        }
    }
}

// Function to generate code for three operand multiply instruction

void iimul3op(dopd, sopd, iopd, opt)
struct op    *dopd;
struct op    *sopd;
struct op    *iopd;
struct optbl *opt;

{
    opcdout(0x69 | ((iopd->op_val.vl_size == 1)? 2: 0), sopd, opt);
    oprdout(regindx[(int)(dopd->op_val.vl_val.vl_vl)]<<3, sopd);
					// Followed by operand byte(s)
    valuesout(iopd, opt);		// Followed by immediate value
}

// Function to handle IO instructions

void iinout(opt)
struct optbl *opt;

{
    stopper = nxtnbc();			// Get first character
    if (stopper == '[')			// Might it be [DX]?
    {
        stopper = nxtnbc();
        primary(&val);			// Yes - get expression
        if (stopper != ']' || val.vl_kind != VK_REG ||
                val.vl_val.vl_vi != RG_DX)
        {
            adrerr();
            return;
        }
        stopper = nxtnbc();
        iinoutop(opt->ot_val2, opt);
    }
    else
    {
        hldchar = stopper;
        exprsn();			// Evalulate port number expression
        iinoutop(opt->ot_val1, opt);
        putbyte(U_VALUE, &val);
    }
}

// Function to output opcode byte for IN or OUT instructions

void iinoutop(byte, opt)
int    byte;
struct optbl *opt;

{
    if (opt->ot_size > 1 &&
            ((opt->ot_size == 2) ^ ((segatrb & SA_32BIT) == 0)))
        absbyte(0x66);
    absbyte(byte);
}

// Function to handle INC and DEC instructions

void iincdec(opt)
struct optbl *opt;

{
    if (getopr(&opd1, opt, RB_DAT))	// Get operand
    {
        if (opd1.op_type == OP_REG && opd1.op_size != 1)
            opcdout(opt->ot_val2 + regindx[(int)(opd1.op_val.vl_val.vl_vl)],
                    &opd1, opt);
        else
        {
            opcdout(opt->ot_val1, &opd1, opt);
            oprdout(opt->ot_val4, &opd1);
        }
	return;
    }
    adrerr();
}

// Function to handle CLR instruction (this is not really an instruction, it
//   generates XOR reg,reg!)

void iclear(opt)
struct optbl *opt;

{
    if (getopr(&opd1, opt, RB_DAT))	// Get operand
    {
        if (opd1.op_type == OP_REG)
        {
            opcdout(opt->ot_val1, &opd1, opt);
            oprdout(0xC0 + (regindx[(int)(opd1.op_val.vl_val.vl_vl)] << 3),
                    &opd1);
        }
        else
            adrerr();
    }
}

// Function to handle the PUSH and POP instructions
//   optbl fields are used as follows:
//	ot_val1 - First opcode byte if general memory operand
//	ot_val2 - Opcode byte if general register operand
//	ot_val3 - Opcode byte if short form segment register operand
//	ot_val4 - Second byte if general memory operand

void ipushpop(opt)
struct optbl *opt;

{
    if (getopr(&opd1, opt, RB_DAT|RB_SEG)) // Get operand
    {
        if (opd1.op_type == OP_REG)
        {
            if (opd1.op_size == 1)
                sizerr();
            else
            {
                if (opd1.op_val.vl_val.vl_vi < RG_CS)
					// Is this a segment register?
                    opcdout(regindx[(int)(opd1.op_val.vl_val.vl_vl)] +
                            opt->ot_val2, &opd1, opt);
					// No - general register
                else if (opd1.op_val.vl_val.vl_vi < RG_FS)
					// Yes - is it a 80386 segment reg?
                    opcdout(regindx[(int)(opd1.op_val.vl_val.vl_vl)] +
                            opt->ot_val3, &opd1, opt);
					// No - non-80386 segment register
                else
                {			// Yes - this is a 80386 segment reg
                    if (ptype >= P_80386) // Yes - is this a 386 or 486?
                    {			// Yes - need size prefix?
                        if ((opt->ot_size == 2) ^
                                ((segatrb & SA_32BIT) == 0))
                            absbyte(0x66); // Yes
                        absbyte(0x0F);
                        absbyte(((opt->ot_val3 & 0x01) | 0x80) |
                                regindx[(int)(opd1.op_val.vl_val.vl_vl)]);
                    }
                    else
                        adrerr();	// Not 80386 - error
                }
            }
        }
        else if (opd1.op_type == OP_IMM) // If immediate operand
        {
            if ((unsigned char)(opt->ot_val1) == 0xFF && ptype >= P_80186)
            {				// Must be PUSH and at least a 80186
                opcdout((opd1.op_val.vl_size == 1)? 0x6A: 0x68, &opd1, opt);
                valuesout(&opd1, opt);
            }
            else
                adrerr();
        }
        else				// If memory operand
        {
            opcdout(opt->ot_val1, &opd1, opt);
            oprdout(opt->ot_val4, &opd1);
        }
    }
}

// Function to handle the XCHG instruction

void ixchg(opt)
struct optbl *opt;

{
    if (getopr1(&opd1, opt, RB_DAT)	// Get first operand
	    && getopr(&opd2, opt, RB_DAT)) // Get second operand
    {
        if (opd1.op_type != OP_REG && opd2.op_type != OP_REG)
        {
            adrerr();
            return;
        }
        if (opd1.op_type == OP_REG && opd2.op_type == OP_REG)
        {
            if (opd1.op_val.vl_val.vl_vi == RG_AX ||
                    opd1.op_val.vl_val.vl_vi == RG_EAX)
            {
                opcdout(0x90 | regindx[(int)(opd2.op_val.vl_val.vl_vl)],
                        &opd2, opt);
                return;
            }
            if (opd2.op_val.vl_val.vl_vi == RG_AX ||
                    opd2.op_val.vl_val.vl_vi == RG_EAX)
            {
                opcdout(0x90 | regindx[(int)(opd1.op_val.vl_val.vl_vl)],
                        &opd1, opt);
                return;
            }
        }
        if (opd1.op_type == OP_REG)
        {
            opcdout(opt->ot_val1, &opd2, opt); // Output opcode byte
            oprdout(regindx[(int)(opd1.op_val.vl_val.vl_vl)]<<3, &opd2);
        }				// Followed by operand byte(s)
        else
        {
            opcdout(opt->ot_val1, &opd1, opt); // Yes - output opcode byte
            oprdout(regindx[(int)(opd2.op_val.vl_val.vl_vl)]<<3, &opd1);
        }				// Followed by operand byte(s)
    }
}

// Function to handle general branch instructions
//   Usage of the optable fields are as follows:
//	ot_val1 - First opcode byte for 8 bit displacement form (0 if not
//		    available)
//	ot_val2 - Second opcode byte for 8 bit displacement forms (0 if only
//		    one opcode byte is needed)
//	ot_val3 - First opcode byte for 16 or 32 bit displacement forms (0 if
//		    not available)
//	ot_val4 - Second opcode byte for 16 or 32 bit displacement forms (0 if
//		    only one opcode byte is needed)
//   Note that if no size is specified, a 16/32 bit displacement is used if it
//     is available; if a 16/32 bit form is not available, the 8 bit form is
//     used.

void ibranch(opt)
struct optbl *opt;

{
    char jsize;

    if (getopr(&opd1, opt, 0))		// Get operand
    {
        if (opd1.op_type == OP_MEM)
        {
            if (opd1.op_val.vl_type == VL_VAL) // Size specified?
            {
                if (opt->ot_val3 &&	// No - have 16/32 bit form?
                        (ptype >= P_80386 || !(opt->ot_flag & OF_B3)))
		{			// Yes - use it
                    if (segatrb & SA_32BIT) // 32 bit segment?
                    {
                        opd1.op_val.vl_type = VL_LREL; // Yes - use 32 bits
                        opd1.op_val.vl_size = 4;
                    }
                    else
                    {
                        opd1.op_val.vl_type = VL_WREL; // No - use 16 bits
                        opd1.op_val.vl_size = 2;
                    }
                }
                else			// No 16/32 bit form - use 8 bit
                {			//   form of instruction
                    opd1.op_val.vl_type = VL_BREL;
                    opd1.op_val.vl_size = 1;
                }
            }
            if (opd1.op_val.vl_type < VL_BREL)
            {
                if (!opd1.op_val.vl_undef) // Undefined expression?
                    adrerr();		// No - report address error
                return;
            }
            if (opd1.op_val.vl_type == VL_BREL)
            {
                if (opt->ot_val1 == 0)
                {
                    adrerr();
                    return;
                }
            }
            else
            {
                if (opt->ot_val3 == 0)
                {
                    adrerr();
                    return;
                }
            }
            if (opt->ot_flag & OF_SS)	// Is this a CALLxx?
            {				// Yes - must make width match stack
					//   size! - ignore operand size!!
                jsize = stack32? VL_LREL: VL_WREL;
                if ((stack32 ^ ((segatrb & SA_32BIT) != 0)))
					// Need operand size prefix?
                    absbyte(0x66);	// Yes - output it
            }
            else
            {
                jsize = opd1.op_val.vl_type;
                if ((jsize != VL_BREL) &&    // Need size prefix?
                        ((jsize == VL_WREL) ^
                        ((segatrb & SA_32BIT) == 0)))
                {
                    if (ptype >= P_80386)	// Yes - can we do this?
                        absbyte(0x66);	// Yes
                    else
                    {
                        adrerr();
                        return;
                    }
                }
            }
            if (jsize == VL_BREL)
            {
                absbyte(opt->ot_val1);
                if (opt->ot_val2)
                    absbyte(opt->ot_val2);
                opd1.op_val.vl_val.vl_vl -= 1;
                putbyte(S_VALUE, &opd1.op_val);
            }
            else
            {
                absbyte(opt->ot_val3);
                if (opt->ot_val4)
                    absbyte(opt->ot_val4);
                if (jsize == VL_WREL)
                {
                    opd1.op_val.vl_val.vl_vl -= 2;
                    putword(S_VALUE, &opd1.op_val);
                }
                else
                {
                    opd1.op_val.vl_val.vl_vl -= 4;
                    putlong(S_VALUE, &opd1.op_val);
                }
            }
        }
        else
            adrerr();
    }
}

// Function to handle loop instructions
//   Usage of the optable fields are as follows:
//	ot_val1 - Opcode byte

void iloop(opt)
struct optbl *opt;

{
    if (getopr(&opd1, opt, RB_DAT))	// Get first operand
    {
        if (opd1.op_type == OP_REG && (opd1.op_val.vl_val.vl_vl == RG_CX
                || opd1.op_val.vl_val.vl_vl == RG_ECX))
        {
            if (getopr(&opd2, opt, 0))	// Get second operand
            {
                if (opd2.op_type == OP_MEM)
                {
                    opd2.op_val.vl_type = VL_BREL; // Always use 8 bit form
                    opd2.op_val.vl_size = 1;
                    if ((opd1.op_val.vl_val.vl_vl == RG_CX)
                            ^ ((segatrb & SA_32BIT) == 0))
                    {			// Need address size prefix?
                        if (ptype >= P_80386) // Yes - can we do this?
                            absbyte(0x67); // Yes
                        else
                        {
                            adrerr();
                            return;
                        }
                    }
                    absbyte(opt->ot_val1);
                    opd2.op_val.vl_val.vl_vl -= 1;
                    putbyte(S_VALUE, &opd2.op_val);
                }
            }
        }
        else
            adrerr();
    }
}

// Function to handle the JMPF instruction

void ijumpf(opt)
struct optbl *opt;

{
    char offset16;

    if (getopr(&opd1, opt, RB_DAT))	// Get operand
    {
        if (opd1.op_type == OP_MEM)
        {
            if (opd1.op_val.vl_type == VL_VAL) // Size specified?
            {
                if (segatrb & SA_32BIT) // No - 32 bit segment?
                {
                    opd1.op_val.vl_type = VL_LONG; // Yes - use 32 bits
                    opd1.op_val.vl_size = 4;
                }
                else
                {
                    opd1.op_val.vl_type = VL_WORD; // No - use 16 bits
                    opd1.op_val.vl_size = 2;
                }
            }
            offset16 = (opd1.op_val.vl_type != VL_LONG);
            if (offset16 ^ ((segatrb & SA_32BIT) == 0))
            {
                if (ptype >= P_80386)	// Yes - can we do this?
                    absbyte(0x66);	// Yes
                else
                {
                    adrerr();
                    return;
                }
            }
            absbyte(0xEA);
            if (offset16)
                putword(U_VALUE, &opd1.op_val);
            else
                putlong(U_VALUE, &opd1.op_val);
            putsel(&opd1.op_val);
        }
        else
            adrerr();
    }
}

// Function to handle the CALLF instruction

void icallf(opt)
struct optbl *opt;

{
    if (getopr(&opd1, opt, RB_DAT))	// Get operand
    {
        if (opd1.op_type == OP_MEM)
        {
            if ((stack32 ^ ((segatrb & SA_32BIT) != 0)))
					// Need operand size prefix?
                absbyte(0x66);		// Yes - output it
            absbyte(0x9A);
            if (stack32)
                putaddrl(&opd1.op_val);
            else
                putaddrw(&opd1.op_val);
        }
        else
            adrerr();
    }
}

// Function to handle INT instruction

void iintins(opt)
struct optbl *opt;

{
    if (getopr(&opd1, opt, 0))		// Get operand
    {   
        if (opd1.op_type == OP_MEM && opd1.op_val.vl_type == VL_VAL)
        {
            absbyte(0xCD);
            putbyte(U_VALUE, &opd1.op_val);
        }
        else
            adrerr();
    }
}

// Function to handle shift and rotate instructions

void ishrot(opt)
struct optbl *opt;

{
    if (getopr1(&opd1, opt, RB_DAT))	// Get first operand
    {
        stopper = nxtnbc();
        if (stopper != '#')		// Immediate value?
        {
            primary(&val);		// No - must be CL (can't call
					//   getopr here because size may
					//   not match!
            if (val.vl_kind != VK_REG || val.vl_val.vl_vi != RG_CL)
            {
                adrerr();
                return;
            }
            opcdout(opt->ot_val2, &opd1, opt); // Output opcode byte
            oprdout(opt->ot_val4, &opd1);
        }
        else
        {
            exprsn();			// Get immediate value
            if (val.vl_kind == VK_ABS && val.vl_val.vl_vl == 1) // Is it 1?
            {
                opcdout(opt->ot_val1, &opd1, opt);
                oprdout(opt->ot_val4, &opd1);
            }
            else
            {
                if (ptype < P_80286)
                {
                    adrerr();
                    return;
                }
                opcdout(opt->ot_val3, &opd1, opt);
                oprdout(opt->ot_val4, &opd1);
                putbyte(U_VALUE, &val);
            }
        }
    }
}

// Function to handle double precision shift instructions

void idblshf(opt)
struct optbl *opt;

{
    if (getopr1(&opd1, opt, RB_DAT) &&	// Get first and second operands
            getopr1(&opd2, opt, RB_DAT))
    {
        stopper = nxtnbc();
        if (stopper != '#')		// Immediate value?
        {
            primary(&val);		// No - must be CL (can't call
					//   getopr here because size may
					//   not match!
            if (val.vl_kind != VK_REG || val.vl_val.vl_vi != RG_CL)
            {
                adrerr();
                return;
            }
            opcdout(opt->ot_val1, &opd1, opt); // Output opcode byte
            oprdout(regindx[(int)(opd2.op_val.vl_val.vl_vl)]<<3, &opd1);

        }
        else
        {
            exprsn();			// Get immediate value
            opcdout(opt->ot_val2, &opd1, opt);
            oprdout(regindx[(int)(opd2.op_val.vl_val.vl_vl)]<<3, &opd1);
            putbyte(U_VALUE, &val);
        }
    }
}

// Function to handle RET and RETF instructions

void iret(opt)
struct optbl *opt;

{
    if (((opt->ot_size)? (opt->ot_size == 4): stack32) ^
            ((segatrb & SA_32BIT) != 0)) // Need operand size prefix?
        absbyte(0x66);			// Yes - output it
    if (ifend(stopper))			// Have operand?
        absbyte(opt->ot_val1);		// No - output value for opcode
    else
    {
        exprsn();			// Get fixup amount
        absbyte(opt->ot_val2);		// Output opcode
        putword(U_VALUE, &val);		// Output fixup amount (always 
    }					//   16 bits)
}

// Function to handle one byte instructions with no operands

void ionebyte(opt)
struct optbl *opt;

{
    absbyte(opt->ot_val1);		// Output value for opcode
}

// Function to handle one byte instructions with no operands which may require
//   an operand size prefix

void ionebytx(opt)
struct optbl *opt;

{
    if (((opt->ot_size)? (opt->ot_size == 4): stack32) ^
            ((segatrb & SA_32BIT) != 0)) // Need operand size prefix?
        absbyte(0x66);			// Yes - output it
    absbyte(opt->ot_val1);		// Output value for opcode
}

// Function to handle two byte instructions with no operands

void itwobyte(opt)
struct optbl *opt;

{
    absbyte(opt->ot_val1);		// Output first byte of opcode
    absbyte(opt->ot_val2);		// Output second byte of opcode
}

// Function to handle three byte instructions with no operands

void ithrbyte(opt)
struct optbl *opt;

{
    absbyte(opt->ot_val1);		// Output first byte of opcode
    absbyte(opt->ot_val2);		// Output second byte of opcode
    absbyte(opt->ot_val3);		// Output third byte of opcode
}

// Function to output single opcode byte

void opcdout(
    long   value,
    struct op    *opd,
    struct optbl *opt)

{
    if (opd->op_seg)			// Need segment override prefix?
        absbyte(opd->op_seg);		// Yes
    if (opd->op_apfx)			// Need address size prefix?
        absbyte(opd->op_apfx);		// Yes
    if (opt->ot_size > 1 && !(opt->ot_flag & OF_NS))
					// Might this instruction need an
					//   operand size prefix?
    {					// Yes - does it really need one?

//      if ((opt->ot_size == 2) ^ !(segatrb & SA_32BIT))
//	Above genrates bad code under TC 2.0!

        if ((opt->ot_size == 2) ^ ((segatrb & SA_32BIT) == 0))
            absbyte(0x66);		// Yes
    }
    else if ((opt->ot_flag & OF_SS) &&
            (stack32 ^ ((segatrb & SA_32BIT) != 0)))
        absbyte(0x66);
    if (opt->ot_flag & OF_XP)		// Need 30386 prefix?
        absbyte(0x0F);			// Yes
    absbyte(value);			// Output opcode byte
}

// Function to output operand

void oprdout(
    long   extra,
    struct op *opd)

{
    absbyte(opd->op_modrm | extra);	// Output first operand byte
    if ((unsigned char)(opd->op_sib) != 0xFF) // Need sib byte?
        absbyte(opd->op_sib);		// Yes
    switch((int)opd->op_nab)
    {
    case 1:
        putbyte(S_VALUE, &opd->op_val);
        break;
    case 2:
        putword(U_VALUE, &opd->op_val);
        break;
    case 4:
        putlong(U_VALUE, &opd->op_val);
        break;
    }
}

// Function to output data value for instruction which uses the S bit

void valuesout(
    struct op    *opd,
    struct optbl *opt)

{
    if (opt->ot_size ==  1)
        putbyte(U_VALUE, &opd->op_val);
    else if (opd->op_val.vl_size == 1)
        putbyte(S_VALUE, &opd->op_val);
    else if (opt->ot_size == 2)
        putword(U_VALUE, &opd->op_val);
    else
        putlong(U_VALUE, &opd->op_val);
}

// Function to output data value for instruction which does not use the S bit

void valueout(
    struct op    *opd,
    struct optbl *opt)

{
    if (opt->ot_size ==  1)
        putbyte(U_VALUE, &opd->op_val);
    else if (opt->ot_size == 2)
        putword(U_VALUE, &opd->op_val);
    else
        putlong(U_VALUE, &opd->op_val);
}

// Function to report addressing error
//   Returns FALSE

int adrerr(void)

{
    skiprest();
    if (!(errflag & ~(ERR_U|ERR_V)))
	errsub(erramsg, ERR_A);
    return (FALSE);
}

// Function to report size error

void sizerr(void)

{
    skiprest();
    errsub(errsmsg, ERR_S);
}
