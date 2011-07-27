#ifndef SCRIPT_EXECUTER_H
#define SCRIPT_EXECUTER_H

#ifdef __cplusplus
extern "C" {
#endif

int ScriptExec(const char * str);
#ifdef LINUX
void Executer(int msgID, pid_t pid, char * procName);
#else
void Executer(int msgID, pid_t pid);
#endif

#ifdef __cplusplus
}
#endif

#endif
