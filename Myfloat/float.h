#ifndef _LXFLOAT_H
#define _LXFLOAT_H
#define NUMEWIDTH	50
typedef unsigned int dword;
typedef unsigned long long qword;

dword Jatof(char *s);
char *Jftoa(dword f);

#endif
