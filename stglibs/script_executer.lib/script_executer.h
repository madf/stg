#ifndef SCRIPT_EXECUTER_H
#define SCRIPT_EXECUTER_H

#include <string>
#include <sys/types.h>

int ScriptExec(const std::string & str);
void Executer(int msgKey, int msgID, pid_t pid, char * procName);

#endif //SCRIPT_EXECUTER_H


