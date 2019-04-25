#include "types.h"
#include "stat.h"
#include "user.h"

#define N 10000

int
main(int argc, char *argv[])
{
	int z = 0;
	printf(1,"Comienzo \n");

	for(int i = 0; i < N; i++)
		for(int j = 0; j < N; j++)
			z++;

	printf(1,"Fin test, z es: %d \n",z);
	exit();
}
