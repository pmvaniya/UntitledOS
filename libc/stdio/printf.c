#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_LENGTH_SHORT 2
#define PRINTF_STATE_LENGTH_LONG 3
#define PRINTF_STATE_SPEC 4

#define PRINTF_LENGTH_DEFAULT 0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4

static bool print(const char *data, size_t length)
{
	const unsigned char *bytes = (const unsigned char *)data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

void puts_f(const char *str)
{
	while (*str)
	{
		print(str, 1);
		str++;
	}
}

const char g_HexChars[] = "0123456789abcdef";

int *printf_number(int *argp, int length, bool sign, int radix)
{
	char buffer[32];
	unsigned long long number = 0;
	int number_sign = 1;
	int pos = 0;

	// process length
	switch (length)
	{
	case PRINTF_LENGTH_SHORT_SHORT:
	case PRINTF_LENGTH_SHORT:
	case PRINTF_LENGTH_DEFAULT:
		if (sign)
		{
			int n = *argp;
			if (n < 0)
			{
				n = -n;
				number_sign = -1;
			}
			number = (unsigned long long)n;
		}
		else
		{
			number = *(unsigned int *)argp;
		}
		argp++;
		break;

	case PRINTF_LENGTH_LONG:
		if (sign)
		{
			long int n = *(long int *)argp;
			if (n < 0)
			{
				n = -n;
				number_sign = -1;
			}
			number = (unsigned long long)n;
		}
		else
		{
			number = *(unsigned long int *)argp;
		}
		argp += 2;
		break;

	case PRINTF_LENGTH_LONG_LONG:
		if (sign)
		{
			long long int n = *(long long int *)argp;
			if (n < 0)
			{
				n = -n;
				number_sign = -1;
			}
			number = (unsigned long long)n;
		}
		else
		{
			number = *(unsigned long long int *)argp;
		}
		argp += 4;
		break;
	}

	char str[2];

	// convert number to ASCII
	do
	{
		unsigned long int rem = number % radix;
		buffer[pos++] = g_HexChars[rem];
		number /= radix;
	} while (number > 0);

	// add sign
	if (sign && number_sign < 0)
		buffer[pos++] = '-';

	// print number in reverse order
	while (--pos >= 0)
	{
		str[0] = buffer[pos];
		str[1] = '\0';
		print(str, 1);
	}

	return argp;
}

int printf(const char *restrict fmt, ...)
{
	int *argp = (int *)&fmt;
	int state = PRINTF_STATE_NORMAL;
	int length = PRINTF_LENGTH_DEFAULT;
	int radix = 10;
	bool sign = false;

	argp++;

	while (*fmt)
	{
		switch (state)
		{
		case PRINTF_STATE_NORMAL:
			switch (*fmt)
			{
			case '%':
				state = PRINTF_STATE_LENGTH;
				break;
			default:
				print(fmt, 1);
				break;
			}
			break;

		case PRINTF_STATE_LENGTH:
			switch (*fmt)
			{
			case 'h':
				length = PRINTF_LENGTH_SHORT;
				state = PRINTF_STATE_LENGTH_SHORT;
				break;
			case 'l':
				length = PRINTF_LENGTH_LONG;
				state = PRINTF_STATE_LENGTH_LONG;
				break;
			default:
				goto PRINTF_STATE_SPEC_;
			}
			break;

		case PRINTF_STATE_LENGTH_SHORT:
			if (*fmt == 'h')
			{
				length = PRINTF_LENGTH_SHORT_SHORT;
				state = PRINTF_STATE_SPEC;
			}
			else
				goto PRINTF_STATE_SPEC_;
			break;

		case PRINTF_STATE_LENGTH_LONG:
			if (*fmt == 'l')
			{
				length = PRINTF_LENGTH_LONG_LONG;
				state = PRINTF_STATE_SPEC;
			}
			else
				goto PRINTF_STATE_SPEC_;
			break;

		case PRINTF_STATE_SPEC:
		PRINTF_STATE_SPEC_:
			switch (*fmt)
			{
			case 'c':
				print((char *)argp, 1);
				argp++;
				break;

			case 's':
				if (length == PRINTF_LENGTH_LONG || length == PRINTF_LENGTH_LONG_LONG)
				{
					puts_f(*(const char **)argp);
					argp += 2;
				}
				else
				{
					puts(*(const char **)argp);
					argp++;
				}
				break;

			case '%':
				print("%%", 1);
				break;

			case 'd':
			case 'i':
				radix = 10;
				sign = true;
				argp = printf_number(argp, length, sign, radix);
				break;

			case 'u':
				radix = 10;
				sign = false;
				argp = printf_number(argp, length, sign, radix);
				break;

			case 'X':
			case 'x':
			case 'p':
				radix = 16;
				sign = false;
				argp = printf_number(argp, length, sign, radix);
				break;

			case 'o':
				radix = 8;
				sign = false;
				argp = printf_number(argp, length, sign, radix);
				break;

			// ignore invalid spec
			default:
				break;
			}

			// reset state
			state = PRINTF_STATE_NORMAL;
			length = PRINTF_LENGTH_DEFAULT;
			radix = 10;
			sign = false;
			break;
		}

		fmt++;
	}

	return 0;
}
