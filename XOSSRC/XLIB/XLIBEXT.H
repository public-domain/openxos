/*
 * External declarations for XLIB
 *
 * Written by John Goltz / tmg
 */

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

extern char   prgname[];
extern int    cmdindex;
extern int    symsize;
extern int    outdd;
extern int    pageno;
extern int    headsz;
extern long   modoffset;
extern long   firstmod;
extern char  *membtm;
extern struct obj      inplib;
extern struct obj      inpfil;
extern struct nameblk *firstname;
extern struct nameblk *thisname;
extern struct nameblk *prevname;
extern struct nameblk *libname;
extern struct et      *prevet;
extern struct et       ethead;
extern char  *outfsp;
extern char  *listfsp;
extern FILE  *listfp;
extern FILE  *outfp;
extern char   cmdline;
extern char   lowfirst;
extern char   havlname;
extern char   nocmd;
extern char   symbuf[];
extern char   tempname[];
extern char   notstart[];
