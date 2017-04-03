#ifndef _LXFLOAT_H
#define _LXFLOAT_H
#define NUMEWIDTH	50
#define POSINF		0x7F800000;	// inf
#define NEGINF		0xFF800000; // -inf
typedef unsigned int		dword;
typedef unsigned long long	qword;
typedef unsigned char		byte;

dword Jatof(char *);
char *Jftoa(dword);
dword Jfadd(dword, dword);
dword Jfsub(dword, dword);
dword Jfmul(dword, dword);
dword Jfdiv(dword, dword);

#endif
