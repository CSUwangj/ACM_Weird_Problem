#include <stdio.h>

int main()
{
	int a;
	char *st = "%d";
	printf("%p", a);
	printf("%p", st);
	scanf(st, a);
	return 0;
}
