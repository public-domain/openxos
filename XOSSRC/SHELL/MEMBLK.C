//------------------------------------------------------------------------------
// MEMBLK.C - Memory block functions for SHELL and BATCH
//
// Written by Bruce R. Nevins
//
// Edit History:
// -------------
// 02/18/90(brn) - Added new function blkstrnicmp
// 08/16/92(brn) - Change stderr and stdout to cmderr and cmdout
// 03/28/94(brn) - Add blkungetc function
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

#include <CTYPE.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <XOS.H>
#include "SHELL.H"
#include "SHELLEXT.H"
#include "BATCHEXT.H"

#define DEBUG 0                 /* Define for debugging */

struct firstblk *freelist = NULL;       /* Pointer to first free block */
/*
 * getblock - Get a free memory block
 */
struct firstblk *getblock(void)
{
    struct firstblk *temp;

    if ((temp = freelist) != NULL)
        freelist = (struct firstblk *)freelist->a.next; /* Point to next block in the list */
    else
    {
        temp = (struct firstblk *)sbrk(BLOCKSIZE);      /* Return pointer to memory */
        if (temp == NULL)
        {
            fprintf(cmderr, "?getblock - No Memory Available\n");
	    if (detached)
		fflush(cmderr);
            return (NULL);
        }
    }


    temp->a.next = NULL;                /* Make sure next block pointer is clear */
    temp->a.txtptr = NULL;
    temp->a.txtcount = 0;
    temp->a.totalcnt = 0;
    return (temp);
}

/*
 * giveblock - Return a memory block to the free list
 */
struct firstblk *giveblock(struct firstblk *blkptr)
{
    struct nextblk *temp;

    if (blkptr == NULL)         /* If don't have a block just return */
    {
        fprintf(cmderr, "giveblock - Called with NULL pointer\n");
	if (detached)
	    fflush(cmderr);
        return (blkptr);
    }

#ifdef DEBUG
    temp = (struct nextblk *)freelist;

    while ((blkptr != (struct firstblk *)temp) && (temp != NULL))
        temp = temp->next;              /* Get next in list */

    if (temp != NULL)
    {
        fprintf(cmderr, "?giveblock - Memory block pointer was in free list\n");
	if (detached)
	    fflush(cmderr);
        return (blkptr);
    }
#endif

    temp = blkptr->a.next;              /* Next block if any */
    blkptr->a.next = (struct nextblk *)freelist;                /* Link list to new block */
    freelist = blkptr;  /* New list pointer */

    blkptr = (struct firstblk *)temp;                   /* Point to next block if any */
    return (blkptr);
}

/*
 * givechain - Return a linked chain of memory blocks to the free list
 */
struct firstblk *givechain(struct firstblk *blkptr)
{
    register struct nextblk *temp;

    if (blkptr == NULL)         /* If don't have a block just return */
        return (blkptr);

#if DEBUG
    temp = freelist;

    while (temp != NULL)
    {
        while (blkptr != temp)
            temp = temp->a.next;        /* Get next in list */
    
        if (temp == NULL)
        {
              temp = blkptr->a.next;    /* Get next in the list */
            blkptr->a.next = freelist;  /* Link list to new block */
            freelist = blkptr;  /* New list pointer */
            blkptr = temp;              /* Move up block to check */
        }
        else
        {
            fprintf(cmderr, "?giveblock - Memory block pointer was in free list\n");
	    if (detached)
	    	fflush(cmderr);
            return (NULL);
        }
    }
#else
    temp = (struct nextblk *)blkptr;
    while (temp->next != NULL)
        temp = temp->next;

    temp->next = (struct nextblk *)freelist;            /* Link in the free list */
    freelist = blkptr;  /* Point to new free list */
#endif
    return (NULL);                      /* Make sure pointer is cleared */
}

/*
 * blkgetc - Get a character from a block
 */
char blkgetc(register struct firstblk *blkptr)
{
    struct nextblk *tempblock;
    char temp;

    if ((temp = blkptr->a.saved) != '\0')
    {
	blkptr->a.saved = '\0';
	return (temp);
    }

    if (blkptr->a.txtcount == 0)
    {
        tempblock = (struct nextblk *)(blkptr->a.txtptr - BLOCKSIZE);
        tempblock = tempblock->next;    /* Link to next block in chain */

        if (tempblock == NULL)  /* If no more data assume NULL */
            return ('\0');

        blkptr->a.txtptr = &tempblock->text[0]; /* Point to next data */
        blkptr->a.txtcount = BLOCKSIZE - sizeof(struct nextblk *);
                                /* Max. number of available characters */
    }
    temp = *blkptr->a.txtptr++; /* Get the next character */
    blkptr->a.txtcount--;

    if (temp == '\0')           /* If end of string make end of block */
    {
        blkptr->a.txtptr += (int)blkptr->a.txtcount; /* Point to the end */
        blkptr->a.txtcount = 0; /* Clear count */
    }

    return (temp);
}

/*
 * blkungetc - Unget a character from a block
 */
int blkungetc(register struct firstblk *blkptr, char chr)
{
    if (blkptr->a.saved != '\0')
	return (FALSE);

    blkptr->a.saved = chr;
    return (TRUE);
}

/*
 * blkputc - Put a character in a block
 */
struct firstblk *blkputc(struct firstblk *blkptr, char newchr)
{
    struct nextblk *newblock, *tempblock;       /* Pointer to memory block */

    if ((blkptr == NULL) || (blkptr->a.txtcount == 0))
    {
        if ((newblock = (struct nextblk *)getblock()) == NULL)  /* Get next block */
        {
            fprintf(cmderr, "? %s: Out of memory - Command lost.\n", prgname);
            if (blkptr != NULL) /* Make sure there is a block to free */
                blkptr = givechain(blkptr);     /* Give up our old chain of memory */
                                        /*  blocks */
	    if (detached)
	    	fflush(cmderr);
            return (NULL);
        }
        if (blkptr == NULL)
        {
            blkptr = (struct firstblk *)newblock;       /* Make first block */
            blkptr->a.txtptr = &blkptr->text[0]; /* Set up the text pointer */
            blkptr->a.txtcount = BLOCKSIZE - sizeof(struct firsthdr);
                                        /* Number of available characters */
	    blkptr->a.saved = 0;	// No saved char
        }
        else
        {
            tempblock = (struct nextblk *)(blkptr->a.txtptr - BLOCKSIZE);
            tempblock->next = newblock; /* Link to last block in chain */
            blkptr->a.txtptr = &newblock->text[0];
                                        /* Skip past the next pointer */
            blkptr->a.txtcount = BLOCKSIZE - sizeof(struct nextblk *);
                                        /* Number of available characters */
        }
    }
    *blkptr->a.txtptr++ = newchr & 0x7F;
    blkptr->a.txtcount--;       /* Drop the block count */
    blkptr->a.totalcnt++;       /* Bump the total count */
    return (blkptr);
}

/*
 * blkinic - Init a block pointer for use by blkputc and blkgetc
 */
void blkinic(struct firstblk *blkptr)
{
    if (blkptr == NULL)
    {
        fprintf(cmderr, "? %s: blkinic - Call with NULL pointer\n", prgname);
	if (detached)
	    fflush(cmderr);
        return;
    }
    blkptr->a.txtptr = &blkptr->text[0];        /* Point to the first character */
    blkptr->a.txtcount = BLOCKSIZE - sizeof(struct firsthdr);
                                                /* Init count */
    return;
}

/*
 * Function to copy second string into first string
 */

struct firstblk * blkcpy(struct firstblk *outblk, struct firstblk *inblk)
{
    register char chr;

    outblk = givechain(outblk);
    while ((chr = blkgetc(inblk)) != '\0')
    {
        outblk = blkputc(outblk, chr&0x7F);
    }

    outblk = blkputc(outblk, 0);	/* Include the NULL byte */
    return (outblk);
}

/*
 * Function to concatinate second string into first string
 */

struct firstblk * blkcat(struct firstblk *outblk, struct firstblk *inblk)
{
    register char chr;

    while ((chr = blkgetc(inblk)) != '\0')
    {
        outblk = blkputc(outblk, chr);
    }

    outblk = blkputc(outblk, chr);              /* Include the NULL byte */
    return (outblk);
}

/*
 * blklen - Function to count charaters in a block string
 * With out changing the character pointer or count
 */

long blklen(register struct firstblk *inblk)
{
    long totalcount;
    long blockcount;
    char *chrptr;

    if (inblk == NULL)
    {
        fprintf(cmderr, "? %s: blklen - Called with a NULL pointer\n",
		prgname);
	if (detached)
	    fflush(cmderr);
        return (0);
    }

    totalcount = 0;
    blockcount = inblk->a.txtcount;
    chrptr = inblk->a.txtptr;

    while (blkgetc(inblk) != '\0')
        totalcount++;

    inblk->a.txtptr = chrptr;
    inblk->a.txtcount = blockcount;

    return (totalcount);
}

/*
 * blkputs - Function to print a block string
 * With out changing the character pointer or count
 * returns the count of characters printed
 */

long blkputs(register struct firstblk *inblk, FILE *stream)
{
    long totalcount;
    char chr;

    if (inblk == NULL)
    {
        return (0L);
    }

    totalcount = 0;

    while ((chr = blkgetc(inblk)) != '\0')
    {
        fputc(chr, stream);
        totalcount++;
    }
    return (totalcount);
}

/*
 * blkstrncmp - Function to compare two block strings, with limit
 */

int blkstrncmp(register struct firstblk *blk1, register struct firstblk *blk2, register long num)
{
    char chr1, chr2;

    while (num-- != 0)
    {
        chr1 = blkgetc(blk1);
        chr2 = blkgetc(blk2);
        if (chr1 != chr2)
            break;

        if (chr1 == '\0')
            return (0);                 /* They're equal to the end */
    }
    return (chr1 - chr2);
}

/*
 * blkstrnicmp - Function to compare two block strings, with limit, ignoring
 *		  case
 */

int blkstrnicmp(register struct firstblk *blk1, register struct firstblk *blk2, register long num)
{
    char chr1, chr2;

    while (num-- != 0)
    {
        chr1 = toupper(blkgetc(blk1));
        chr2 = toupper(blkgetc(blk2));
        if (chr1 != chr2)
            break;

        if (chr1 == '\0')
            return (0);                 /* They're equal to the end */
    }
    return (chr1 - chr2);
}

/*
 * blkstrcmp - Function to compare two block strings
 */

int blkstrcmp(register struct firstblk *blk1, register struct firstblk *blk2)
{
    char chr1, chr2;

    while (TRUE)
    {
        chr1 = blkgetc(blk1);
        chr2 = blkgetc(blk2);
        if (chr1 != chr2)
            break;

        if (chr1 == '\0')
            return (0);                 /* They're equal to the end */
    }
    return (chr1 - chr2);
}

