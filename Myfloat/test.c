#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "float.h"
union {
	float f;
	dword d;
} u;

int main()
{
	char s[50];
	float f;
	/*dword a = 0x80000000, i;
	scanf("%f", &u.f);
	for (i = 0; i < 32; i++)
	{
		if (u.d & a) printf("1");
		else printf("0");
		a >>= 1;
		if (i == 0 || i == 8)
			printf(" ");
	}*/
	
	scanf("%s", s);
	u.d = atof(s);
	printf("%.10f\n", u.f);
	scanf("%f", &f);
	printf("%.10f\n", f);


	return 0;
}
