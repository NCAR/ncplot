/*
-------------------------------------------------------------------------
OBJECT NAME:	search.c

FULL NAME:	Searches

ENTRY POINTS:	SearchTable()

DESCRIPTION:	Search for target in list.  The last pointer in the list
		array must be NULL.  It only compares the first word of
		what the list items points to.

INPUT:		An array of pointers and target pointer.

OUTPUT:		pointer to located item or NULL

COPYRIGHT:	University Corporation for Atmospheric Research, 1992-2005
-------------------------------------------------------------------------
*/

#include <ctype.h>

#include "define.h"


/* -------------------------------------------------------------------- */
int SearchTable(std::vector<VARTBL*>& table, std::string target)
{
  for (size_t i = 0; i < table.size(); ++i)
    if (table[i]->name == target)
      return(i);

  return(-1);

}	/* END SEARCHTABLE */

/* END SEARCH.C */
