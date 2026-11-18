#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "gc_dns.h"

#include <network.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define GC_DNS_PORT        53
#define GC_DNS_TIMEOUT_MS  1000
#define GC_DNS_POLL_MS     10
#define GC_DNS_PACKET_MAX  512

static struct in_addr gc_dns_server;

void GC_DnsSetServer(const struct in_addr *server)
{
	if (server)
		gc_dns_server = *server;
	else
		memset(&gc_dns_server, 0, sizeof(gc_dns_server));
}

static u16 GC_DnsRead16(const byte *p)
{
	return (u16)(((u16)p[0] << 8) | p[1]);
}

static int GC_DnsBuildQuery(byte *buf, int max, u16 id, const char *name)
{
	int len = 12;
	int namelen = 0;
	const char *p = name;

	if (max < 17)
		return -1;

	memset(buf, 0, 12);
	buf[0] = (byte)(id >> 8);
	buf[1] = (byte)(id & 0xff);
	buf[2] = 0x01;
	buf[5] = 0x01;

	while (1) {
		const char *dot = strchr(p, '.');
		int n = dot ? (int)(dot - p) : (int)strlen(p);

		if (n <= 0 || n > 63)
			return -1;
		namelen += n + 1;
		if (namelen > 255 || len + n + 6 > max)
			return -1;

		buf[len++] = (byte)n;
		memcpy(buf + len, p, n);
		len += n;

		if (!dot)
			break;
		p = dot + 1;
		if (!*p)
			break;
	}

	buf[len++] = 0;
	buf[len++] = 0;
	buf[len++] = 1;
	buf[len++] = 0;
	buf[len++] = 1;
	return len;
}

static int GC_DnsSkipName(const byte *pkt, int len, int off)
{
	while (off < len) {
		byte c = pkt[off++];

		if (c == 0)
			return off;
		if ((c & 0xc0) == 0xc0)
			return (off < len) ? off + 1 : -1;
		if (c & 0xc0)
			return -1;

		off += c;
		if (off > len)
			return -1;
	}
	return -1;
}

static qboolean GC_DnsParse(const byte *pkt, int len, u16 id, struct in_addr *out)
{
	int off;
	int qdcount, ancount;
	int i;

	if (len < 12)
		return qfalse;
	if (GC_DnsRead16(pkt) != id)
		return qfalse;
	if (!(pkt[2] & 0x80) || (pkt[3] & 0x0f) != 0)
		return qfalse;

	qdcount = GC_DnsRead16(pkt + 4);
	ancount = GC_DnsRead16(pkt + 6);
	off = 12;

	for (i = 0; i < qdcount; i++) {
		off = GC_DnsSkipName(pkt, len, off);
		if (off < 0 || off + 4 > len)
			return qfalse;
		off += 4;
	}

	for (i = 0; i < ancount; i++) {
		int type, rrclass, rdlen;

		off = GC_DnsSkipName(pkt, len, off);
		if (off < 0 || off + 10 > len)
			return qfalse;

		type = GC_DnsRead16(pkt + off);
		rrclass = GC_DnsRead16(pkt + off + 2);
		rdlen = GC_DnsRead16(pkt + off + 8);
		off += 10;

		if (rdlen < 0 || off + rdlen > len)
			return qfalse;

		if (type == 1 && rrclass == 1 && rdlen == 4) {
			memcpy(&out->s_addr, pkt + off, 4);
			return qtrue;
		}
		off += rdlen;
	}

	return qfalse;
}

static qboolean GC_DnsWait(int s, u16 id, struct in_addr *out)
{
	byte reply[GC_DNS_PACKET_MAX];
	int waited;

	for (waited = 0; waited < GC_DNS_TIMEOUT_MS; waited += GC_DNS_POLL_MS) {
		struct sockaddr_in from;
		socklen_t fromlen = sizeof(from);
		int ret;

		ret = net_recvfrom(s, reply, sizeof(reply), 0,
		                   (struct sockaddr *)&from, &fromlen);
		if (ret > 0 && GC_DnsParse(reply, ret, id, out))
			return qtrue;
		if (ret < 0 && ret != -EAGAIN)
			return qfalse;

		usleep(GC_DNS_POLL_MS * 1000);
	}

	return qfalse;
}

qboolean GC_DnsResolve(const char *name, struct in_addr *out)
{
	struct in_addr servers[2];
	int nservers = 0;
	int i;
	int s;
	u16 id;
	int qlen;
	byte query[GC_DNS_PACKET_MAX];
	u32 nonblock = 1;

	if (!name || !*name || !out)
		return qfalse;

	id = (u16)(rand() & 0xffff);
	qlen = GC_DnsBuildQuery(query, sizeof(query), id, name);
	if (qlen < 0) {
		Com_Printf("NET: cannot resolve %s\n", name);
		return qfalse;
	}

	inet_aton("8.8.8.8", &servers[nservers]);
	nservers++;

	if (gc_dns_server.s_addr != 0 && gc_dns_server.s_addr != servers[0].s_addr) {
		servers[1] = servers[0];
		servers[0] = gc_dns_server;
		nservers = 2;
	}

	s = net_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s >= 0 && net_ioctl(s, FIONBIO, &nonblock) >= 0) {
		for (i = 0; i < nservers; i++) {
			struct sockaddr_in to;

			memset(&to, 0, sizeof(to));
			to.sin_len = (u8)sizeof(to);
			to.sin_family = AF_INET;
			to.sin_port = htons(GC_DNS_PORT);
			to.sin_addr = servers[i];

			if (net_sendto(s, query, qlen, 0, (struct sockaddr *)&to,
			               sizeof(to)) < 0)
				continue;

			if (GC_DnsWait(s, id, out)) {
				net_close(s);
				return qtrue;
			}
		}
	}
	if (s >= 0)
		net_close(s);

	Com_Printf("NET: cannot resolve %s\n", name);
	return qfalse;
}
