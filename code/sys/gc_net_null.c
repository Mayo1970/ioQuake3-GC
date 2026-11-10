#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

/*
====================
NET_Init
====================
*/
void NET_Init(void)
{
	Cvar_Get("net_enabled", "0", CVAR_LATCH | CVAR_ARCHIVE);
	Cmd_AddCommand("net_restart", NET_Restart_f);
}

/*
====================
NET_Shutdown
====================
*/
void NET_Shutdown(void)
{
}

void NET_Restart_f(void)
{
	NET_Config(qtrue);
}

void NET_Config(qboolean enableNetworking)
{
}

void NET_Sleep(int msec)
{
	if (msec > 0)
		Sys_Sleep(msec);
}

/*
=============
Sys_StringToAdr
=============
*/
qboolean Sys_StringToAdr(const char *s, netadr_t *a, netadrtype_t family)
{
	if (!Q_stricmp(s, "localhost")) {
		Com_Memset(a, 0, sizeof(*a));
		a->type = NA_LOOPBACK;
		return qtrue;
	}

	return qfalse;
}

/*
===================
NET_CompareBaseAdrMask

Compare without port, and up to the bit number given in netmask.
===================
*/
qboolean NET_CompareBaseAdrMask(netadr_t a, netadr_t b, int netmask)
{
	byte cmpmask, *addra, *addrb;
	int curbyte;

	if (a.type != b.type)
		return qfalse;

	if (a.type == NA_LOOPBACK)
		return qtrue;

	if (a.type == NA_IP) {
		addra = (byte *)&a.ip;
		addrb = (byte *)&b.ip;

		if (netmask < 0 || netmask > 32)
			netmask = 32;
	} else if (a.type == NA_IP6) {
		addra = (byte *)&a.ip6;
		addrb = (byte *)&b.ip6;

		if (netmask < 0 || netmask > 128)
			netmask = 128;
	} else {
		Com_Printf("NET_CompareBaseAdr: bad address type\n");
		return qfalse;
	}

	curbyte = netmask >> 3;

	if (curbyte && memcmp(addra, addrb, curbyte))
		return qfalse;

	netmask &= 0x07;
	if (netmask) {
		cmpmask = (1 << netmask) - 1;
		cmpmask <<= 8 - netmask;

		if ((addra[curbyte] & cmpmask) == (addrb[curbyte] & cmpmask))
			return qtrue;
	} else {
		return qtrue;
	}

	return qfalse;
}

/*
===================
NET_CompareBaseAdr

Compares without the port
===================
*/
qboolean NET_CompareBaseAdr(netadr_t a, netadr_t b)
{
	return NET_CompareBaseAdrMask(a, b, -1);
}

const char *NET_AdrToString(netadr_t a)
{
	static char s[NET_ADDRSTRMAXLEN];

	if (a.type == NA_LOOPBACK) {
		Com_sprintf(s, sizeof(s), "loopback");
	} else if (a.type == NA_BOT) {
		Com_sprintf(s, sizeof(s), "bot");
	} else if (a.type == NA_IP) {
		Com_sprintf(s, sizeof(s), "%u.%u.%u.%u", a.ip[0], a.ip[1], a.ip[2], a.ip[3]);
	} else {
		Com_sprintf(s, sizeof(s), "unknown");
	}

	return s;
}

const char *NET_AdrToStringwPort(netadr_t a)
{
	static char s[NET_ADDRSTRMAXLEN];

	if (a.type == NA_LOOPBACK)
		Com_sprintf(s, sizeof(s), "loopback");
	else if (a.type == NA_BOT)
		Com_sprintf(s, sizeof(s), "bot");
	else if (a.type == NA_IP)
		Com_sprintf(s, sizeof(s), "%s:%hu", NET_AdrToString(a), a.port);
	else
		Com_sprintf(s, sizeof(s), "unknown");

	return s;
}

qboolean NET_CompareAdr(netadr_t a, netadr_t b)
{
	if (!NET_CompareBaseAdr(a, b))
		return qfalse;

	if (a.type == NA_IP || a.type == NA_IP6) {
		if (a.port == b.port)
			return qtrue;
	} else {
		return qtrue;
	}

	return qfalse;
}

qboolean NET_IsLocalAddress(netadr_t adr)
{
	return adr.type == NA_LOOPBACK;
}

void Sys_SendPacket(int length, const void *data, netadr_t to)
{
}

/*
==================
Sys_IsLANAddress

LAN clients will have their rate var ignored
==================
*/
qboolean Sys_IsLANAddress(netadr_t adr)
{
	return adr.type == NA_LOOPBACK ? qtrue : qfalse;
}

/*
==================
Sys_ShowIP
==================
*/
void Sys_ShowIP(void)
{
}

void NET_JoinMulticast6(void)
{
}

void NET_LeaveMulticast6(void)
{
}
