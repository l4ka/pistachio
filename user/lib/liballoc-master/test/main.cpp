#include <stdlib.h>
#include <stdio.h>



extern int malloc_test( int verbose );


int main( int argc, char *argv )
{
	int verbose = 0;
	if ( argc > 1 ) verbose = 1;

	printf("%s\n","memory testing application" );

	malloc_test( verbose );

	printf("%s\n","all tests passed!");
	
	return EXIT_SUCCESS;
}



