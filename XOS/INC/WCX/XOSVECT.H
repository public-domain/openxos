//-------------------------------------------------------------------------
// XOSVECT.H
//
// Written by: John R. Goltz
//
// Edit History:
// 09/07/92(brn) - Add comment Header
//
//-------------------------------------------------------------------------

// ++++
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

#ifndef _XOSVECT_H
#define _XOSVECT_H

# include <_prolog.h>

// Define software interrupt numbers

// Interrupts 0x000 through 0x0FF are protected mode INTs

// Interrupts 0x100 through 0x11F are protected mode processor traps

#define VECT_PDIVERR  0x100	// Divide error trap
#define VECT_PTRACE   0x101	// Trace trap
#define VECT_PNMI     0x102	// Non-maskable interrupt
#define VECT_PBRKPNT  0x103	// Breakpoint trap
#define VECT_PINTO    0x104	// INTO instruction trap
#define VECT_PBOUND   0x105	// BOUND instruction trap
#define VECT_PILLINS  0x106	// Illegal instruction trap
#define VECT_PFPPNAVL 0x107	// FPP not available trap
#define VECT_PDBLEXP  0x108	// Double exception trap
#define VECT_PFPPSOVR 0x109	// FPP segment overrun trap
#define VECT_PITSS    0x10A	// Invalid task state segment
#define VECT_PSEGNP   0x10B	// Segment not present
#define VECT_PSTKERR  0x10C	// Stack overflow
#define VECT_PPROT    0x10D	// Memory protection error
#define VECT_PPAGEFLT 0x10E	// Page fault

#define VECT_PFPPERR  0x110	// Floating point processor error
#define VECT_PALNCHK  0x111	// Alignment check

// Interrupts 0x120 through 0x13F are real mode processor traps

#define VECT_RDIVERR  0x120	// Divide error
#define VECT_RTRACE   0x121	// Trace trap
#define VECT_RNMI     0x122	// Non-maskable interrupt
#define VECT_RBRKPNT  0x123	// Breakpoint
#define VECT_RINTO    0x124	// INTO instruction trap
#define VECT_RBOUND   0x125	// BOUND instruction trap
#define VECT_RILLINS  0x126	// Illegal instruction trap
#define VECT_RFPPNAVL 0x127	// FPP not available trap
#define VECT_RDBLEXP  0x128	// Double exception
#define VECT_RFPPSOVR 0x129	// PPU segment overrun
#define VECT_RITSS    0x12A	// Invalid task state segment

#define VECT_RSTKERR  0x12C	// Stack overflow
#define VECT_RPROT    0x12D	// Memory protection error
#define VECT_RPAGEFLT 0x12E	// Page fault

#define VECT_RFPPERR  0x130	// Floating point error
#define VECT_RALNCHK  0x131	// Alignment check

// Interrupts 0x140 through 0x15F are XOS traps

#define VECT_EXIT     0x140	// Process termination trap
#define VECT_CHILD    0x141	// Child died
#define VECT_CNTC     0x142	// Control C
#define VECT_CNTP     0x143	// Control P
#define VECT_HNGUP    0x144	// Controlling terminal hung up

// Interrupts 0x160 through 0x1FF are reserved

// Interrupts 0x200 through 0x2FF are real mode INTs

// Define interrupt vector type values

#define VT_NONE    0	// Idle
#define VT_XOSS    1	// XOS signal vector
#define VT_XOST    2	// XOS trap vector
#define VT_HWS16   3	// 16-bit hardware signal vector
#define VT_HWS32   4	// 32-bit hardware signal vector
#define VT_HWT16   5	// 16-bit hardware trap vector
#define VT_HWT32   6	// 32-bit hardware trap vector
#define VT_DPMI16O 7	// DPMI v0.9 16-bit CPU exception vector
#define VT_DPMI32O 8	// DPMI v0.9 32-bit CPU exception vector
#define VT_DPMI16N 9	// DPMI v1.0 16-bit CPU exception vector
#define VT_DPMI32N 10	// DPMI v1.0 32-bit CPU exception vector

int setvector(short vec, short level, void (*func)());

# include <_epilog.h>

#endif	// _XOSVECT_H_
