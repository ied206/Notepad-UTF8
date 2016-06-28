#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "BasicIO.h"

// Linux-Style Hex Dump
void BinaryDump(const uint8_t buf[], const uint32_t bufsize)
{
	uint32_t base = 0;
	uint32_t interval = 16;
	while (base < bufsize)
	{
		if (base + 16 < bufsize)
			interval = 16;
		else
			interval = bufsize - base;

		printf("0x%04x:   ", base);
		for (uint32_t i = base; i < base + 16; i++) // i for dump
		{
			if (i < base + interval)
				printf("%02x", buf[i]);
			else
			{
				putchar(' ');
				putchar(' ');
			}

			if ((i+1) % 2 == 0)
				putchar(' ');
			if ((i+1) % 8 == 0)
				putchar(' ');
		}
		putchar(' ');
		putchar(' ');
		for (uint32_t i = base; i < base + 16; i++) // i for dump
		{
			if (i < base + interval)
			{
				if (0x20 <= buf[i] && buf[i] <= 0x7E)
					printf("%c", buf[i]);
				else
					putchar('.');
			}
			else
			{
				putchar(' ');
				putchar(' ');
			}

			if ((i+1) % 8 == 0)
				putchar(' ');
		}
		putchar('\n');


		if (base + 16 < bufsize)
			base += 16;
		else
			base = bufsize;
	}

	return;
}

int WhatBitOS (int print) // 몇비트 컴파일인가 알아내기
{
    if (print)
    {
        if (sizeof(int*) == 4)
        {
            printf("This is 32bit Program!\n");
            return 32;
        }
        else if (sizeof(int*) == 8)
        {
            printf("This is 64bit Program!\n");
            return 64;
        }
        else
        {
			printf("This is %Iubit Program!\n", sizeof(int*) * 8);
            return (sizeof(int*) * 8);
        }
    }
    else
    {
        if (sizeof(int*) == 4)
            return 32;
        else if (sizeof(int*) == 8)
            return 64;
        else
            return (sizeof(int*) * 8);
    }
}

int BytePrefix (int sizelen)
{
	int whatbyte;

    if (sizelen < KILOBYTE) // Byte
		whatbyte = 0;
	else if (sizelen < MEGABYTE) // KB
		whatbyte = 1;
	else if (sizelen < GIGABYTE) // MB
		whatbyte = 2;
	else // GB
		whatbyte = 3;

	return whatbyte;
}

int CompileYear () // 컴파일한 년도
{
	const char macro[16] = __DATE__;
	char stmp[8] = {0}; // 전체 0으로 초기화

	stmp[0] = macro[7];
	stmp[1] = macro[8];
	stmp[2] = macro[9];
	stmp[3] = macro[10];
	stmp[4] = '\0';

	return atoi(stmp);
}

int CompileMonth ()
{ // 컴파일한 월 표시
	const char macro[16] = __DATE__;
	const char smonth[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	int i = 0;

// 월 감지
	for (i = 0; i < 12; i++)
	{
		if (strstr(macro, smonth[i]) != NULL)
			return i + 1;
	}

// -1이 반환되는 경우는 월 인식 불가
	return -1;
}

int CompileDate ()
{ // 컴파일한 일 표시
	const char macro[16] = __DATE__;
	char stmp[4] = {0}; // 전체 0으로 초기화

// 일 감지
	stmp[0] = macro[4];
	stmp[1] = macro[5];
	stmp[2] = '\0';
	return atoi(stmp);
}
