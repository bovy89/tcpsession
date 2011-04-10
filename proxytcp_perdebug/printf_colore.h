#ifndef  __PRINTF_COLORE_H__
#define  __PRINTF_COLORE_H__

#include <stdio.h>
#include <stdarg.h>


#define DEFAULTCOLOR "\033[0m"
#define ROSSO  "\033[22;31m"
#define VERDE  "\033[22;32m"

#define PROSSO(X)        do { printf(ROSSO);        (X); printf(DEFAULTCOLOR); } while(0)
#define PVERDE(X)        do { printf(VERDE);        (X); printf(DEFAULTCOLOR); } while(0)
#define PDEFAULTCOLOR(X) do { printf(DEFAULTCOLOR); (X); printf(DEFAULTCOLOR); } while(0)

/* esempio di chiamata
void main (void) {
	// Mprintf_C( ROSSO, ( "%d %s\n", 30, "stringa ROSSO" ) );
	// Mprintf_C( VERDE, ( "%d %s\n", 30, "stringa VERDE" ) );
	PROSSO(  printf("%d %s\n", 30, "stringa ROSSO"  ) );
	PVERDE(  printf("%d %s\n", 30, "stringa VERDE"  ) );
	PDEFAULTCOLOR( printf("%d %s\n", 30, "stringa DEFAULTCOLOR" ) );
}
*/

#endif


