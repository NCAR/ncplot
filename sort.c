/*
-------------------------------------------------------------------------
OBJECT NAME:	sort.c

FULL NAME:	

ENTRY POINTS:	SortTable()

STATIC FNS:	sort_the_table()

DESCRIPTION:	

INPUT:		table, start & position in table to sort

OUTPUT:		none

REFERENCES:	none

REFERENCED BY:	hdr_decode.c

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-8
-------------------------------------------------------------------------
*/

#include <string.h>

static char	*mid, *temp, **sort_table;

static void sort_the_table(int, int);

/* -------------------------------------------------------------------- */
void SortTable(char **table, int beg, int end)
{
	sort_table = table;
	sort_the_table(beg, end);

}	/* SORTTABLE */

/* -------------------------------------------------------------------- */
static void sort_the_table(int beg, int end)
{
	int	x = beg,
		y = end;

	mid = sort_table[(x + y) / 2];

	while (x <= y)
		{
		while (strcmp(sort_table[x], mid) < 0)
			++x;

		while (strcmp(sort_table[y], mid) > 0)
			--y;

		if (x <= y)
			{
			temp = sort_table[x];
			sort_table[x] = sort_table[y];
			sort_table[y] = temp;

			++x;
			--y;
			}
		}

	if (beg < y)
		sort_the_table(beg, y);

	if (x < end)
		sort_the_table(x, end);

}	/* END SORT_THE_TABLE */

/* END SORT.C */
