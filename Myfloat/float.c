#include <stdio.h>
#include "float.h"
typedef unsigned long long qword;

/*
  dword atof(char *s)
  转化思路：
	1. 读取符号
	2. 读取整数部分，但最多读8位十进制，因为float类型的精度只有十进制的7.1位
	3. 如果整数不足8位，则继续读取小数；否则继续读取剩余整数，只计算阶，忽略小数
	4. 计算整数的二进制长度（包括阶在内）
	5. 计算整数的二进制长度（不含阶）
	6. 对符号、二进制阶、整数部分、小数部分进行组合
		1) 二进制阶==0
		2) 二进制阶<=24
		3) 二进制阶>=25
  存在的问题：
    当整数较大时会有误差
	小于0.00000001的数会有问题
*/
dword atof(char *s)
{
	dword i, j, cnt = 1, tmp;
	dword sign = 0, order, inte = 0, nume = 0, biorder = 0, binume = 0, \
		intebiorder = 0,exbinume = 0;
	dword flt = 0;

	while (*s == ' ') 
		s++;
	// ------------------------------ 设定小数的符号 ----------------------------------
	if (*s == '-')
	{
		sign = 0x80000000;
		s++;
	}
	else if (*s == '+') 
		s++;
	while (*s == '0')
		s++;

	// -------------------------------- 读取前8位整数 ---------------------------------- 
	order = 0; // 最多保留8位整数
	while (*s != '.' && *s != '\0' && order < 8)
	{
		inte = inte * 10 + *s - '0';
		s++;
		order++;
	}

	// --------------------------------- 读取小数部分 ----------------------------------
	if (*s == '.')
	{	
		s++;
		i = order; // 整数和小数总共最多保留8位
		while (*s != '\0' && i < 8)
		{
			nume = nume * 10 + *s - '0';
			s++;
			cnt *= 10;
			i++;
		}
		// 小数部分转成二进制
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
	else // -------------------------- 继续读取余下的整数 ------------------------------
	{	
		while (*s != '\0' && *s != '.')
		{
			order++;
			s++;
		}
	}

	// -------------------------- 计算整数部分的二进制长度biorder -----------------------
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

	// -------------------------- 计算inte中整数的位数intebiorder -----------------------
	tmp = inte;
	while (tmp)
	{	
		intebiorder++;
		tmp >>= 1;
	}

	// ---------------------- 各部分(sign,biorder,inte,binume)开始组合 ------------------
	if (intebiorder == 0)
	{	// 整数部分为0
		tmp = binume;
		while (!(tmp & 0x80000000)) // 去掉小数部分开头的0 
		{
			biorder++;
			tmp <<= 1;
		}
		if (biorder <= 8)
		{
			inte = (binume >> (8 - biorder)) & 0xFF7FFFFF; // 去掉首位的1
			if (((binume >> (7 - biorder)) & 1) == 1)
				inte++;
		}
		else
			inte = ((binume << (biorder - 8)) | (exbinume >> (40 - biorder))) & 0xFF7FFFFF;
		flt = sign | ((127 - 1 - biorder) << 23) | inte;
	}
	else if (intebiorder <= 24)
	{	// inte不足25位，用小数补齐
		inte = (inte << (24 - intebiorder)) & 0xFF7FFFFF; // 去掉首位的1
		inte |= (binume >> (8 + intebiorder));
		if (((binume >> (7 + intebiorder)) & 1) == 1)
			inte++;
		flt = sign | ((biorder + 127 - 1) << 23) | inte;
	}
	else
	{	// inte足够25位，则无须小数部分，取中间前23位即可(首位的1要去掉)
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
	binume = (f & 0x007FFFFF) | 0x00800000; // 高位补1

	if (!sign)	// 输出符号
		*(sp++) = '-';
	
	// 计算整数部分inte、小数部分nume、十进制阶order
	if (biorder >= 127)
	{	// 大于1
		biorder -= 127;
		if (biorder < 23)
		{	// binume同时有整数和小数
			inte = (binume >> (23 - biorder));
			tmp = (0xFFFFFFFF >> (9 + biorder));
			binume &= tmp;
			// 小数二进制转十进制
			biorder = 23 - biorder;
			i = 1;
			while (biorder--)
			{
				nume = nume * 10 + _pow(5, i++);
				if (i > 19)	// 超过longlong的长度！
					break;
			}
			while (nume)
			{	// 小数部分二进制转十进制
				*(srev2++) = nume % 10;
				nume /= 10;
			}
			srev2--;
		}
		else
		{	// binume只有整数; 大整数
			biorder -= 23;
			while (biorder--)
			{	// 将binume与二进制biorder转化为新的binume和十进制order
				binume <<= 1;
				if (binume >= 2147483648) // 2**31
				{
					order++;
					binume /= 10;
				}
			}
			inte = binume;
		}

		// 输出到字符串s
		while (inte)
		{	// 整数部分二进制转十进制
			*(srev++) = inte % 10;
			inte /= 10;
		}
		srev--;
		while (srev-- != rev)	// 输出整数
			*(sp++) = *srev + '0';
		while (order--)		// 输出整数末尾的0
			*(sp++) = '0';
		*(sp++) = '.';	// 输出小数点
		i = 6;
		if (srev2 == rev2)	// 输出小数
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
	{	// 小于1
		biorder = 126 - biorder; // 二进制小数部分开头0的个数
	}

}
