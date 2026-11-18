#ifndef GC_DNS_H
#define GC_DNS_H

#include "../qcommon/q_shared.h"
#include <netinet/in.h>

void GC_DnsSetServer(const struct in_addr *server);
qboolean GC_DnsResolve(const char *name, struct in_addr *out);

#endif
