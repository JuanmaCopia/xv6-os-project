#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	int i;
	int j;
	int z = 0;
	printf(1,"Comienzo \n");

	for (i = 0; i < 752150000; i++)
		for (j = 0; j < 752150000; j++)
			z = (-i*2) + z + i*2 + 1;

	printf(1,"Fin test, z es: %d \n",z);
	exit();
}
