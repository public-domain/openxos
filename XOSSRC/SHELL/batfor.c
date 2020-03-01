//------------------------------------------------------------------------------
//
//  batfor.c - FOR routine for BATCH
//
//  Written by Bruce R. Nevins
//
//  Edit History:
//  -------------
//  11/16/89(brn) - Removed / for path specifier
//  05/13/91(brn) - Change to return TRUE if command accepted
//  04/25/94(brn) - Convert comments and modify synerr calls
//  02/29/96(brn) - Fix FOR statement handling
//
//------------------------------------------------------------------------------

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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <xos.h>
#include <xossvc.h>
#include <xoserr.h>
#include <xoserrmsg.h>
#include <xosstr.h>
#include "shell.h"
#include "shellext.h"
#include "batchext.h"

#define SIZE 50			// Size of filename buffer

struct forparm
{   byte4_parm  filoptn;
    lngstr_parm filspec;
    byte4_parm  dirofs;
    char        end;
} forparm =
{   {(PAR_SET|REP_HEXV), 4, IOPAR_FILOPTN, FO_FILENAME},
    {(PAR_GET|REP_STR ), 0, IOPAR_FILSPEC, NULL, 0, SIZE, 0},
    {(PAR_GET|PAR_SET|REP_HEXV), 4, IOPAR_DIROFS, 0}
};

//
// Function to process FOR command
//

int batfor(void)

{
    long     rtn;
    uchar    returnname[SIZE];	// Name returned by open
    char     curvarname[FILESPCSIZE];// New name to place in replacable param
    struct   firstblk *newcmd;	// New command to execute
    struct   firstblk *expndlcl;// Local copy of expanded line
    struct   firstblk *tempblk;	// Temp copy of command string tail
    char     chr;				// Temp for holding the character being
								//   checked
    char     wildname[FILESPCSIZE]; // Wildcard name
    char     wildpath[FILESPCSIZE]; // Wildcard path spec.
    char    *newstr;			// Pointer for new CMD
    char    *temppos;			// Temp string pointer
    char     localvar;			// Local variable name, 1 character
    char     endseen;			// Flag for end of string of filenames seen
    char     lclkeyword[256];	// Local keyword
    char     inwild;			// TRUE if in wildcard expansion
    char    *wildptr;			// Pointer into wild card string
    char    *tokptr;			// Pointer to next token for strchr
    char    *name;				// Pointer to the current name in the buffer
    long     iotemp;
    type_qab forqab;

	// Init variables

    forparm.filspec.buffer = (char *)returnname; // Buffer for returned text
    forparm.dirofs.value = 0;
    forqab.qab_func = QFNC_WAIT|QFNC_DEVPARM;
    forqab.qab_vector = 0;
    forqab.qab_option = O_REPEAT;
    forqab.qab_buffer2 = NULL;
    forqab.qab_parm = &forparm;

    inwild = FALSE;             	// TRUE if in wildcard expansion
    endseen = FALSE;            	// End of filename string not seen yet

    expndlcl = NULL;            	// Let copy allocate the block
    newcmd = NULL;
    tempblk = NULL;
    iotemp = 0;
    wildptr = NULL;

    blkinic(expndarg);					// Start from beginning of string
    expndlcl = blkcpy(expndlcl, expndarg); // Make a copy in case we recurse
    blkinic(expndlcl);					// Start from beginning of string
    tempblk = blkcpy(tempblk, expndlcl);// Copy string to play with
    blkinic(expndlcl);          		// Start from beginning of string

    batnkeyw(&lclkeyword[0], expndlcl);	// Get rid of command part

	// Look for variable

    if (!batnkeyw(&lclkeyword[0], expndlcl)) // Get keyword
    {
        synerr("FOR:", NULL);			// Syntax error msg
        expndlcl = givechain(expndlcl);
        tempblk = givechain(tempblk);
        return (TRUE);
    }

    if (lclkeyword[0] == '%')			// Was it a variable?
    {
        if (!isalnum(localvar = lclkeyword[1]))
        {
            synerr("FOR:", NULL);		// Syntax error msg
            expndlcl = givechain(expndlcl);
            tempblk = givechain(tempblk);
            return (TRUE);
        }
    }
    else
    {
        synerr("FOR:", NULL);			// Syntax error msg
        expndlcl = givechain(expndlcl);
        tempblk = givechain(tempblk);
        return (TRUE);
    }

	// look for IN

    if (!batnkeyw(&lclkeyword[0], expndlcl)) // Get keyword
    {
        synerr("FOR:", NULL);			// Syntax error msg
        expndlcl = givechain(expndlcl);
        tempblk = givechain(tempblk);
        return (TRUE);
    }

    if (stricmp(&lclkeyword[0], "IN") != 0)
    {
        synerr("FOR:", NULL);			// Syntax error msg
        expndlcl = givechain(expndlcl);
        tempblk = givechain(tempblk);
        return (TRUE);
    }

	// Get first file name or pattern

    while (!endseen || inwild)
    {
        if (inwild)
        {
//printf("inwild\n");
            strcpy(curvarname, wildpath); // Prepend path
//printf("inwild: name = %s\n", name);
            if ((tokptr = strchr(name, FS_FILENAME)) != NULL || *name != '\0')
            {
				if (tokptr != NULL)
				{
					strncat(curvarname, name, tokptr - name); // Add the file name
					name = tokptr + 1;
				}
				else
				{
//printf("inwild: FS_FILENAME not seen\n");
                    strcat(curvarname, name); // Add the file name
					name = name + strlen(name); // Point to end of string
				}
            }
            else
            {
                if ((iotemp & 0x80000000) != 0)
                {
//printf("inwild: Getting next block\n");
                    if ((rtn=svcIoQueue(&forqab)) < 0 ||
							(rtn = forqab.qab_error) < 0)
					{
//printf("inwild: Error on read\n");
                        if (rtn == ER_FILNF)
                        {
                            inwild = FALSE;
                            if (endseen)
                                break;
                            else
                                continue;
                        }
                        else
                        {
                            inwild = FALSE;
                            femsg(prgname, rtn, keyword);
                            if (endseen)
                                break;
                            else
                                continue;
                        }
                    }
                    else
                    {
                        iotemp = forqab.qab_amount; // Save the item count
                        strcpy(curvarname, wildpath); // Prepend path

						// Get next token

						name = (char *)forparm.filspec.buffer + 1;
//printf("name = %s\n", name);
						if ((tokptr = strchr(name, FS_FILENAME)) != NULL)
						{
							strncat(curvarname, name, tokptr - name);
							name = tokptr + 1; // Add the file name
						}
						else
						{
//printf("inwild: FS_FILENAME not seen\n");
							strcat(curvarname, name); // Add the file name
							name = name + strlen(name);
						}				// Point to end of string
                    }
                }
                else
                {
                    inwild = FALSE;
                    if (endseen)
                        break;
                    else
                        continue;
                }
            }
        }

        if (!inwild)
        {
//printf("Not inwild\n");
            if (!batnkeyw(&lclkeyword[0], expndlcl)) // Get keyword
            {
                synerr("FOR:", NULL);	// Syntax error msg
                break;
            }
        
            temppos = &lclkeyword[0];
//printf("Not inwild: temppos = %s\n", temppos);
            if (*temppos == '(')	// Look for the (
                temppos++;		// Skip to start of filename
    
            strcpy(curvarname, temppos); // Copy the variable name
            if ((temppos = strchr(curvarname, ')')) != NULL)
            {
                *temppos = '\0';	// Get rid of ) on file name
                endseen = TRUE;
            }
//
// check if wild card expansion is needed
//
            if (strpbrk(curvarname, "*?") != NULL)
            {
                strcpy(wildname, curvarname);
                strcpy(wildpath, curvarname);
                if (strpbrk(wildpath, ":\\") == NULL)
                    wildpath[0] = '\0'; /* NUll string */
                else
                {
                    if ((wildptr = strrchr(wildpath, '\\')) == NULL)
                            if ((wildptr = strrchr(wildpath, ':')) == NULL)
                            {
                                synerr("FOR: Device or Path specifier failed",
										NULL);
                                break;
                            }
							*++wildptr  = '\0'; // Drop all but the path
                }

                forparm.filoptn.value = FO_FILENAME;
                forqab.qab_buffer1 = wildname;
                if ((rtn=svcIoQueue(&forqab)) < 0 ||
						(rtn = forqab.qab_error) < 0)
                {
                    if (rtn == ER_FILNF)
                    {
                        inwild = FALSE;
                        if (endseen)
                            break;
                        else
                            continue;
                    }
                    else
                    {
                        inwild = FALSE;
                        femsg(prgname, rtn, keyword);
                        if (endseen)
                            break;
                        else
                            continue;
                    }
                }
                else
                {
                    iotemp = forqab.qab_amount; // Save the item count
                    strcpy(curvarname, wildpath); // Prepend path
					name = (char *)forparm.filspec.buffer + 1;
                    if ((tokptr = strchr(name, FS_FILENAME)) == NULL) // Get keyword
                    {
//printf("Not inwild: FS_FILENAME not seen\n");
						strcat(curvarname, name); // Add the file name
                        inwild = FALSE;
                        if (endseen)
                            break;
                        else
                            continue;
                    }
                    strncat(curvarname, name, tokptr - name); // Add the file name
					name = tokptr + 1;
                    inwild = TRUE;
                }
            }
        }

		// Search for DO

        blkinic(tempblk);				// Reset to beginning
        while (TRUE)
        {
            if(!batnkeyw(lclkeyword, tempblk))
            {
                synerr("FOR:", NULL);	// Syntax error msg
                expndlcl = givechain(expndlcl);
                tempblk = givechain(tempblk);
                return (TRUE);
            }
            if ((strchr(&lclkeyword[0], ')')) != NULL)
                break;
        }
        if (!batnkeyw(&lclkeyword[0], tempblk)) // Get keyword
        {
            synerr("FOR:", NULL);		// Syntax error msg
            break;
        }
        if (stricmp(&lclkeyword[0], "DO") != 0)
        {
            synerr("FOR: Could not find DO", NULL); // Syntax error msg
            break;
        }

        while ((chr = blkgetc(tempblk)) != '\0')
        {
            if (chr == '%')
            {
                if ((chr = blkgetc(tempblk)) == localvar)
                {
                    newstr = curvarname;
                    while ((chr = *newstr++) != '\0')
                    {
                        newcmd = blkputc(newcmd, chr);
                    }
                }
                else					// If not our local variable, just
                {						//   insert back in
                    newcmd = blkputc(newcmd, '%');
                      newcmd = blkputc(newcmd, '%');
                    newcmd = blkputc(newcmd, chr);
                }
            }
            else
                newcmd = blkputc(newcmd, chr);
        }
        newcmd = blkputc(newcmd, '\0');	// End the line

        blkinic(newcmd);				// Start command at beginning
        dointcmd(newcmd);
        newcmd = givechain(newcmd);		//  and the command
    }
    expndlcl = givechain(expndlcl);
    tempblk = givechain(tempblk);
    return (TRUE);
}
