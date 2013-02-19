#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<sys/wait.h>

#include   "Common.h"
void * Calloc(size_t n, size_t size)
{
	void	*ptr;

	if ( (ptr = calloc(n, size)) == NULL)
		DieWithSystemMessage("calloc error");
	return(ptr);
}

void Close(int fd)
{
	if (close(fd) == -1)
		DieWithSystemMessage("close error");
}

void Dup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) == -1)
		DieWithSystemMessage("dup2 error");
}

pid_t Fork(void)
{
	pid_t	pid;

	if ( (pid = fork()) == -1)
		DieWithSystemMessage("fork error");
	return(pid);
}

void * Malloc(size_t size)
{
	void	*ptr;

	if ( (ptr = malloc(size)) == NULL)
		DieWithSystemMessage("malloc error");
	return(ptr);
}

#include	<sys/mman.h>

void Pipe(int *fds)
{
	if (pipe(fds) < 0)
		DieWithSystemMessage("pipe error");
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1)
		DieWithSystemMessage("read error");
	return(n);
}


char * Strdup(const char *str)
{
	char	*ptr;

	if ( (ptr = strdup(str)) == NULL)
		DieWithSystemMessage("strdup error");
	return(ptr);
}


pid_t Wait(int *iptr)
{
	pid_t	pid;

	if ( (pid = wait(iptr)) == -1)
		DieWithSystemMessage("wait error");
	return(pid);
}

pid_t Waitpid(pid_t pid, int *iptr, int options)
{
	pid_t	retpid;

	if ( (retpid = waitpid(pid, iptr, options)) == -1)
		DieWithSystemMessage("waitpid error");
	return(retpid);
}

void Write(int fd, void *ptr, size_t nbytes)
{
	if (write(fd, ptr, nbytes) != nbytes)
		DieWithSystemMessage("write error");
}
