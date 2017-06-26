
#include "print.h"

static void printchar(char **str, uartMap_t uart, int c) {

    if (str) {
        **str = c;
        ++(*str);
    } else {

        switch(uart){
            case UART_USB:
                uartUsbSendByte(c);
                break;
            case UART_232:
                uart232SendByte(c);
                break;
        }

    }

}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, uartMap_t uart, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, uart, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, uart, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, uart, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, uartMap_t uart, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, uart, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, uart, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, uart, s, width, pad);
}

int print(char **out, uartMap_t uart, int *varg)
{
	register int width, pad;
	register int pc = 0;
	register char *format = (char *)(*varg++);
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = *((char **)varg++);
				pc += prints (out, uart, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, uart, *varg++, 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, uart, *varg++, 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, uart, *varg++, 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, uart, *varg++, 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = *varg++;
				scr[1] = '\0';
				pc += prints (out, uart, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			printchar (out, uart, *format);
			++pc;
		}
	}
    if (out) **out = '\0';
	return pc;
}

int debugFormat(const char *format, ...) {
	register int *varg = (int *)(&format);
	return print(NULL, UART_USB, varg);
}

int modemUartFormat(const char *format, ...) {
	register int *varg = (int *)(&format);
	return print(NULL, UART_232, varg);
}

int usbUartFormat(const char *format, ...) {
	register int *varg = (int *)(&format);
	return print(NULL, UART_USB, varg);
}

int sprintf(char *out, const char *format, ...)
{
    register int *varg = (int *)(&format);
    return print(&out, 0, varg);
}