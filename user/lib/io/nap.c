//http://plan9.bell-labs.com/sources/plan9/sys/src/ape/lib/v/nap.c
int
nap(int n)
{
	register i;

	while(n-- > 0){
		for(i = 0; i < 1000*1000*10; i++)
			;
	}
	return 0;
}

