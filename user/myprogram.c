#include "types.h"
#include "stat.h"
#include "user.h"

int
main()
{
	int n = nice(0);

	switch (n) {
		case 0:
			printf(1, "Nice es 0\n");
			break;
		case 1:
			printf(1, "Nice es 1\n");
			break;
		case 2:
			printf(1, "Nice es 2\n");
			break;
		case 3:
			printf(1, "Nice es 3\n");
			break;
		default:
			printf(1, "ERROR: Nice no valido\n");
	}

	n = nice(2);
	n = nice(2);

	switch (n) {
		case 0:
			printf(1, "el nuevo Nice es 0\n");
			break;
		case 1:
			printf(1, "el nuevo Nice es 1\n");
			break;
		case 2:
			printf(1, "el nuevo Nice es 2\n");
			break;
		case 3:
			printf(1, "el nuevo Nice es 3\n");
			break;
		default:
			printf(1, "ERROR: el nuevo Nice no es valido\n");
	}

	exit();
}

