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
		if (dest[i] > '9')	// 判断进位，十分位一定不会进位
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
dword Jatof(char *s)
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
  char *ftoa(dword f)
  转化思路：
	1. 以dword读入小数
	2. 拆分为符号sign、二进制阶biorder、二进制小数binume
	3. 如果阶>=127，则小数点向后移；如果阶<127，则小数点向左移
	4. 整数部分直接转化为字符串；小数部分利用numerical字符串数组进行计算
	5. 最后以字符串输出符号、整数、小数点、小数
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
	sign = (f >> 31);						// 符号位
	biorder = ((f & 0x7FFFFFFF) >> 23);		// 二进制阶
	binume = (f & 0x007FFFFF) | 0x00800000; // 二进制小数，高位补1

	if (sign)	// 输出符号
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
			for (i = 0; i < biorder; i++)
			{
				if (binume & 1)
					stradd(rev2, Jnumerical[biorder - i]);
				binume >>= 1;
			}
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
		// 整数部分二进制转十进制
		while (inte)
		{
			*(srev++) = inte % 10 + '0';
			inte /= 10;
		}
		while (srev-- != rev)		// 输出整数
			*(sp++) = *srev;
		while (order--)				// 输出整数末尾的0
			*(sp++) = '0';
		*(sp++) = '.';				// 输出小数点
		for (i = 0; i < 6; i++)		// 输出小数
			*(sp++) = rev2[i];
	}
	else
	{	// 小于1
		biorder = 126 - biorder; // 二进制小数部分开头0的个数
		*(sp++) = '0';
		*(sp++) = '.';
		mark = 0x00800000;
		for (i = biorder + 1; i < NUMEWIDTH; i++)
		{
			if (binume & mark)
				stradd(rev2, Jnumerical[i]);
			mark >>= 1;
		}
		for (i = 0; i < 6; i++)		// 输出小数
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

	// 对阶
	diforder = biorder1 - biorder2; 
	if (diforder > 40) // 阶数相差太大则同时binume1左移和binume2右移
	{
		binume2 >>= (diforder - 40);
		diforder = 40;
	}
	binume1 <<= diforder;

	// 同号相加，异号相减
	if (sign1 ^ sign2)
		binume1 -= binume2;
	else
		binume1 += binume2;

	// 调整阶
	tmp = binume1;
	i = 0;
	while (tmp)
	{	// 获取相加结果的位数
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

	if (!f1 || !f2)	// +0 或 -0
		return 0 | (((f1 >> 31) ^ (f2 >> 31)) ? 0x80000000 : 0);

	sign1 = f1 >> 31;
	sign2 = f2 >> 31;
	biorder1 = ((f1 & 0x7FFFFFFF) >> 23);
	biorder2 = ((f2 & 0x7FFFFFFF) >> 23);
	binume1 = (qword)((f1 & 0x007FFFFF) | 0x00800000);
	binume2 = (qword)((f2 & 0x007FFFFF) | 0x00800000);

	// 符号
	if (sign1 ^ sign2)
		sign1 = 1;
	else
		sign1 = 0;

	// 数
	binume1 *= binume2;	// 如果最高位不进位，则乘积为47位
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

	// 阶
	biorder1 = (dword)((byte)biorder1 + (byte)biorder2 - (byte)127);
	biorder1 += (i - 47);	// 加上小数乘法的进位

	return (sign1 << 31) | (biorder1 << 23) | ((dword)binume1 & 0x007FFFFF);
}

dword Jfdiv(dword f1, dword f2)
{
	dword sign1, sign2, biorder1, biorder2;
	qword binume1, binume2, tmp;
	dword i, round_flag = 0;

	if (!f2 && !(f1 & 0x80000000)) // 除数为0，被除数为正数
		return POSINF;
	if (!f2 && (f1 & 0x80000000)) // 除数为0，被除数为负数
		return NEGINF;
	if (!f1) // +0 或 -0
		return 0 | ((f2 >> 31) ? 0x80000000 : 0);

	sign1 = f1 >> 31;
	sign2 = f2 >> 31;
	biorder1 = ((f1 & 0x7FFFFFFF) >> 23);
	biorder2 = ((f2 & 0x7FFFFFFF) >> 23);
	binume1 = (qword)((f1 & 0x007FFFFF) | 0x00800000);
	binume2 = (qword)((f2 & 0x007FFFFF) | 0x00800000);

	// 符号
	if (sign1 ^ sign2)
		sign1 = 1;
	else
		sign1 = 0;

	// 阶
	biorder1 = (dword)((byte)biorder1 - (byte)biorder2 + (byte)127);
	if (binume1 < binume2) // 1.xxxx < 1.yyyyy 商的阶为-1，否则为0
		biorder1 -= 1;

	// 数
	binume1 <<= 40; // 放大
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
