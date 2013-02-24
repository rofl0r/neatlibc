#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

static int ic(FILE *fp)
{
	int nr;
	if (fp->back != EOF) {
		int i = fp->back;
		fp->back = EOF;
		return i;
	}
	while (fp->fd >= 0 && fp->icur == fp->ilen) {
		int nr = read(fp->fd, fp->ibuf, fp->isize);
		if (nr <= 0)
			break;
		fp->ilen = nr;
		fp->icur = 0;
	}
	return fp->icur < fp->ilen ? (unsigned char) fp->ibuf[fp->icur++] : EOF;
}

void setbuf(FILE *fp, char *buf)
{
}

int getc(FILE *fp)
{
	return ic(fp);
}

int getchar(void)
{
	return ic(stdin);
}

int ungetc(int c, FILE *fp)
{
	if (fp->back == EOF)
		fp->back = c;
	return fp->back;
}

static int iint(FILE *fp, void *dst, int l)
{
	long n = 0;
	int c;
	int neg = 0;
	c = ic(fp);
	if (c == '-') {
		neg = 1;
		c = ic(fp);
	}
	if (!isdigit(c)) {
		ungetc(c, fp);
		return 1;
	}
	do {
		n = n * 10 + c - '0';
	} while (isdigit(c = ic(fp)));
	ungetc(c, fp);
	if (l)
		*(long *) dst = neg ? -n : n;
	else
		*(int *) dst = neg ? -n : n;
	return 0;
}

static int istr(FILE *fp, char *dst)
{
	char *d = dst;
	int c;
	while ((c = ic(fp)) != EOF && !isspace(c))
		*d++ = c;
	*d = '\0';
	ungetc(c, fp);
	return d == dst;
}

int vfscanf(FILE *fp, char *fmt, va_list ap)
{
	int ret = 0;
	int l = 0;
	int c;
	char *s;
	while (*fmt) {
		while (isspace(*fmt))
			fmt++;
		while (isspace(c = ic(fp)))
			;
		ungetc(c, fp);
		while (*fmt && *fmt != '%' && !isspace(*fmt))
			if (*fmt++ != ic(fp))
				return ret;
		if (*fmt != '%')
			continue;
		fmt++;
		if (*fmt == 'l') {
			l = 1;
			fmt++;
		}
		switch (*fmt++) {
		case 'u':
		case 'd':
			if (iint(fp, va_arg(ap, long *), l))
				return ret;
			ret++;
			break;
		case 's':
			if (istr(fp, va_arg(ap, char *)))
				return ret;
			ret++;
			break;
		}
	}
	return ret;
}

int fscanf(FILE *fp, char *fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vfscanf(fp, fmt, ap);
	va_end(ap);
	return ret;
}

int scanf(char *fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vfscanf(stdin, fmt, ap);
	va_end(ap);
	return ret;
}

int vsscanf(char *s, char *fmt, va_list ap)
{
	FILE f = {-1, EOF};
	f.ibuf = s;
	f.ilen = strlen(s);
	return vfscanf(&f, fmt, ap);
}

int sscanf(char *s, char *fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vsscanf(s, fmt, ap);
	va_end(ap);
	return ret;
}