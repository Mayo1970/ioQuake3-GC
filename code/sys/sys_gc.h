#ifndef __SYS_GC_H
#define __SYS_GC_H

void GC_InitConsole(void);
qboolean GC_InitStorage(int argc, char **argv);
void GC_WaitForStorage(int argc, char **argv);

#endif
