#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "hooks_net.h"

#include "../include/hybris/hook.h"

struct android_sockaddr {
    unsigned short sa_family;
    char sa_data[14];
};

struct android_addrinfo {
    int	ai_flags;	/* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
    int	ai_family;	/* PF_xxx */
    int	ai_socktype;	/* SOCK_xxx */
    int	ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
    socklen_t ai_addrlen;	/* length of ai_addr */
    char	*ai_canonname;	/* canonical name for hostname */
    struct	android_sockaddr *ai_addr;	/* binary address */
    struct	android_addrinfo *ai_next;	/* next structure in linked list */
};

static int darwin_convert_sockopt_socket_option(int opt)
{
    if (opt == 1) return SO_DEBUG;
    if (opt == 2) return SO_REUSEADDR;
    if (opt == 3) return SO_TYPE;
    if (opt == 4) return SO_ERROR;
    if (opt == 5) return SO_DONTROUTE;
    if (opt == 6) return SO_BROADCAST;
    if (opt == 7) return SO_SNDBUF;
    if (opt == 8) return SO_RCVBUF;
    //if (opt == 32) return SO_SNDBUFFORCE;
    //if (opt == 33) return SO_RCVBUFFORCE;
    if (opt == 9) return SO_KEEPALIVE;
    if (opt == 10) return SO_OOBINLINE;
    //if (opt == 11) return SO_NO_CHECK;
    //if (opt == 12) return SO_PRIORITY;
    if (opt == 13) return SO_LINGER;
    //if (opt == 14) return SO_BSDCOMPAT;
    if (opt == 15) return SO_REUSEPORT;
    return -1;
}

static int darwin_convert_sockopt_ip_option(int opt)
{
    if (opt == 1) return IP_TOS;
    if (opt == 2) return IP_TTL;
    if (opt == 3) return IP_HDRINCL;
    if (opt == 4) return IP_OPTIONS;
    //if (opt == 5) return IP_ROUTER_ALERT;
    if (opt == 6) return IP_RECVOPTS;
    if (opt == 7) return IP_RETOPTS;
    //if (opt == 8) return IP_PKTINFO;
    //if (opt == 9) return IP_PKTOPTIONS;
    //if (opt == 10) return IP_MTU_DISCOVER;
    //if (opt == 11) return IP_RECVERR;
    //if (opt == 12) return IP_RECVTTL;
    //if (opt == 13) return IP_RECVTOS;
    //if (opt == 14) return IP_MTU;
    //if (opt == 15) return IP_FREEBIND;
    if (opt == 16) return IP_IPSEC_POLICY;
    //if (opt == 17) return IP_XFRM_POLICY;
    //if (opt == 18) return IP_PASSSEC;

    if (opt == 32) return IP_MULTICAST_IF;
    if (opt == 33) return IP_MULTICAST_TTL;
    if (opt == 34) return IP_MULTICAST_LOOP;
    if (opt == 35) return IP_ADD_MEMBERSHIP;
    if (opt == 36) return IP_DROP_MEMBERSHIP;
    //if (opt == 37) return IP_UNBLOCK_SOURCE;
    //if (opt == 38) return IP_BLOCK_SOURCE;
    //if (opt == 39) return IP_ADD_SOURCE_MEMBERSHIP;
    //if (opt == 40) return IP_DROP_SOURCE_MEMBERSHIP;
    //if (opt == 41) return IP_MSFILTER;
    return -1;
}

static int darwin_convert_sockopt_ipv6_option(int opt)
{
    // if (opt == 1) return IPV6_ADDRFORM;
    if (opt == 2) return IPV6_2292PKTINFO;
    if (opt == 3) return IPV6_2292HOPOPTS;
    if (opt == 4) return IPV6_2292DSTOPTS;
    if (opt == 5) return IPV6_2292RTHDR;
    if (opt == 6) return IPV6_2292PKTOPTIONS;
    if (opt == 7) return IPV6_CHECKSUM;
    if (opt == 8) return IPV6_2292HOPLIMIT;
    // if (opt == 9) return IPV6_NEXTHOP;
    // if (opt == 10) return IPV6_AUTHHDR;
    if (opt == 16) return IPV6_UNICAST_HOPS;
    if (opt == 17) return IPV6_MULTICAST_IF;
    if (opt == 18) return IPV6_MULTICAST_HOPS;
    if (opt == 19) return IPV6_MULTICAST_LOOP;
    if (opt == 20) return IPV6_JOIN_GROUP;
    if (opt == 21) return IPV6_LEAVE_GROUP;
    // if (opt == 22) return IPV6_ROUTER_ALERT;
    // if (opt == 23) return IPV6_MTU_DISCOVER;
    // if (opt == 24) return IPV6_MTU;
    // if (opt == 25) return IPV6_RECVERR;
    if (opt == 26) return IPV6_V6ONLY;
    ///if (opt == 27) return IPV6_JOIN_ANYCAST;
    // if (opt == 28) return IPV6_LEAVE_ANYCAST;
    if (opt == 34) return IPV6_IPSEC_POLICY;
    // if (opt == 35) return IPV6_XFRM_POLICY;
    // if (opt == 36) return IPV6_HDRINCL;
    return -1;
}

static int darwin_convert_sockopt_option(int *level, int *optname)
{
    if (*level == 1) { // SOL_SOCKET
        *level = SOL_SOCKET;
        *optname = darwin_convert_sockopt_socket_option(*optname);
    }
    if (*level == IPPROTO_IP) {
        *optname = darwin_convert_sockopt_ip_option(*optname);
    }
    if (*level == IPPROTO_IPV6) {
        *optname = darwin_convert_sockopt_ipv6_option(*optname);
    }
    if (*optname == -1) {
        printf("WARN: unknown sockopt\n");
        return 0;
    }
    return 1;
}

int darwin_my_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    if (!darwin_convert_sockopt_option(&level, &optname))
        return 0;
    return getsockopt(sockfd, level, optname, optval, optlen);
}

int darwin_my_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    if (!darwin_convert_sockopt_option(&level, &optname))
        return 0;
    return setsockopt(sockfd, level, optname, optval, optlen);
}


sa_family_t darwin_convert_family_to_native(unsigned short family)
{
    if (family == 0) return AF_UNSPEC;
    if (family == 2) return AF_INET;
    if (family == 10) return AF_INET6;
    printf("darwin_convert_family_to_native: unsupported family %i\n", family);
    return family;
}

unsigned short darwin_convert_family_from_native(sa_family_t family)
{
    if (family == AF_UNSPEC) return 0;
    if (family == AF_INET) return 2;
    if (family == AF_INET6) return 10;
    printf("darwin_convert_family_to_native: unsupported family %i\n", family);
    return family;
}

int darwin_get_addr_size(__const struct android_sockaddr *from_addr)
{
    sa_family_t family = darwin_convert_family_to_native(from_addr->sa_family);
    if (family == AF_INET) {
        return sizeof(struct sockaddr_in);
    } else if (family == AF_INET6) {
        return sizeof(struct sockaddr_in6);
    } else {
        printf("darwin_get_addr_size: unsupported family\n");
        return 0;
    }
}

int darwin_convert_addr_to_native(__const struct android_sockaddr *from_addr, struct sockaddr *to_addr)
{
    sa_family_t family = darwin_convert_family_to_native(from_addr->sa_family);
    if (family == AF_INET) {
        memcpy(to_addr, from_addr, sizeof(struct sockaddr_in));
        to_addr->sa_len = sizeof(struct sockaddr_in);
    } else if (family == AF_INET6) {
        memcpy(to_addr, from_addr, sizeof(struct sockaddr_in6));
        to_addr->sa_len = sizeof(struct sockaddr_in6);
    } else {
        printf("darwin_convert_addr_to_native: unsupported family\n");
        return 0;
    }
    to_addr->sa_family = family;
    return 1;
}

int darwin_convert_addr_from_native(__const struct sockaddr *from_addr, struct android_sockaddr *to_addr,
                                    size_t max_size)
{
    size_t size = from_addr->sa_len;
    if (max_size < size)
        size = max_size;
    memcpy(to_addr, from_addr, size);
    to_addr->sa_family = darwin_convert_family_from_native(from_addr->sa_family);
    return 1;
}

int darwin_my_socket(int socket_family, int socket_type, int protocol)
{
    socket_family = darwin_convert_family_to_native(socket_family);
    return socket(socket_family, socket_type, protocol);
}

int darwin_my_bind(int sockfd, const struct android_sockaddr *addr, socklen_t addrlen)
{
    struct sockaddr *conv_addr = alloca(darwin_get_addr_size(addr));
    if (!darwin_convert_addr_to_native(addr, conv_addr))
        return -1;
    return bind(sockfd, conv_addr, addrlen);
}

ssize_t darwin_my_sendto(int socket, const void *buffer, size_t length, int flags,
                         const struct android_sockaddr *dest_addr, socklen_t dest_len)
{
    struct sockaddr *conv_addr = alloca(darwin_get_addr_size(dest_addr));
    if (!darwin_convert_addr_to_native(dest_addr, conv_addr))
        return -1;
    return sendto(socket, buffer, length, flags, conv_addr, dest_len);
}

ssize_t darwin_my_recvfrom(int sockfd, void *buf, size_t len, int flags,
                           struct android_sockaddr *addr, socklen_t *addrlen)
{
    struct sockaddr_storage stor;
    socklen_t ret_size = sizeof(stor);
    int ret = recvfrom(sockfd, buf, len, flags, (struct sockaddr *) &stor, &ret_size);
    if (ret >= 0) {
        //stor.ss_len = ret_size;
        if (!darwin_convert_addr_from_native((struct sockaddr *) &stor, addr, *addrlen))
            return -1;
        *addrlen = ret_size - 1;
    }
    return ret;
}

int darwin_my_getsockname(int sockfd, struct android_sockaddr *addr, socklen_t *addrlen)
{
    struct sockaddr_storage stor;
    socklen_t ret_size = sizeof(stor);
    int ret = getsockname(sockfd, (struct sockaddr *) &stor, &ret_size);
    if (ret == 0) {
        //stor.ss_len = ret_size;
        if (!darwin_convert_addr_from_native((struct sockaddr *) &stor, addr, *addrlen))
            return -1;
        *addrlen = ret_size - 1;
    }
    return ret;
}

struct android_addrinfo* darwin_convert_addrinfo(struct addrinfo* res)
{
    struct android_addrinfo* ares = (struct android_addrinfo*) malloc(sizeof(struct android_addrinfo));
    ares->ai_flags = res->ai_flags;
    ares->ai_family = darwin_convert_family_from_native(res->ai_family);
    ares->ai_socktype = res->ai_socktype;
    ares->ai_protocol = res->ai_protocol;
    ares->ai_addrlen = res->ai_addrlen;
    ares->ai_canonname = NULL; //res->ai_canonname;
    size_t conv_addrlen = res->ai_addrlen;
    struct android_sockaddr *conv_addr = malloc(conv_addrlen);
    darwin_convert_addr_from_native(res->ai_addr, conv_addr, conv_addrlen);
    ares->ai_addrlen = res->ai_addrlen;
    ares->ai_addr = conv_addr;
    ares->ai_next = NULL;
    if (res->ai_next != NULL) {
        ares->ai_next = darwin_convert_addrinfo(res->ai_next);
    }
    return ares;
}

int darwin_my_getaddrinfo(const char *node, const char *service,
                          const struct android_addrinfo *ahints,
                          struct android_addrinfo **ares)
{
    struct addrinfo hints;
    if (ahints != NULL) {
        hints.ai_flags = ahints->ai_flags;
        hints.ai_family = darwin_convert_family_to_native(ahints->ai_family);
        hints.ai_socktype = ahints->ai_socktype;
        hints.ai_protocol = ahints->ai_protocol;
        hints.ai_addrlen = ahints->ai_addrlen;
        hints.ai_canonname = ahints->ai_canonname;
        hints.ai_addr = NULL;
        if (ahints->ai_addr != NULL) {
            hints.ai_addr = alloca(darwin_get_addr_size(ahints->ai_addr));
            darwin_convert_addr_to_native(ahints->ai_addr, hints.ai_addr);
        }
    }
    struct addrinfo* res;
    int ret = getaddrinfo(node, service, (ahints == NULL ? NULL : &hints), &res);
    if (ret != 0) {
        return ret;
    }
    if (res != NULL) {
        *ares = darwin_convert_addrinfo(res);
        freeaddrinfo(res);
    } else {
        *ares = NULL;
    }
    return ret;
}

void darwin_my_freeaddrinfo(struct android_addrinfo *ai)
{
    struct android_addrinfo *ai_next;
    while (ai) {
        if (ai->ai_canonname)
            free(ai->ai_canonname);
        if (ai->ai_addr)
            free(ai->ai_addr);
        ai_next = ai->ai_next;
        free(ai);
        ai = ai_next;
    }
}

int darwin_my_getnameinfo (const struct android_sockaddr *__restrict sa,
                    socklen_t salen, char *__restrict host,
                    socklen_t hostlen, char *__restrict serv,
                    socklen_t servlen, int flags) {
    socklen_t conv_salen = darwin_get_addr_size(sa);
    struct sockaddr *conv_addr = alloca(conv_salen);
    if (!darwin_convert_addr_to_native(sa, conv_addr))
        return -1;
    int glibc_flags = convert_getnameinfo_flags(flags);
    return getnameinfo(conv_addr, conv_salen, host, hostlen, serv, servlen, glibc_flags);
}


static struct _hook net_darwin_hooks[] = {
    {"socket", darwin_my_socket},
    {"bind", darwin_my_bind},
    {"sendto", darwin_my_sendto},
    {"recvfrom", darwin_my_recvfrom},
    {"getsockname", darwin_my_getsockname},
    {"getsockopt", darwin_my_getsockopt},
    {"setsockopt", darwin_my_setsockopt},
    {"getaddrinfo", darwin_my_getaddrinfo},
    {"freeaddrinfo", darwin_my_freeaddrinfo},
    {"getnameinfo", darwin_my_getnameinfo},
    {NULL, NULL}
};
REGISTER_HOOKS(net_darwin_hooks)