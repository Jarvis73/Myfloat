#include <stdio.h>
#include "float.h"
#pragma warning(disable:4996)
extern char Jnumerical[NUMEWIDTH][NUMEWIDTH];

union {
	float f;
	dword d;
} u, u1, u2;

int main()
{
	scanf("%f %f", &u1.f, &u2.f);
	u.d = Jfadd(u1.d, u2.d);
	printf("%f + %f = %.10f\n", u1.f, u2.f, u.f);
	printf("%f + %f = %.10f\n", u1.f, u2.f, u1.f + u2.f);
	u.d = Jfsub(u1.d, u2.d);
	printf("%f - %f = %.10f\n", u1.f, u2.f, u.f);
	printf("%f - %f = %.10f\n", u1.f, u2.f, u1.f - u2.f);
	u.d = Jfmul(u1.d, u2.d);
	printf("%f * %f = %.10f\n", u1.f, u2.f, u.f);
	printf("%f * %f = %.10f\n", u1.f, u2.f, u1.f * u2.f);
	u.d = Jfdiv(u1.d, u2.d);
	printf("%f / %f = %.10f\n", u1.f, u2.f, u.f);
	printf("%f / %f = %.10f\n", u1.f, u2.f, u1.f / u2.f);

/*
	dword a = 0x80000000, i;
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
