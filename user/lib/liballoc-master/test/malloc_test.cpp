#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>


#define MAX_BLOCKS			100
#define MAX_SIZE			(1024 * 1024)
#define MAX_TIME			( 1 * 60 )


/** A testing block to hold all allocated data. */
struct block
{
	unsigned char *data;
	int size;
	unsigned char key;
};



/** The testing blocks. */
static struct block blocks[ MAX_BLOCKS ];
static long long totalMemory = 0;
static int totalBlocks = 0;


static int g_verbose = 0;




static int malloc_random( int verbose )
{
	g_verbose = verbose;
	totalMemory = 0;
	totalBlocks = 0;

	printf("malloc_random: this will take %i minute...\n", MAX_TIME/ 60 );
	

	for ( int i = 0; i < MAX_BLOCKS; i++ )
	{
		blocks[ i ].data = NULL;
		blocks[ i ].size = 0;
		blocks[ i ].key  = 0;
	}

	int transactions = 0;
	time_t start_time = time(NULL);

	//	Random madness.
	while (1==1)
	{
		int position = rand() % MAX_BLOCKS;

		int diff = time(NULL) - start_time;
		if ( diff > ( MAX_TIME ) ) break;
		
		int tps = (++transactions) / (diff + 1);



		  if ( blocks[position].data == NULL )
		  {
			blocks[position].size = rand() % MAX_SIZE;
			blocks[position].data = (unsigned char*)malloc( blocks[position].size );
			blocks[position].key  = rand() % 256;

			if ( g_verbose != 0 )
				printf("%i left, %i tps : %i, %i : %i: allocating %i bytes with %i key\n", 
								( MAX_TIME - diff ),
								tps,
								totalBlocks * 100 / MAX_BLOCKS,
								(int)(totalMemory / (1024)),
								position,
								blocks[position].size,
								blocks[position].key );

			if ( blocks[position].data != NULL )
			{
				totalMemory += blocks[position].size;
				totalBlocks += 1;

				for ( int j = 0; j < blocks[position].size; j++ )
					blocks[position].data[j] = blocks[position].key;
			}

		  } 
		  else
		  {
				for ( int j = 0; j < blocks[position].size; j++ )
					if ( blocks[position].data[j] != blocks[position].key )
					{
						printf( "%i: %x (%i bytes, position %i) %i != %i: ERROR! Memory not consistent",
										position, 
										blocks[position].data, 
										blocks[position].size, 
										j,
										blocks[position].data[j], 
										blocks[position].key );
						abort();
					}


				if ( g_verbose != 0 )
					printf("%i left, %i tps : %i, %i : %i: freeing %i bytes with %i key\n", 
								( MAX_TIME - diff ),
								tps,
								totalBlocks * 100 / MAX_BLOCKS,
								(int)(totalMemory / (1024)),
								position,
								blocks[position].size,
								blocks[position].key );
			
				free( blocks[position].data );
				blocks[position].data = NULL;

				totalMemory -= blocks[position].size;
				totalBlocks -= 1;
		  }

	}	

	// Dump the memory map here.
		
		
	// Free.
	for ( int i = 0; i < MAX_BLOCKS; i++ )
	{
		if ( blocks[ i ].data != NULL ) free( blocks[ i ].data );
		blocks[ i ].size = 0;
		blocks[ i ].key  = 0;
	}


	// Final results.
	printf("%i TPS, %i%s USAGE\n", transactions / MAX_TIME, totalBlocks * 100 / MAX_BLOCKS, "%" );
					
	return 0;
}




static int malloc_large( int verbose )
{
	g_verbose = verbose;

	printf("malloc_large: going to exhaust the memory...\n" );
	
	for ( int i = 0; i < MAX_BLOCKS; i++ )
		blocks[ i ].data = NULL;

	int transactions = 0;
	time_t start_time = time(NULL);

	for ( int i = 0; i < MAX_BLOCKS; i++ )
	{
		blocks[ i ].data = (unsigned char*)malloc( MAX_SIZE );
		if ( blocks[i].data == NULL ) break;

		transactions += 1;
	}	

	for ( int i = 0; i < MAX_BLOCKS; i++ )
		if ( blocks[ i ].data != NULL ) free( blocks[ i ].data );


	// Final results.
	printf("%i blocks of %i size = %i MB, %i seconds\n",
			 transactions, 
			 MAX_SIZE,
			 (transactions * MAX_SIZE) / (1024 * 1024),
			 time(NULL) - start_time
			 );
					
	return 0;
}



int malloc_test( int verbose )
{
	malloc_random( verbose );
	malloc_random( verbose );
	malloc_random( verbose );
	malloc_large( verbose );
	malloc_large( verbose );
	malloc_large( verbose );
	return 0;
}



