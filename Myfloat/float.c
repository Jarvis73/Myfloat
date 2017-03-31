#include <stdio.h>
#include "float.h"
typedef unsigned long long qword;

/*
  dword atof(char *s)
  ת��˼·��
	1. ��ȡ����
	2. ��ȡ�������֣�������8λʮ���ƣ���Ϊfloat���͵ľ���ֻ��ʮ���Ƶ�7.1λ
	3. �����������8λ���������ȡС�������������ȡʣ��������ֻ����ף�����С��
	4. ���������Ķ����Ƴ��ȣ����������ڣ�
	5. ���������Ķ����Ƴ��ȣ������ף�
	6. �Է��š������ƽס��������֡�С�����ֽ������
		1) �����ƽ�==0
		2) �����ƽ�<=24
		3) �����ƽ�>=25
  ���ڵ����⣺
    �������ϴ�ʱ�������
	С��0.00000001������������
*/
dword atof(char *s)
{
	dword i, j, cnt = 1, tmp;
	dword sign = 0, order, inte = 0, nume = 0, biorder = 0, binume = 0, \
		intebiorder = 0,exbinume = 0;
	dword flt = 0;

	while (*s == ' ') 
		s++;
	// ------------------------------ �趨С���ķ��� ----------------------------------
	if (*s == '-')
	{
		sign = 0x80000000;
		s++;
	}
	else if (*s == '+') 
		s++;
	while (*s == '0')
		s++;

	// -------------------------------- ��ȡǰ8λ���� ---------------------------------- 
	order = 0; // ��ౣ��8λ����
	while (*s != '.' && *s != '\0' && order < 8)
	{
		inte = inte * 10 + *s - '0';
		s++;
		order++;
	}

	// --------------------------------- ��ȡС������ ----------------------------------
	if (*s == '.')
	{	
		s++;
		i = order; // ������С���ܹ���ౣ��8λ
		while (*s != '\0' && i < 8)
		{
			nume = nume * 10 + *s - '0';
			s++;
			cnt *= 10;
			i++;
		}
		// С������ת�ɶ�����
		for (j = 0; j < 32; j++)
		{
			nume <<= 1;
			if (nume >= cnt)
			{
				binume = (binume << 1) + 1;
				nume -= cnt;
			}
			else
				binume = (binume << 1); // + 0
		}
		for (j = 0; j < 32; j++)
		{
			nume <<= 1;
			if (nume >= cnt)
			{
				exbinume = (exbinume << 1) + 1;
				nume -= cnt;
			}
			else
				exbinume = (exbinume << 1); // + 0
		}
	}
	else // -------------------------- ������ȡ���µ����� ------------------------------
	{	
		while (*s != '\0' && *s != '.')
		{
			order++;
			s++;
		}
	}

	// -------------------------- �����������ֵĶ����Ƴ���biorder -----------------------
	while (order > 8)
	{
		inte >>= 1;
		biorder++;
		if (inte <= 429496729)
		{
			inte *= 10;
			order--;
		}
	}
	tmp = inte;
	while (tmp)
	{
		biorder++;
		tmp >>= 1;
	}

	// -------------------------- ����inte��������λ��intebiorder -----------------------
	tmp = inte;
	while (tmp)
	{	
		intebiorder++;
		tmp >>= 1;
	}

	// ---------------------- ������(sign,biorder,inte,binume)��ʼ��� ------------------
	if (intebiorder == 0)
	{	// ��������Ϊ0
		tmp = binume;
		while (!(tmp & 0x80000000)) // ȥ��С�����ֿ�ͷ��0 
		{
			biorder++;
			tmp <<= 1;
		}
		if (biorder <= 8)
		{
			inte = (binume >> (8 - biorder)) & 0xFF7FFFFF; // ȥ����λ��1
			if (((binume >> (7 - biorder)) & 1) == 1)
				inte++;
		}
		else
			inte = ((binume << (biorder - 8)) | (exbinume >> (40 - biorder))) & 0xFF7FFFFF;
		flt = sign | ((127 - 1 - biorder) << 23) | inte;
	}
	else if (intebiorder <= 24)
	{	// inte����25λ����С������
		inte = (inte << (24 - intebiorder)) & 0xFF7FFFFF; // ȥ����λ��1
		inte |= (binume >> (8 + intebiorder));
		if (((binume >> (7 + intebiorder)) & 1) == 1)
			inte++;
		flt = sign | ((biorder + 127 - 1) << 23) | inte;
	}
	else
	{	// inte�㹻25λ��������С�����֣�ȡ�м�ǰ23λ����(��λ��1Ҫȥ��)
		tmp = inte;
		inte = (inte >> (intebiorder - 24)) & 0xFF7FFFFF;
		if (((tmp >> (intebiorder - 25)) & 1) == 1)
			inte++;
		flt = sign | ((biorder + 127 - 1) << 23) | inte;
	}
	return flt;
}


/*
  
*/
char *ftoa(dword f)
{
	char s[50], *sp;
	char rev[11], *srev;
	char rev2[21], *srev2;
	dword i;
	dword sign, biorder, binume;
	dword tmp, inte = 0, order = 0;
	qword nume = 0;

	sp = s;
	srev = rev;
	srev2 = rev2;
	sign = (f >> 31);
	biorder = ((f & 0x7FFFFFFF) >> 23);
	binume = (f & 0x007FFFFF) | 0x00800000; // ��λ��1

	if (!sign)	// �������
		*(sp++) = '-';
	
	// ������������inte��С������nume��ʮ���ƽ�order
	if (biorder >= 127)
	{	// ����1
		biorder -= 127;
		if (biorder < 23)
		{	// binumeͬʱ��������С��
			inte = (binume >> (23 - biorder));
			tmp = (0xFFFFFFFF >> (9 + biorder));
			binume &= tmp;
			// С��������תʮ����
			biorder = 23 - biorder;
			i = 1;
			while (biorder--)
			{
				nume = nume * 10 + _pow(5, i++);
				if (i > 19)	// ����longlong�ĳ��ȣ�
					break;
			}
			while (nume)
			{	// С�����ֶ�����תʮ����
				*(srev2++) = nume % 10;
				nume /= 10;
			}
			srev2--;
		}
		else
		{	// binumeֻ������; ������
			biorder -= 23;
			while (biorder--)
			{	// ��binume�������biorderת��Ϊ�µ�binume��ʮ����order
				binume <<= 1;
				if (binume >= 2147483648) // 2**31
				{
					order++;
					binume /= 10;
				}
			}
			inte = binume;
		}

		// ������ַ���s
		while (inte)
		{	// �������ֶ�����תʮ����
			*(srev++) = inte % 10;
			inte /= 10;
		}
		srev--;
		while (srev-- != rev)	// �������
			*(sp++) = *srev + '0';
		while (order--)		// �������ĩβ��0
			*(sp++) = '0';
		*(sp++) = '.';	// ���С����
		i = 6;
		if (srev2 == rev2)	// ���С��
		{
			while (i--)
				*(sp++) = '0';
		}
		else
		{
			while (i--)
				*(sp++) = *(srev2--);
		}
	}
	else
	{	// С��1
		biorder = 126 - biorder; // ������С�����ֿ�ͷ0�ĸ���
	}

}
