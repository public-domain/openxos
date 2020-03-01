#include <STDIO.H>
#include <STDLIB.H>
#include <CTYPE.H>


//******************************************************************
// Function: sortindex - Sort an index using its comparison function
// Returned: Nothing
//******************************************************************

// This function implements a heap sort with the heap implemented in a
//   linear array (the index array). This routine has somewhat more overhead
//   than quicksort, but it is always of order n(log n) and it requires
//   no additional memory or stack space, no matter how large the array is.
//   This routine is from Data Structures Using C by Tenenbaum, Langsam,
//   and Augenstein, (C) 1990, by Prentice-Hall.

void aheapsort(
    void **base,
    int    size,
    int  (*comp)(void *key1, void *key2))

{
    void *elt;
    int   i;
    int   s;
    int   f;

    // Preprocessing phase; create the initial heap

    for (i = 1; i < size; i++)
    {
		elt = base[i];

		// Insert the element in its proper place in the tree

		s = i;
		f = (s - 1)/2;
		while (s > 0 && comp(base[f], elt) < 0)
		{
			base[s] = base[f];
			s = f;
			f = (s - 1)/2;
		}
		base[s] = elt;
	}

	// Selection phase: repeatedly remove x[0], insert it in its proper
	//   position and adjust the heap

	for (i = size - 1; i > 0; i--)
	{
		// pqmaxdelete(x, i + 1);

		elt = base[i];
		base[i] = base[0];
		f = 0;

		// s = largeson (0, i - 1);

		s = ((i == 1) ? -1 : 1);
		if (i > 2 && comp(base[2], base[1]) > 0)
			s = 2;
		while (s >= 0 && comp(elt, base[s]) < 0)
		{
			base[f] = base[s];
			f = s;

			// s = largeson(f, i - 1);

			s = 2 * f + 1;
			if ((s + 1) <= (i - 1) && comp(base[s], base[s+1]) < 0)
				s++;
			if (s > (i - 1))
				s = -1;
		}
		base[f] = elt;
	}
}
