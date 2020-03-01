//**************************
// Pseudo-op tables for XMAC
//**************************
// Written by John Goltz
//**************************

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

// Define normal pseudo-op table

struct pstbl psutbl[] =
{ {{'A','D','D','R','L',' '}, paddrl},
  {{'A','D','D','R','W',' '}, paddrw},
  {{'A','L','M','E','X',' '}, palmex},
  {{'A','S','C','I','I',' '}, pascii},
  {{'A','S','C','I','L',' '}, pascil},
  {{'A','S','C','I','U',' '}, pasciu},
  {{'A','S','C','I','Z',' '}, pasciz},
  {{'B','L','K','B',' ',' '}, pblkb},
  {{'B','L','K','L',' ',' '}, pblkl},
  {{'B','L','K','W',' ',' '}, pblkw},
  {{'B','L','M','E','X',' '}, pblmex},
  {{'B','Y','T','E',' ',' '}, pexpb},
  {{'C','O','T',' ',' ',' '}, pcot},
  {{'C','R','E','F',' ',' '}, pcref},
  {{'D','E','B','U','G',' '}, pdebug},
  {{'E','N','D','C',' ',' '}, pendc},
  {{'E','N','T','R','Y',' '}, pentry},
  {{'E','R','R','O','R',' '}, peerror},
  {{'E','V','E','N',' ',' '}, peven},
  {{'E','X','P','B',' ',' '}, pexpb},
  {{'E','X','P','L',' ',' '}, pexpl},
  {{'E','X','P','O','R','T'}, pexport},
  {{'E','X','P','W',' ',' '}, pexpw},
  {{'E','X','T','E','R','N'}, pextern},
  {{'I','F',' ',' ',' ',' '}, pif},
  {{'I','F','F',' ',' ',' '}, piff},
  {{'I','F','T',' ',' ',' '}, pift},
  {{'I','F','T','F',' ',' '}, (void (*)())incond},
  {{'I','M','P','O','R','T'}, pimport},
  {{'I','N','C','L','U','D'}, pinclud},
  {{'I','N','T','E','R','N'}, pintern},
  {{'I','R','P',' ',' ',' '}, pirp},
  {{'I','R','P','C',' ',' '}, pirpc},
  {{'L','B','E','X',' ',' '}, plbex},
  {{'L','I','S','T',' ',' '}, plist},
  {{'L','N','K','S','E','G'}, plnkseg},
  {{'L','O','N','G',' ',' '}, pexpl},
  {{'L','S','B',' ',' ',' '}, plsb},
  {{'L','S','Y','M',' ',' '}, plsym},
  {{'M','A','C','R','O',' '}, pmacro},
  {{'M','O','D',' ',' ',' '}, pmod},
  {{'M','S','E','C','T',' '}, pmsect},
  {{'N','C','H','A','R',' '}, pnchar},
  {{'N','C','O','T',' ',' '}, pncot},
  {{'N','C','R','E','F',' '}, pncref},
  {{'N','G','S','Y','M',' '}, pngsym},
  {{'N','L','B','E','X',' '}, pnlbex},
  {{'N','L','I','S','T',' '}, pnlist},
  {{'N','L','M','E','X',' '}, pnlmex},
  {{'N','L','S','Y','M',' '}, pnlsym},
  {{'N','R','N','A','M','E'}, pnrname},
  {{'O','D','D',' ',' ',' '}, podd},
  {{'P','A','G','E',' ',' '}, ppage},
  {{'P','A','R','M',' ',' '}, pparm},
  {{'P','R','I','N','T',' '}, pprint},
  {{'P','R','N','A','M','E'}, pprname},
  {{'P','R','O','C',' ',' '}, pproc},
  {{'P','S','E','C','T',' '}, ppsect},
  {{'R','A','D','I','X',' '}, pradix},
  {{'R','E','P','T',' ',' '}, prept},
  {{'R','E','Q','U','I','R'}, prequir},
  {{'S','B','T','T','L',' '}, psbttl},
  {{'S','E','G',' ',' ',' '}, pseg},
  {{'S','E','G','1','6',' '}, pseg16},
  {{'S','E','G','3','2',' '}, pseg32},
  {{'S','L','M','E','X',' '}, pslmex},
  {{'S','T','A','C','K',' '}, pstack},
  {{'S','T','A','R','T',' '}, pstart},
  {{'S','T','K','1','6',' '}, pstk16},
  {{'S','T','K','3','2',' '}, pstk32},
  {{'S','T','Y','P','E',' '}, pstype},
  {{'T','I','T','L','E',' '}, ptitle},
  {{'W','O','R','D',' ',' '}, pexpw}
};

int psusize = sizeof(psutbl) / sizeof(struct pstbl);
int psuents = sizeof(struct pstbl);

// Define table of pseudo-op checked for false conditionals

struct cptbl cptbl[] =
{ {{'E','N','D','C',' ',' '}, cpendc},
  {{'I','F',' ',' ',' ',' '}, cpif},
  {{'I','F','F',' ',' ',' '}, cpiff},
  {{'I','F','T',' ',' ',' '}, cpift},
  {{'I','F','T','F',' ',' '}, cpiftf}
};

int cpsize = sizeof(cptbl) / sizeof(struct cptbl);
int cpents = sizeof(struct cptbl);

// Define condition table for .IF pseudo-op

struct iftbl ifctbl[] =
{ {{'B',' ',' '}, ifb},
  {{'D','E','F'}, ifdef},
  {{'D','I','F'}, ifdif},
  {{'E','Q',' '}, ifeq},
  {{'G','E',' '}, ifge},
  {{'G','T',' '}, ifgt},
  {{'I','D','N'}, ifidn},
  {{'L','E',' '}, ifle},
  {{'L','T',' '}, iflt},
  {{'N','B',' '}, ifnb},
  {{'N','D','F'}, ifndf},
  {{'N','E',' '}, ifne},
  {{'P','1',' '}, ifp1},
  {{'P','2',' '}, ifp2}
};

int ifsize = sizeof(ifctbl) / sizeof(struct iftbl);
int ifents = sizeof(struct iftbl);
