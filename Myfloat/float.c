#include <stdio.h>
#include <string.h>
#include "float.h"
#pragma warning(disable:4996)
extern char Jnumerical[NUMEWIDTH][NUMEWIDTH];

void stradd(char *dest, char * source)
{
	dword i, flag = 0, n = strlen(dest);
	for (i = n - 1; i < n; i--)
	{
		dest[i] += (source[i] - '0' + flag);
		if (dest[i] > '9')	// �жϽ�λ��ʮ��λһ�������λ
		{
			dest[i] -= 10;
			flag = 1;
		}
		else
			flag = 0;
	}
}

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
dword Jatof(char *s)
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
  char *ftoa(dword f)
  ת��˼·��
	1. ��dword����С��
	2. ���Ϊ����sign�������ƽ�biorder��������С��binume
	3. �����>=127����С��������ƣ������<127����С����������
	4. ��������ֱ��ת��Ϊ�ַ�����С����������numerical�ַ���������м���
	5. ������ַ���������š�������С���㡢С��
*/
char *Jftoa(dword f)
{
	static char s[50];
	char *sp;
	char rev[11], *srev;
	char rev2[NUMEWIDTH];
	dword i, mark;
	dword sign, biorder, binume;
	dword tmp, inte = 0, order = 0;
	qword nume = 0;

	sp = s;
	srev = rev;
	strcpy(rev2, Jnumerical[0]);
	sign = (f >> 31);						// ����λ
	biorder = ((f & 0x7FFFFFFF) >> 23);		// �����ƽ�
	binume = (f & 0x007FFFFF) | 0x00800000; // ������С������λ��1

	if (sign)	// �������
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
			for (i = 0; i < biorder; i++)
			{
				if (binume & 1)
					stradd(rev2, Jnumerical[biorder - i]);
				binume >>= 1;
			}
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
		// �������ֶ�����תʮ����
		while (inte)
		{
			*(srev++) = inte % 10 + '0';
			inte /= 10;
		}
		while (srev-- != rev)		// �������
			*(sp++) = *srev;
		while (order--)				// �������ĩβ��0
			*(sp++) = '0';
		*(sp++) = '.';				// ���С����
		for (i = 0; i < 6; i++)		// ���С��
			*(sp++) = rev2[i];
	}
	else
	{	// С��1
		biorder = 126 - biorder; // ������С�����ֿ�ͷ0�ĸ���
		*(sp++) = '0';
		*(sp++) = '.';
		mark = 0x00800000;
		for (i = biorder + 1; i < NUMEWIDTH; i++)
		{
			if (binume & mark)
				stradd(rev2, Jnumerical[i]);
			mark >>= 1;
		}
		for (i = 0; i < 6; i++)		// ���С��
			*(sp++) = rev2[i];
	}
	*sp = '\0';
	return s;
}

dword Jfadd(dword f1, dword f2)
{
	dword sign1, sign2, biorder1, biorder2;
	qword binume1, binume2, tmp;
	dword round_flag = 0, diforder, i;

	if (((f1 & 0x7FFFFFFF) >> 23) < ((f2 & 0x7FFFFFFF) >> 23))
		f1 ^= f2 ^= f1 ^= f2;

	if (!(f2 & 0x7FFFFFFF))
		return f1;

	sign1 = f1 >> 31;
	sign2 = f2 >> 31;
	biorder1 = ((f1 & 0x7FFFFFFF) >> 23);
	biorder2 = ((f2 & 0x7FFFFFFF) >> 23);
	binume1 = (qword)((f1 & 0x007FFFFF) | 0x00800000);
	binume2 = (qword)((f2 & 0x007FFFFF) | 0x00800000);

	// �Խ�
	diforder = biorder1 - biorder2; 
	if (diforder > 40) // �������̫����ͬʱbinume1���ƺ�binume2����
	{
		binume2 >>= (diforder - 40);
		diforder = 40;
	}
	binume1 <<= diforder;

	// ͬ����ӣ�������
	if (sign1 ^ sign2)
		binume1 -= binume2;
	else
		binume1 += binume2;

	// ������
	tmp = binume1;
	i = 0;
	while (tmp)
	{	// ��ȡ��ӽ����λ��
		tmp >>= 1;
		i++;
	}
	if (i >= 25)
	{
		binume1 >>= (i - 25);
		if (binume1 & 1)
			round_flag = 1;
		else
			round_flag = 0;
		binume1 >>= 1;
		binume1 += round_flag;
		biorder1 += (i - 24 - diforder);
	}
	else if (i >= 1)
	{
		binume1 <<= (24 - i);
		biorder1 -= (24 + diforder - i);
	}
	else // 0
	{
		return 0;
	}
	return (sign1 << 31) | (biorder1 << 23) | ((dword)binume1 & 0x007FFFFF);
}

dword Jfsub(dword f1, dword f2)
{
	return Jfadd(f1, f2 ^ 0x80000000);
}

dword Jfmul(dword f1, dword f2)
{
	dword sign1, sign2, biorder1, biorder2;
	qword binume1, binume2, tmp;
	dword i, round_flag = 0;

	if (!f1 || !f2)	// +0 �� -0
		return 0 | (((f1 >> 31) ^ (f2 >> 31)) ? 0x80000000 : 0);

	sign1 = f1 >> 31;
	sign2 = f2 >> 31;
	biorder1 = ((f1 & 0x7FFFFFFF) >> 23);
	biorder2 = ((f2 & 0x7FFFFFFF) >> 23);
	binume1 = (qword)((f1 & 0x007FFFFF) | 0x00800000);
	binume2 = (qword)((f2 & 0x007FFFFF) | 0x00800000);

	// ����
	if (sign1 ^ sign2)
		sign1 = 1;
	else
		sign1 = 0;

	// ��
	binume1 *= binume2;	// ������λ����λ����˻�Ϊ47λ
	tmp = binume1;
	i = 0;
	while (tmp)
	{
		tmp >>= 1;
		i++;
	}
	binume1 >>= (i - 25);
	if (binume1 & 1)
		round_flag = 1;
	binume1 >>= 1;
	binume1 += round_flag;

	// ��
	biorder1 = (dword)((byte)biorder1 + (byte)biorder2 - (byte)127);
	biorder1 += (i - 47);	// ����С���˷��Ľ�λ

	return (sign1 << 31) | (biorder1 << 23) | ((dword)binume1 & 0x007FFFFF);
}

dword Jfdiv(dword f1, dword f2)
{
	dword sign1, sign2, biorder1, biorder2;
	qword binume1, binume2, tmp;
	dword i, round_flag = 0;

	if (!f2 && !(f1 & 0x80000000)) // ����Ϊ0��������Ϊ����
		return POSINF;
	if (!f2 && (f1 & 0x80000000)) // ����Ϊ0��������Ϊ����
		return NEGINF;
	if (!f1) // +0 �� -0
		return 0 | ((f2 >> 31) ? 0x80000000 : 0);

	sign1 = f1 >> 31;
	sign2 = f2 >> 31;
	biorder1 = ((f1 & 0x7FFFFFFF) >> 23);
	biorder2 = ((f2 & 0x7FFFFFFF) >> 23);
	binume1 = (qword)((f1 & 0x007FFFFF) | 0x00800000);
	binume2 = (qword)((f2 & 0x007FFFFF) | 0x00800000);

	// ����
	if (sign1 ^ sign2)
		sign1 = 1;
	else
		sign1 = 0;

	// ��
	biorder1 = (dword)((byte)biorder1 - (byte)biorder2 + (byte)127);
	if (binume1 < binume2) // 1.xxxx < 1.yyyyy �̵Ľ�Ϊ-1������Ϊ0
		biorder1 -= 1;

	// ��
	binume1 <<= 40; // �Ŵ�
	binume1 /= binume2;
	tmp = binume1;
	i = 0;
	while (tmp)
	{
		tmp >>= 1;
		i++;
	}
	binume1 >>= (i - 25);
	if (binume1 & 1)
		round_flag = 1;
	binume1 >>= 1;
	binume1 += round_flag;

	return (sign1 << 31) | (biorder1 << 23) | ((dword)binume1 & 0x007FFFFF);
}






char Jnumerical[NUMEWIDTH][NUMEWIDTH] = {
	"0000000000000000000000000000000000000000000000000",
	"5000000000000000000000000000000000000000000000000",
	"2500000000000000000000000000000000000000000000000",
	"1250000000000000000000000000000000000000000000000",
	"0625000000000000000000000000000000000000000000000",
	"0312500000000000000000000000000000000000000000000",
	"0156250000000000000000000000000000000000000000000",
	"0078125000000000000000000000000000000000000000000",
	"0039062500000000000000000000000000000000000000000",
	"0019531250000000000000000000000000000000000000000",
	"0009765625000000000000000000000000000000000000000",
	"0004882812500000000000000000000000000000000000000",
	"0002441406250000000000000000000000000000000000000",
	"0001220703125000000000000000000000000000000000000",
	"0000610351562500000000000000000000000000000000000",
	"0000305175781250000000000000000000000000000000000",
	"0000152587890625000000000000000000000000000000000",
	"0000076293945312500000000000000000000000000000000",
	"0000038146972656250000000000000000000000000000000",
	"0000019073486328125000000000000000000000000000000",
	"0000009536743164062500000000000000000000000000000",
	"0000004768371582031250000000000000000000000000000",
	"0000002384185791015625000000000000000000000000000",
	"0000001192092895507812500000000000000000000000000",
	"0000000596046447753906250000000000000000000000000",
	"0000000298023223876953125000000000000000000000000",
	"0000000149011611938476562500000000000000000000000",
	"0000000074505805969238281250000000000000000000000",
	"0000000037252902984619140625000000000000000000000",
	"0000000018626451492309570312500000000000000000000",
	"0000000009313225746154785156250000000000000000000",
	"0000000004656612873077392578125000000000000000000",
	"0000000002328306436538696289062500000000000000000",
	"0000000001164153218269348144531250000000000000000",
	"0000000000582076609134674072265625000000000000000",
	"0000000000291038304567337036132812500000000000000",
	"0000000000145519152283668518066406250000000000000",
	"0000000000072759576141834259033203125000000000000",
	"0000000000036379788070917129516601562500000000000",
	"0000000000018189894035458564758300781250000000000",
	"0000000000009094947017729282379150390625000000000",
	"0000000000004547473508864641189575195312500000000",
	"0000000000002273736754432320594787597656250000000",
	"0000000000001136868377216160297393798828125000000",
	"0000000000000568434188608080148696899414062500000",
	"0000000000000284217094304040074348449707031250000",
	"0000000000000142108547152020037174224853515625000",
	"0000000000000071054273576010018587112426757812500",
	"0000000000000035527136788005009293556213378906250",
	"0000000000000017763568394002504646778106689453125"
};
