void * Calloc(size_t , size_t );
void Close(int );
void Dup2(int , int );
pid_t Fork(void);
void * Malloc(size_t );
void Pipe(int *);
ssize_t Read(int , void *, size_t );
char * Strdup(const char *)
pid_t Wait(int *);
pid_t Waitpid(pid_t , int *, int );
void Write(int , void *, size_t );
