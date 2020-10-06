#include <stdio.h>

int main()
{
	int a=123;
	char *st = "%d";
	printf("%p\n", a);
	printf("%p\n", st);
	scanf(st, a);
	return 0;
}
