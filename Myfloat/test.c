#include <stdio.h>
#include "float.h"
#pragma warning(disable:4996)
extern char Jnumerical[NUMEWIDTH][NUMEWIDTH];

union {
	float f;
	dword d;
} u;

int main()
{
	/*char s[50];
	float f;*/
	dword a = 0x80000000, i;
	
	/*
	scanf("%s", s);
	u.d = Jatof(s);
	printf("%.10f\n", u.f);
	scanf("%f", &f);
	printf("%.10f\n", f);
*/ 
	char *s;
	scanf("%f", &u.f);
	s = Jftoa(u.d);
	printf("%s\n", s);
/*
	for (i = 0; i < 32; i++)
	{
		if (u.d & a) printf("1");
		else printf("0");
		a >>= 1;
		if (i == 0 || i == 8)
			printf(" ");
	}
	printf("\n");
*/

	return 0;
}
