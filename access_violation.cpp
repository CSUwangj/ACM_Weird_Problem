#include<stdio.h>

int main()
{
	int a[16], b[16], t;
	printf("%p %p %p\n", &a, &b, &t);
	for(int i = 0; i < 34 ; ++ i){
		a[i] = i;
	}
	for(int i = 0; i < 16 ; ++ i){
		printf("%d ", b[i]);
	}
	printf("%d\n", t);
	return 0;
}
