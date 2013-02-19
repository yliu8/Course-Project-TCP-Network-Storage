/*
 * Standard I/O wrapper functions.
 */
#include	<stdio.h>
#include	"Common.h"

void Fclose(FILE *fp)
{
	if (fclose(fp) != 0)
		DieWithSystemMessage("fclose error");
}

FILE * Fdopen(int fd, const char *type)
{
	FILE	*fp;

	if ( (fp = fdopen(fd, type)) == NULL)
		DieWithSystemMessage("fdopen error");

	return(fp);
}

char * Fgets(char *ptr, int n, FILE *stream)
{
	char	*rptr;

	if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
		DieWithSystemMessage("fgets error");

	return (rptr);
}

FILE * Fopen(const char *filename, const char *mode)
{
	FILE	*fp;

	if ( (fp = fopen(filename, mode)) == NULL)
		DieWithSystemMessage("fopen error");

	return(fp);
}

void Fputs(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF)
		DieWithSystemMessage("fputs error");
}
