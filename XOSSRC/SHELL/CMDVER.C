//------------------------------------------------------------------------------
// CMDVER.C - shell and XOS version numbers
// 
// Written by Bruce R. Nevins
// 
// Edit History:
// -------------
// 05/17/89(brn) - Created first version
// 05/22/91(brn) - Moved date version string out of version module
// 08/27/96(brn) - Add fflush if detached process
//------------------------------------------------------------------------------*

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

#include <STDIO.H>
#include <XOS.H>
#include <XOSSVC.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

int cmdver(void)
{
    union
    {
        struct xos_ver
        {
            unsigned edit : 16;
            unsigned minor : 8;
            unsigned major : 8;
        } x_version;
        unsigned long x_long;
    } a;

    fprintf(cmdout, "%s version %d.%d (%s)\n", prgname, major_version,
		minor_version, date_version);

    a.x_long = getsyschar("XOSVER");	/* Get XOS version */
    fprintf(cmdout, "XOS version %d.%d.%d\n", a.x_version.major,
            a.x_version.minor, a.x_version.edit);

    a.x_long = getsyschar("DOSVER");	/* Get DOS emulator version */
    fprintf(cmdout, " with DOS version %d.%d emulation\n", a.x_version.major,
            a.x_version.minor);
    if (detached)
        fflush(cmdout);
    return (TRUE);
}
