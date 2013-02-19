#define MAXLEN_CMD 100

void DieWithUserMessage(const char *, const char *);
void DieWithSystemMessage(const char *);
char *StringToUpper(char *);
char *StringToLower(char *);
char *trimLeft(char *);
char *trimRight(char *);
char *ReturnFirstString(char *);
char *ModifyString(char *);
char *CheckLoginCommandAndReturnName(char *);
int CommandCheckAndReturncmdCode(char *);
int GetParaNumOfCmd(char *);
char *GetCmdParaStringWithcmdCode(int  ,char *);
char *GetCmdParaString(char *);