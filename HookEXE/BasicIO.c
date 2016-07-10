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

int BytePrefix(int sizelen)
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

// Get compiled year from MinGW-w64's __DATE__
int CompileYear()
{
	const char macro[16] = __DATE__;
	char stmp[8] = {0};

	stmp[0] = macro[7];
	stmp[1] = macro[8];
	stmp[2] = macro[9];
	stmp[3] = macro[10];
	stmp[4] = '\0';

	return atoi(stmp);
}

// Get compiled month from MinGW-w64's __DATE__
int CompileMonth()
{
	const char macro[16] = __DATE__;
	const char smonth[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	int i = 0;

	for (i = 0; i < 12; i++)
	{
		if (strstr(macro, smonth[i]) != NULL)
			return i + 1;
	}

	// return -1 for error
	return -1;
}

// Get compiled day from MinGW-w64's __DATE__
int CompileDay()
{
	const char macro[16] = __DATE__;
	char stmp[4] = {0};

	stmp[0] = macro[4];
	stmp[1] = macro[5];
	stmp[2] = '\0';
	return atoi(stmp);
}
