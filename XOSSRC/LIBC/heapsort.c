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
#include <STRING.H>

typedef struct item
{   struct item *c1;
    struct item *c2;
    char  *name;
    int    value;
} item;

int   path;
item *root;

/* long  cmpcnt; */

item *heapsort(
    item *list,
    int (*comp)(item *a, item *b, void *data),
    void *data)

{
    item **pntr;
    item  *first;
    item  *last;
    item  *cp;
    item  *co;
    item  *c1;
    item  *c2;
    int    levelmask;

///	cmpcnt = 0;
    if (list == NULL)					// Make sure list not empty
        return (NULL);
    root = list;						// Make first item into a one
    list = list->c1;					//   item heap
    root->c1 = NULL;
    root->c2 = NULL;
    path = 0;
    while (list != NULL)				// Loop for each item in list
    {
        first = list->c1;
        list->c1 = NULL;
        list->c2 = NULL;
        levelmask = 1;					// Start at the top
        last = root;
        pntr = (item **)&root;
        for(;;)							// Scan down the current path
        {								//   to find place for this item
///			cmpcnt++;
            if (comp(list, last, data) < 0) // Does it go here?
            {
                for (;;)				// Yes - put it in in place of
                {						//   this item
                    *pntr = list;
                    list->c1 = last->c1;
                    list->c2 = last->c2;
                    if (path & levelmask) // Now replace next item in path
                    {					  //   with the one we just removed
                        if (list->c2 == NULL)
                        {
                            list->c2 = last; // If at end of path, just
                            last->c1 = NULL; //   extend the path with
                            last->c2 = NULL; //  this item
                            break;
                        }
                        pntr = &list->c2;
                    }
                    else
                    {
                        if (list->c1 == NULL)
                        {
                            list->c1 = last;
                            last->c1 = NULL;
                            last->c2 = NULL;
                            break;
                        }
                        pntr = &list->c1;
                    }
                    list = last;		// Advance down the path
                    last = *pntr;
                    levelmask <<= 1;
                }
                break;
            }
            else						// If new item does not go at
            {							//   current position, continue
                if (path & levelmask)
                {
                    pntr = &last->c2;
                    if ((last = last->c2) == NULL)
                    {					// If at end of path, just add it
                        *pntr = list;	//   to the end of the path
                        break;
                    }
                }
                else
                {
                    pntr = &last->c1;
                    if ((last = last->c1) == NULL)
                    {
                        *pntr = list;
                        break;
                    }
                }
                levelmask <<= 1;
            }
        }
        list = first;
        ++path;							// Advance insert path
    }
    first = NULL;
    last = (item *)&first;

///	printf("### compares = %ld\n", cmpcnt);

    // At this point we have constructed an accending heap - now we
    //   convert it to a sorted list

    for (;;)
    {
        cp = root->c1;					// Put current root on the end of
        co = root->c2;					//   the list we are building
        last->c1 = root;
        root->c1 = NULL;
        last = root;
        if (co == NULL)					// Is right child null?
        {
            if (cp == NULL)				// Yes - is left child also null?
                break;					// Yes - all done
            root = cp;					// No - use left child
        }
        else if (cp == NULL)			// Right child not null - is left
        {								//   child null?
            root = co;					// Yes - use right child
            cp = co;
            co = NULL;
        }
        else if (comp(cp, co, data) < 0) // Neither child is null - must
        {								 //   compare
///			cmpcnt++;
            root = cp;					// Left child is smaller - use it
        }
        else
        {								// Right child is smaller - use it
///			cmpcnt++;
            root = co;
            co = cp;
            cp = root;
        }
        for (;;)						// Now loop to promote nodes up the
        {								//   tree to fill it in - at this
										//   point cp points to the child
										//   to promote and co points to
										//   the other child of the same
										//   node
            c1 = cp->c1;
            c2 = cp->c2;
            cp->c2 = co;
            if (c2 == NULL)
            {
                if (c1 == NULL)
                    break;
                cp->c1 = c1;
                cp = c1;
                co = NULL;
            }
            else if (c1 == NULL)
            {
                cp->c1 = c2;
                cp = c2;
                co = NULL;
            }
            else if (comp(c1, c2, data) < 0)
            {
///				cmpcnt++;
                cp->c1 = c1;
                cp = c1;
                co = c2;
            }
            else
            {
///				cmpcnt++;
                cp->c1 = c2;
                cp = c2;
                co = c1;
            }
        }
    }

///	printf("### compares = %ld\n", cmpcnt);

    first->c2 = last;
    return (first);
}
