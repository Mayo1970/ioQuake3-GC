#ifndef GC_NET_H
#define GC_NET_H

#include <network.h>
#include <errno.h>
#include <string.h>

#undef INVALID_SOCKET
#undef SOCKET_ERROR

#ifdef __cplusplus
extern "C" {
#endif

static inline int GC_NetResult(int ret)
{
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

static inline int GC_NetSocket(int domain, int type, int protocol)
{
	return GC_NetResult(net_socket((u32)domain, (u32)type, (u32)protocol));
}

static inline int GC_NetBind(int s, const struct sockaddr *addr, int addrlen)
{
	struct sockaddr_in tmp;
	const struct sockaddr *a = addr;

	if (addr && addr->sa_family == AF_INET && addrlen >= (int)sizeof(tmp)) {
		memcpy(&tmp, addr, sizeof(tmp));
		tmp.sin_len = (u8)sizeof(tmp);
		a = (const struct sockaddr *)&tmp;
		addrlen = sizeof(tmp);
	}
	return GC_NetResult(net_bind(s, (struct sockaddr *)a, (socklen_t)addrlen));
}

static inline int GC_NetSendTo(int s, const void *buf, int len, int flags,
                               const struct sockaddr *addr, int addrlen)
{
	struct sockaddr_in tmp;
	const struct sockaddr *a = addr;

	if (addr && addr->sa_family == AF_INET && addrlen >= (int)sizeof(tmp)) {
		memcpy(&tmp, addr, sizeof(tmp));
		tmp.sin_len = (u8)sizeof(tmp);
		a = (const struct sockaddr *)&tmp;
		addrlen = sizeof(tmp);
	}
	return GC_NetResult(net_sendto(s, buf, len, (u32)flags,
	                               (struct sockaddr *)a, (socklen_t)addrlen));
}

static inline int GC_NetRecvFrom(int s, void *buf, int len, int flags,
                                 struct sockaddr *addr, socklen_t *addrlen)
{
	return GC_NetResult(net_recvfrom(s, buf, len, (u32)flags, addr, addrlen));
}

static inline int GC_NetSetSockOpt(int s, int level, int optname,
                                   const void *optval, int optlen)
{
	return GC_NetResult(net_setsockopt(s, (u32)level, (u32)optname,
	                                   optval, (socklen_t)optlen));
}

static inline int GC_NetGetSockOpt(int s, int level, int optname,
                                   void *optval, socklen_t *optlen)
{
	return GC_NetResult(net_getsockopt(s, (u32)level, (u32)optname,
	                                   optval, optlen));
}

static inline int GC_NetIoctl(int s, u32 cmd, void *argp)
{
	return GC_NetResult(net_ioctl(s, cmd, argp));
}

static inline int GC_NetClose(int s)
{
	return GC_NetResult(net_close(s));
}

static inline int GC_NetSelect(int n, fd_set *readset, fd_set *writeset,
                               fd_set *exceptset, struct timeval *timeout)
{
	return GC_NetResult(net_select(n, readset, writeset, exceptset, timeout));
}

#define socket(d,t,p)           GC_NetSocket((d),(t),(p))
#define bind(s,a,l)             GC_NetBind((s),(a),(l))
#define sendto(s,b,l,f,a,al)    GC_NetSendTo((s),(b),(l),(f),(a),(al))
#define recvfrom(s,b,l,f,a,al)  GC_NetRecvFrom((s),(b),(l),(f),(a),(al))
#define setsockopt(s,lv,o,v,l)  GC_NetSetSockOpt((s),(lv),(o),(v),(l))
#define getsockopt(s,lv,o,v,l)  GC_NetGetSockOpt((s),(lv),(o),(v),(l))
#define ioctlsocket(s,c,a)      GC_NetIoctl((s),(u32)(c),(a))
#define close(s)                GC_NetClose((s))
#define closesocket(s)          GC_NetClose((s))
#define select(n,r,w,e,t)       GC_NetSelect((n),(r),(w),(e),(t))

#ifdef __cplusplus
}
#endif

#endif
