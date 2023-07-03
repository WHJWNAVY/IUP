#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#if defined(_MSC_VER) || defined(_WIN32)
#define _USE_GUI_ 1

#ifndef WIN32
#define WIN32 1
#endif
#endif

#ifdef __GNUC__
#include <unistd.h>
#include <errno.h>
#endif

#ifndef _USE_GUI_
#ifdef __GNUC__
#include <getopt.h>
#else
#include "my_getopt.h"
#endif
#else
#include <iup.h>
#define MCUPG_UI_VERSION               \
    ("Create by WHJWNAVY at ["__DATE__ \
     " " __TIME__ "]")
#endif
#ifndef IUP_MULTILINE
#define IUP_MULTILINE "MULTILINE"
#endif

#ifndef IUP_VALUECHANGED_CB
#define IUP_VALUECHANGED_CB "VALUECHANGED_CB"
#endif

#if defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#include <iphlpapi.h>

#if defined(_MSC_VER)
// Link with Iphlpapi.lib
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

#define ssize_t int32_t
#define socket_t SOCKET
#define SOCKET_INITIALIZE INVALID_SOCKET
//#define sleep_ms(_ms_) Sleep(_ms_)
#define sleep_ms(_ms_) sleep_us((_ms_)*1000)
#define NEW_LINE "\r\n"
#else

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define socket_t int32_t
#define SOCKET_INITIALIZE 0
#define sleep_ms(_ms_) usleep((_ms_)*1000)
#define sleep_us(_us_) usleep(_us_)
#define NEW_LINE "\n"
#endif

#define _DEBUG_ 1
#define MCUPG_BUFF_LEN 256

#define IFACE_LIST_MAX 10

#ifndef IF_NAMESIZE
#define IF_NAMESIZE 16
#endif

typedef struct {
    char ac_name[IF_NAMESIZE + 1];
    char ac_ipaddr[INET_ADDRSTRLEN + 1];
} iface_t;

typedef struct {
    iface_t list[IFACE_LIST_MAX];
    uint32_t num;
} ifaces_t;

#ifdef _USE_GUI_
// TODO make this not global
typedef struct {
    struct {
        Ihandle *uiMainWin;
        Ihandle *uiStartBtn;
        Ihandle *uiStopBtn;
        Ihandle *uiFileLable;
        Ihandle *uiIfaceCombobox;
        Ihandle *uiDelayCombobox;
        Ihandle *uiProgressBar;
        ifaces_t pIfaces;
    } S;

    struct {
        char ifName[MCUPG_BUFF_LEN + 1];
        char *destAddr;
        int32_t destPort;
        int32_t srcPort;
        socket_t serverSck;

        char fileName[MCUPG_BUFF_LEN + 1];
        char *fileBuff;
        uint32_t fileSize;

        int32_t delayTime;
        int32_t loopCnt;
        bool bThreadRun;
        bool bArgsLoaded;
    } D;
} MCUPG_GLOBAL_T;

MCUPG_GLOBAL_T G_ARGS = {0};

#define UI_MWIN G_ARGS.S.uiMainWin
#endif // _USE_GUI_

#ifndef _USE_GUI_
#ifdef _DEBUG_
#define LOG_DEBUG(FMT, ...)                                                              \
    do {                                                                                 \
        fprintf(stderr, "(%s:%d) DEBUG " FMT "\r\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#else
#define LOG_DEBUG(FMT, ...)
#endif
#define LOG_ERROR(FMT, ...)                                                              \
    do {                                                                                 \
        fprintf(stderr, "(%s:%d) ERROR " FMT "\r\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#else
#ifdef _DEBUG_
#define LOG_DEBUG(FMT, ...)                                                              \
    do {                                                                                 \
        fprintf(stderr, "(%s:%d) DEBUG " FMT "\r\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#else
#define LOG_DEBUG(FMT, ...)
#endif
#define LOG_ERROR(FMT, ...)                                                     \
    do {                                                                        \
        IupMessagef("Error", "(%s:%d)" FMT, __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#endif

#define DEF_MCAST_ADDR "224.0.0.120"
#define DEF_DST_PORT 456
#define DEF_SRC_PORT 456

#define MCUPG_SERVICE_PORT (DEF_DST_PORT) // udp 端口号

#if 1 // 分片报文内容长度(不包含报头)
#define MCUPG_PACKET_LEN 1024
#else
// (MTU_SIZE(1500) - ETH_HDR_SIZE(14) - IP_HDR_SIZE(20) - UDP_HDR_SIZE(8) - PKT_HDR_SIZE(24) - RETAIN(8))
//  = 1500 - 14 - 20 - 8 - 24 - 8 = 1426
#define MCUPG_PACKET_LEN 1426
#endif

#define MCUPG_PROTO_NUM (0x0020)
#define MCUPG_PROTO_END (0x0040)
#define MCUPG_FLAGS_LEN 8
#define MCUPG_FLAGS_STR "TCBulkFW"
#define MCUPG_PACKED_LEN 8

#ifndef __GNUC__
#pragma pack(1)
#pragma pack(show)
struct mcupg_pkthdr_s {
    uint16_t ui_proto; // 报文分片标记, 0x0040表示最后一个分片, 0x0020表示其他分片(非最后一个)
    uint16_t ui_seq;   // 报文分片ID, 起始值为1
    uint8_t ac_flag[MCUPG_FLAGS_LEN]; // 报文标记, "TCBulkFW"
    uint32_t ui_len;                  // 镜像文件总长度

    uint8_t __packed__[MCUPG_PACKED_LEN]; // 报文头24字节对齐
};
#pragma pack()
#pragma pack(show)
#else
struct mcupg_pkthdr_s {
    uint16_t ui_proto; // 报文分片标记, 0x0040表示最后一个分片, 0x0020表示其他分片(非最后一个)
    uint16_t ui_seq;   // 报文分片ID, 起始值为1
    uint8_t ac_flag[MCUPG_FLAGS_LEN];     // 报文标记, "TCBulkFW"
    uint32_t ui_len;                      // 镜像文件总长度
    uint8_t __packed__[MCUPG_PACKED_LEN]; // 报文头24字节对齐
} __attribute__((packed));
#endif

typedef struct mcupg_pkthdr_s mcupg_pkthdr_t;

typedef struct mcupg_pkt_s {
    mcupg_pkthdr_t header;
    uint8_t data[MCUPG_PACKET_LEN];
} mcupg_pkt_t;

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define MCUPG_PKTHDR_BYTE_ORDER(_ppkthdr_)
#else
#define MCUPG_PKTHDR_BYTE_ORDER(_ppkthdr_)                                 \
    do {                                                                   \
        mcupg_pkthdr_t *__t_ppkthdr_t__ = (mcupg_pkthdr_t *)(_ppkthdr_);   \
        __t_ppkthdr_t__->ui_proto = __bswap_16(__t_ppkthdr_t__->ui_proto); \
        __t_ppkthdr_t__->ui_seq = __bswap_16(__t_ppkthdr_t__->ui_seq);     \
        __t_ppkthdr_t__->ui_len = __bswap_32(__t_ppkthdr_t__->ui_len);     \
    } while (0)
#endif

#define INT32TOSTR(_v_)                                                     \
    ({                                                                      \
        char __buff__[32] = {0};                                            \
        snprintf(__buff__, sizeof(__buff__), "%d" PRId32, (uint32_t)(_v_)); \
        __buff__;                                                           \
    })

#define INT64TOSTR(_v_)                                                   \
    ({                                                                    \
        char __buff__[64] = {0};                                          \
        snprintf(__buff__, sizeof(__buff__), "%" PRId64, (int64_t)(_v_)); \
        __buff__;                                                         \
    })

#define UINT32TOSTR(_v_)                                                   \
    ({                                                                     \
        char __buff__[32] = {0};                                           \
        snprintf(__buff__, sizeof(__buff__), "%" PRIu32, (uint32_t)(_v_)); \
        __buff__;                                                          \
    })

#define UINT64TOSTR(_v_)                                                   \
    ({                                                                     \
        char __buff__[64] = {0};                                           \
        snprintf(__buff__, sizeof(__buff__), "%" PRIu64, (uint64_t)(_v_)); \
        __buff__;                                                          \
    })

char *G_DELAY_LIST[] = {"0", "10", "50", "100"};
#define ARRAY_SIZE(_array_) (sizeof(_array_) / sizeof(_array_[0]))

#if (defined(WIN32))
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

/* Note: could also use malloc() and free() */
#if 1
void sleep_us(uint64_t waitTime) {
    uint64_t time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *)&time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *)&time2);
    } while ((time2 - time1) < waitTime);
}
#else
void sleep_us(uint64_t delayTime) {
#define MICROSEC (1000000)
    double time = 0.0;
    LARGE_INTEGER freq = {0};
    LARGE_INTEGER start = {0};
    LARGE_INTEGER now = {0};

    if (!QueryPerformanceFrequency(&freq)) {
        LOG_ERROR("Don't support!");
        return;
    }

    QueryPerformanceCounter(&start);

    for (;;) {
        QueryPerformanceCounter(&now);
        time = ((double)((now.QuadPart - start.QuadPart) * MICROSEC) / (double)freq.QuadPart);
        if (time >= (double)delayTime) {
            break;
        }
    }
}
#endif

static int32_t myGetIpAddrTable(PMIB_IPADDRTABLE *ppIPAddrTable) {
    int32_t rc = 0;
    /* Variables used by GetIpAddrTable */
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    PMIB_IPADDRTABLE pIPAddrTable = NULL;

    if (ppIPAddrTable == NULL) {
        LOG_ERROR("Invalid args!");
        rc = -1;
        goto err;
    }

    // Before calling AddIPAddress we use GetIpAddrTable to get
    // an adapter to which we can add the IP.
    pIPAddrTable = (MIB_IPADDRTABLE *)MALLOC(sizeof(MIB_IPADDRTABLE));
    if (pIPAddrTable == NULL) {
        LOG_ERROR("Memory allocation failed for GetIpAddrTable");
        rc = -1;
        goto err;
    }

    // Make an initial call to GetIpAddrTable to get the
    // necessary size into the dwSize variable
    if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIPAddrTable);
        pIPAddrTable = (MIB_IPADDRTABLE *)MALLOC(dwSize);
    }
    if (pIPAddrTable == NULL) {
        LOG_ERROR("Memory allocation failed for GetIpAddrTable");
        rc = -1;
        goto err;
    }
    // Make a second call to GetIpAddrTable to get the
    // actual data we want
    if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) != NO_ERROR) {
        LOG_ERROR("GetIpAddrTable failed with error %d", dwRetVal);
        rc = -1;
        goto err;
    }

    *ppIPAddrTable = pIPAddrTable;
    rc = 0;
err:
    if ((rc != 0) && (pIPAddrTable != NULL)) {
        FREE(pIPAddrTable);
    }
    return rc;
}

static int32_t myGetIpAddrByIfidx(const char *Ifindex, char *pIPAddr) {
    int32_t rc = 0, idx = 0;
    int32_t Ifidx = 0;
    char *aIPAddr = NULL;
    IN_ADDR IPAddr = {0};
    PMIB_IPADDRTABLE pIPAddrTable = NULL;

    if ((Ifindex == NULL) || (pIPAddr == NULL)) {
        LOG_ERROR("Invalid args!");
        rc = -1;
        goto err;
    }

    if ((Ifidx = atol(Ifindex)) < 0) {
        LOG_ERROR("Invalid ifIndex [%s]!", Ifindex);
        rc = Ifidx;
        goto err;
    }

    rc = myGetIpAddrTable(&pIPAddrTable);
    if ((rc != 0) || (pIPAddrTable == NULL)) {
        LOG_ERROR("Failed to get ipaddr table [%d]", rc);
        rc = ((rc == 0) ? -1 : rc);
        goto err;
    }

    for (idx = 0; idx < (int32_t)pIPAddrTable->dwNumEntries; idx++) {
        if (pIPAddrTable->table[idx].dwIndex == Ifidx) {
            IPAddr.S_un.S_addr = (u_long)(pIPAddrTable->table[idx].dwAddr);
            if ((aIPAddr = inet_ntoa(IPAddr)) != NULL) {
                strncpy(pIPAddr, aIPAddr, INET_ADDRSTRLEN);
                rc = 0;
                goto err;
            }
        }
    }
    rc = 1;
err:
    if (pIPAddrTable != NULL) {
        FREE(pIPAddrTable);
        pIPAddrTable = NULL;
    }
    return rc;
}

static int32_t ResolveAddress(char *apIPAddr, uint16_t port, struct addrinfo **result) {
    int32_t rc = 0, ifidx = 0;
    struct addrinfo hints, *res = NULL;
    char aPorts[32] = {0};

    if ((apIPAddr == NULL) || (port == 0) || (result == NULL)) {
        LOG_ERROR("Invalid args!");
        rc = -1;
        goto err;
    }
    snprintf(aPorts, sizeof(aPorts) - 1, "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;        // ((addr) ? 0 : AI_PASSIVE);
    hints.ai_family = AF_INET; // ((addr) ? AF_UNSPEC : af);
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    if ((rc = getaddrinfo(apIPAddr, aPorts, &hints, &res)) != 0) {
        LOG_ERROR("Invalid address %s, getaddrinfo failed: %d\n", apIPAddr, rc);
        goto err;
    }
    *result = res;
    rc = 0;
err:
    if ((rc != 0) && (res != NULL)) {
        freeaddrinfo(res);
    }
    return rc;
}

static int32_t PrintAddress(SOCKADDR *sa, int32_t salen, char *outs) {
    char host[NI_MAXHOST] = {0}, serv[NI_MAXSERV] = {0};
    int32_t hostlen = NI_MAXHOST, servlen = NI_MAXSERV, rc = 0;

    // Validate argument
    if ((sa == NULL) || (outs == NULL)) {
        return WSAEFAULT;
    }

    rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc != 0) {
        LOG_ERROR("getnameinfo failed: %d\n", rc);
        return rc;
    }

    // If the port is zero then don't print it
    if (strncmp(serv, "0", 1) != 0) {
        if (sa->sa_family == AF_INET6) {
            sprintf(outs, "[%s]:%s", host, serv);
        } else {
            sprintf(outs, "%s:%s", host, serv);
        }
    } else {
        sprintf(outs, "%s", host);
    }

    return NO_ERROR;
}

static int32_t SetMulticastLoopBack(SOCKET s, int32_t af, int32_t loopval) {
    char *optval = NULL;
    int32_t optlevel = 0, option = 0, optlen = 0, rc = 0;

    rc = NO_ERROR;
    if (af == AF_INET) {
        // Set the v4 options
        optlevel = IPPROTO_IP;
        option = IP_MULTICAST_LOOP;
        optval = (char *)&loopval;
        optlen = sizeof(loopval);
    } else if (af == AF_INET6) {
        // Set the v6 options
        optlevel = IPPROTO_IPV6;
        option = IPV6_MULTICAST_LOOP;
        optval = (char *)&loopval;
        optlen = sizeof(loopval);
    } else {
        LOG_ERROR("Attemtping to set multicast loopback for invalid address family!\n");
        rc = SOCKET_ERROR;
    }
    if (rc != SOCKET_ERROR) {
        // Set the multpoint loopback
        rc = setsockopt(s, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR) {
            LOG_ERROR("setsockopt failed: %d\n", WSAGetLastError());
        } else {
            LOG_DEBUG("Setting multicast loopback to: %d\n", loopval);
        }
    }
    return rc;
}

static int32_t SetSendInterface(SOCKET s, struct addrinfo *iface) {
    char *optval = NULL;
    int32_t optlevel = 0, option = 0, optlen = 0, rc = 0;
    char aIPaddrs[128] = {0};

    rc = NO_ERROR;
    if (iface->ai_family == AF_INET) {
        // Setup the v4 option values
        optlevel = IPPROTO_IP;
        option = IP_MULTICAST_IF;
        optval = (char *)&((SOCKADDR_IN *)iface->ai_addr)->sin_addr.s_addr;
        optlen = sizeof(((SOCKADDR_IN *)iface->ai_addr)->sin_addr.s_addr);
    } else if (iface->ai_family == AF_INET6) {
        // Setup the v6 option values
        optlevel = IPPROTO_IPV6;
        option = IPV6_MULTICAST_IF;
        optval = (char *)&((SOCKADDR_IN6 *)iface->ai_addr)->sin6_scope_id;
        optlen = sizeof(((SOCKADDR_IN6 *)iface->ai_addr)->sin6_scope_id);
    } else {
        LOG_ERROR("Attemtping to set sent interface for invalid address family!\n");
        rc = SOCKET_ERROR;
    }
    // Set send IF
    if (rc != SOCKET_ERROR) {
        // Set the send interface
        rc = setsockopt(s, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR) {
            LOG_ERROR("setsockopt failed: %d\n", WSAGetLastError());
        } else {
            PrintAddress(iface->ai_addr, (int32_t)iface->ai_addrlen, aIPaddrs);
            LOG_DEBUG("Set sending interface to: %s", aIPaddrs);
        }
    }
    return rc;
}

static int32_t print_iface_list(void) {
    int32_t rc = 0, idx = 0;
    IN_ADDR IPAddr = {0};
    PMIB_IPADDRTABLE pIPAddrTable = NULL;

    rc = myGetIpAddrTable(&pIPAddrTable);
    if ((rc != 0) || (pIPAddrTable == NULL)) {
        LOG_ERROR("Failed to get ipaddr table [%d]", rc);
        rc = ((rc == 0) ? -1 : rc);
        goto err;
    }

    fprintf(stderr, "\tInterface List:\n");
    fprintf(stderr, "\t\tIndex\tIpaddr\n");
    for (idx = 0; idx < (int32_t)pIPAddrTable->dwNumEntries; idx++) {
        IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[idx].dwAddr;
        fprintf(stderr, "\t\t%ld\t%s\n", pIPAddrTable->table[idx].dwIndex, inet_ntoa(IPAddr));
    }

err:
    if (pIPAddrTable != NULL) {
        FREE(pIPAddrTable);
        pIPAddrTable = NULL;
    }
    return rc;
}

static int32_t get_iface_list(ifaces_t *ifaces) {
    int32_t rc = 0;
    int32_t idx = 0;
    IN_ADDR IPAddr = {0};
    PMIB_IPADDRTABLE pIPAddrTable = NULL;
    iface_t *piface = NULL;
    uint32_t *pcnt = NULL;

    if (ifaces == NULL) {
        LOG_ERROR("Invalid args!");
        rc = -1;
        goto err;
    }

    rc = myGetIpAddrTable(&pIPAddrTable);
    if ((rc != 0) || (pIPAddrTable == NULL)) {
        LOG_ERROR("Failed to get ipaddr table [%d]", rc);
        rc = ((rc == 0) ? -1 : rc);
        goto err;
    }

    pcnt = &(ifaces->num);
    *pcnt = 0;
    for (idx = 0; idx < (int32_t)pIPAddrTable->dwNumEntries; idx++) {
        piface = &(ifaces->list[*pcnt]);
        IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[idx].dwAddr;
        snprintf(piface->ac_name, IF_NAMESIZE, "%ld", pIPAddrTable->table[idx].dwIndex);
        snprintf(piface->ac_ipaddr, INET_ADDRSTRLEN, "%s", inet_ntoa(IPAddr));
        *pcnt += 1;
        if ((*pcnt) >= IFACE_LIST_MAX) {
            break;
        }
    }

    rc = 0;
err:
    if (pIPAddrTable != NULL) {
        FREE(pIPAddrTable);
        pIPAddrTable = NULL;
    }
    return rc;
}

static int32_t mcast_server_init(socket_t *psck, const char *ifname, uint16_t src_port) {
    int32_t ret = 0;
    WSADATA wsd = {0};
    struct addrinfo *reslocal = NULL;
    char ac_ipaddr[INET_ADDRSTRLEN] = {0};
    socket_t sck = INVALID_SOCKET;
    int32_t loop = 0;

    if ((psck == NULL) || (ifname == NULL)) {
        LOG_ERROR("Invalid args!");
        ret = -1;
        goto err;
    }

    if ((ret = WSAStartup(MAKEWORD(2, 2), &wsd)) != 0) {
        LOG_ERROR("WSAStartup failed: %d\n", ret);
        goto err;
    }

    ret = myGetIpAddrByIfidx(ifname, ac_ipaddr);
    if (ret != 0) {
        LOG_ERROR("Failed to get ipaddr for ifname [%s]", ifname);
        goto err;
    }
    LOG_DEBUG("Get ipaddr [%s] for ifname [%s]", ac_ipaddr, ifname);

    ret = ResolveAddress(ac_ipaddr, src_port, &reslocal);
    if ((ret != 0) || (reslocal == NULL)) {
        LOG_ERROR("Failed to ResolveAddress for ifname [%s], port [%d]", ifname, src_port);
        ret = ((ret == 0) ? -1 : ret);
        goto err;
    }

    // Create the socket - remember to specify the multicast flags
    sck = WSASocket(reslocal->ai_family, reslocal->ai_socktype, reslocal->ai_protocol, NULL, 0,
                    WSA_FLAG_OVERLAPPED | WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF);
    if (sck == INVALID_SOCKET) {
        LOG_ERROR("socket(af = %d) failed: %d\n", reslocal->ai_family, WSAGetLastError());
        ret = -1;
        goto err;
    }

    ret = bind(sck, reslocal->ai_addr, (int32_t)reslocal->ai_addrlen);
    if (ret != NO_ERROR) {
        LOG_ERROR("bind failed: %d\n", WSAGetLastError());
        goto err;
    }

    ret = SetMulticastLoopBack(sck, reslocal->ai_family, loop);
    if (ret != NO_ERROR) {
        LOG_ERROR("Unable to set multicast loopback flag!");
        goto err;
    }

    ret = SetSendInterface(sck, reslocal);
    if (ret != NO_ERROR) {
        LOG_ERROR("Unable to set outgoing multicast interface!");
        goto err;
    }

    *psck = sck;
    ret = 0;
err:
    if (reslocal != NULL) {
        freeaddrinfo(reslocal);
        reslocal = NULL;
    }
    if (ret != 0) {
        if (sck != INVALID_SOCKET) {
            closesocket(sck);
        }
        WSACleanup();
    }
    return ret;
}
#else
static int32_t mcast_server_init(socket_t *psck, const char *ifname, uint16_t src_port) {
    int32_t ret = 0;
    struct ip_mreqn mreqn = {0};
    struct sockaddr_in local_in = {0};
    int32_t sck = 0, loop = 0, iface_idx = 0;

    if (psck == NULL) {
        LOG_ERROR("Invalid args!");
        ret = -1;
        goto err;
    }

    sck = socket(AF_INET, SOCK_DGRAM, 0);
    if (sck < 0) {
        LOG_ERROR("Socket failed [%d] [%d:%s]!", sck, errno, strerror(errno));
        ret = sck;
        goto err;
    }

    if (ifname == NULL) {
        iface_idx = 0;
    } else {
        iface_idx = if_nametoindex(ifname);
    }
    if (iface_idx < 0) {
        ret = iface_idx;
        LOG_ERROR("Invalid iface index [%d]!", iface_idx);
        goto err;
    }

    LOG_DEBUG("ifname [%s], ifIndex [%d]", ((ifname == NULL) ? "NULL" : ifname), iface_idx);

    memset(&mreqn, 0, sizeof(mreqn));
    mreqn.imr_ifindex = iface_idx;
    ret = setsockopt(sck, IPPROTO_IP, IP_MULTICAST_IF, &mreqn, sizeof(mreqn));
    if (ret < 0) {
        LOG_ERROR("setsockopt IP_MULTICAST_IF failed [%d] [%d:%s]!", ret, errno, strerror(errno));
        goto err;
    }

    ret = setsockopt(sck, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    if (ret < 0) {
        LOG_ERROR("setsockopt IP_MULTICAST_LOOP failed [%d] [%d:%s]!", ret, errno, strerror(errno));
        goto err;
    }

    if (src_port != 0) {
        memset(&local_in, 0, sizeof(local_in));
        local_in.sin_family = AF_INET;
        local_in.sin_port = htons(src_port);
        ret = bind(sck, (const struct sockaddr *)&local_in, sizeof(local_in));
        if (ret < 0) {
            LOG_ERROR("Bind src port [%d] failed [%d] [%d:%s]!", src_port, ret, errno, strerror(errno));
            goto err;
        }
    }

    *psck = sck;
    ret = 0;
err:
    return ret;
}

#define TRACE_NEQ_GOTO(__ret__, __val__, __label__) \
    do {                                            \
        typeof(__val__) __ret1__ = (__ret__);       \
        if ((__ret1__) != __val__) {                \
            goto __label__;                         \
        }                                           \
    } while (0)

#define TRACE_GOTO(__ret__, __label__) TRACE_NEQ_GOTO(__ret__, 0, __label__)

#define _PADDR_(_paddr_) (htonl(*(uint32_t *)(_paddr_)))

static int32_t get_iface_ipaddr(const char *ifname, char *ipaddr) {
    int32_t sck = -1;
    int32_t ecode = 0;
    struct ifreq ifr = {0};
    struct sockaddr_in *ip = NULL;
    in_addr_t *addr = NULL;

    if (!ifname || !ipaddr) {
        TRACE_GOTO((ecode = -1), err);
    }

    if (0 == strlen(ifname)) {
        TRACE_GOTO((ecode = -1), err);
    }

    if ((sck = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
        TRACE_GOTO((ecode = -1), err);
    }

    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

    if (0 != ioctl(sck, SIOCGIFADDR, &ifr)) {
        TRACE_GOTO((ecode = -1), err);
    }

    ip = (struct sockaddr_in *)&ifr.ifr_addr;
    addr = &(ip->sin_addr.s_addr);

    // printf("address: 0x%08x\n", _PADDR_(addr));

    if (IN_MULTICAST(_PADDR_(addr)) || (IN_BADCLASS(_PADDR_(addr))) || (_PADDR_(addr) == INADDR_LOOPBACK)) {
        TRACE_GOTO((ecode = -1), err);
    }

    if (inet_ntop(AF_INET, addr, ipaddr, INET_ADDRSTRLEN) == NULL) {
        TRACE_GOTO((ecode = -1), err);
    }

    ecode = 0;
err:
    if (sck > 0) {
        close(sck);
    }
    return ecode;
}

static int32_t get_iface_list(ifaces_t *ifaces) {
    int32_t rc = 0;
    int32_t idx = 0, jdx = 0;

    struct if_nameindex *if_ni = NULL, *i = NULL;
    char ipaddr[INET_ADDRSTRLEN + 1] = {0};

    iface_t *piface = NULL;
    uint32_t *pcnt = NULL;

    if (ifaces == NULL) {
        LOG_ERROR("Invalid args!");
        rc = -1;
        goto err;
    }

    if_ni = if_nameindex();
    if (if_ni == NULL) {
        perror("if_nameindex");
        LOG_ERROR("if_nameindex!");
        rc = -1;
        goto err;
    }

    pcnt = &(ifaces->num);
    *pcnt = 0;
    for (i = if_ni; !(i->if_index == 0 && i->if_name == NULL); i++) {
        if (get_iface_ipaddr(i->if_name, ipaddr) == 0) {
            piface = &(ifaces->list[*pcnt]);
            snprintf(piface->ac_name, IF_NAMESIZE, "%ld", i->if_name);
            snprintf(piface->ac_ipaddr, INET_ADDRSTRLEN, "%s", ipaddr);
            *pcnt += 1;
            if ((*pcnt) >= IFACE_LIST_MAX) {
                break;
            }
        }
    }

    rc = 0;
err:
    if (if_ni != NULL) {
        if_freenameindex(if_ni);
    }
    return rc;
}
#endif
static int32_t mcast_server_send(socket_t sck, const char *dst_addr, uint16_t dst_port, uint8_t *pbuff, uint32_t blen) {
    int32_t ret = 0;
    ssize_t slen = 0;
    struct sockaddr_in dstaddr = {0};

    if ((sck < 0) || (dst_addr == NULL) || (dst_port == 0) || (pbuff == NULL) || (blen == 0)) {
        LOG_ERROR("Invalid args!");
        ret = -1;
        goto err;
    }

    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_port = htons(dst_port);

    ret = inet_pton(AF_INET, dst_addr, &(dstaddr.sin_addr));
    if (ret <= 0) {
        LOG_ERROR("Invalid dst addr [%s], [%d] [%d:%s]!", dst_addr, ret, errno, strerror(errno));
        goto err;
    }

    if ((dstaddr.sin_addr.s_addr == INADDR_ANY) || (dstaddr.sin_port == 0)) {
        LOG_ERROR("Invalid dstaddr!");
        ret = -1;
        goto err;
    }

    slen = sendto(sck, pbuff, blen, 0, (const struct sockaddr *)&dstaddr, sizeof(dstaddr));
    if (slen != blen) {
        LOG_ERROR("Failed sendto buff len [%u:%lu] to [%s] failed!", blen, slen, dst_addr);
        ret = slen;
        goto err;
    }

    LOG_DEBUG("Send buff len [%u] to [%s] success!", blen, dst_addr);

    ret = 0;
err:
    return ret;
}

static int32_t print_run_progress(uint32_t total, uint32_t current) {

    uint32_t progressBar = 0, i = 0;
    char progress_bar[101] = {0};

    if ((total == 0) || (current == 0)) {
        return -1;
    }

    current -= 1;
    if (current == 0) {
        progressBar = 0;
        fprintf(stderr, NEW_LINE);
    } else {
        current *= 100;
        progressBar = ((current - 1) / total) + 1;
    }

    for (i = 0; (i < 100) && (i < progressBar); i++) {
        progress_bar[i] = '#';
    }
    fprintf(stderr, "[%-100s] [%3d%%]\r", progress_bar, progressBar);

#ifdef _USE_GUI_
    if (G_ARGS.S.uiProgressBar != NULL) {
        IupSetAttribute(G_ARGS.S.uiProgressBar, IUP_VALUE, UINT32TOSTR(progressBar));
    }
#endif // _USE_GUI_
    return 0;
}

static int32_t mcast_server_run(int32_t sck, const char *dst_addr, uint16_t dst_port, uint8_t *pbuff, uint32_t blen,
                                int32_t loop, uint32_t delay) {
    int32_t ret = 0;
    int32_t i = 0, loop_cnt = 0;
    uint32_t flen = 0, plen = 0;
    uint32_t fseq = 0, pseq = 0;
    uint8_t *pdata = NULL;
    mcupg_pkt_t pkt = {0};
    mcupg_pkthdr_t *mcpkt = &(pkt.header);
    bool run_loop = true;
    bool *prun_loop = &run_loop;

#ifdef _USE_GUI_
    prun_loop = &(G_ARGS.D.bThreadRun);
#endif

    if ((sck < 0) || (dst_addr == NULL) || (dst_port == 0) || (pbuff == NULL) || (blen == 0)) {
        LOG_ERROR("Invalid args!");
        ret = -1;
        goto err;
    }

    flen = blen;
    fseq = (flen / MCUPG_PACKET_LEN) + ((flen % MCUPG_PACKET_LEN) ? 1 : 0);

    while (*prun_loop) {
        if ((loop > 0) && (loop_cnt >= loop)) {
            break;
        }
        LOG_DEBUG("Loop [%d]", loop_cnt);
        loop_cnt++;
        pdata = pbuff;
        for (pseq = 1; (pseq <= fseq) && (*prun_loop); pseq++) {
            memset(&pkt, 0, sizeof(pkt));
            mcpkt->ui_len = flen;
            mcpkt->ui_seq = pseq;
            memcpy(mcpkt->ac_flag, MCUPG_FLAGS_STR, MCUPG_FLAGS_LEN);
            for (i = 0; i < MCUPG_PACKED_LEN; i++) {
                mcpkt->__packed__[i] = i + '0';
            }
            if (pseq == fseq) {
                mcpkt->ui_proto = MCUPG_PROTO_END;
            } else {
                mcpkt->ui_proto = MCUPG_PROTO_NUM;
            }

            if (mcpkt->ui_proto == MCUPG_PROTO_END) {
                plen = (flen % MCUPG_PACKET_LEN);
                plen = ((plen == 0) ? MCUPG_PACKET_LEN : plen);
            } else {
                plen = MCUPG_PACKET_LEN;
            }

            MCUPG_PKTHDR_BYTE_ORDER(mcpkt);

            memcpy(&(pkt.data), pdata, plen);
            pdata += plen;

            ret = mcast_server_send(sck, dst_addr, dst_port, (uint8_t *)&(pkt), sizeof(pkt));
            if (ret != 0) {
                LOG_ERROR("Failed to send pkt seq [%d], len [%d]!", pseq, plen);
                goto err;
            }
            LOG_DEBUG("Send pkt seq [%d:%d], len [%d:%d] success!", pseq, fseq, plen, (uint32_t)(sizeof(pkt)));
            print_run_progress(fseq, pseq);
            if (delay > 0) {
                sleep_ms(delay);
            }
        }
    }

    ret = 0;
err:
    return ret;
}

static void mcast_server_close(socket_t *psck) {
    if (psck == NULL) {
        return;
    }
#if (defined(WIN32))
    if (*psck != INVALID_SOCKET) {
        closesocket(*psck);
    }
    WSACleanup();
#else
    if (*psck >= 0) {
        close(*psck);
    }
#endif
}

#ifdef _USE_GUI_
static FILE *open_file(const char *fname) {
    FILE *fp = NULL;
    wchar_t *wname = NULL;
    int32_t wlen = 0;

    wlen = MultiByteToWideChar(CP_UTF8, 0, fname, -1, NULL, 0);
    if (wlen == 0) {
        goto error;
    }
    wname = (wchar_t *)malloc((wlen + 1) * sizeof(wchar_t));
    if (wname == NULL) {
        goto error;
    }

    if (MultiByteToWideChar(CP_UTF8, 0, fname, -1, wname, wlen) == 0) {
        goto error;
    }

    fp = _wfopen(wname, L"rb");
    if (fp == NULL) {
        goto error;
    }

error:
    if (wname != NULL) {
        free(wname);
    }
    return fp;
}
#else
#define open_file(fname) fopen(fname, "rb")
#endif

static int32_t read_file_to_buff(const char *file, uint8_t **fbuff, uint32_t *pflen) {
    int ret = 0;
    FILE *fp = NULL;
    uint8_t *pbuff = NULL;
    uint32_t fsize = 0;
    uint32_t rsize = 0;

    if ((file == NULL) || (fbuff == NULL) || (pflen == NULL)) {
        LOG_ERROR("Invalid file name!");
        ret = -1;
        goto err;
    }

    fp = open_file(file);
    if (fp == NULL) {
        LOG_ERROR("Failed to open file [%s]", file);
        ret = -1;
        goto err;
    }

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    if (fsize <= 0) {
        LOG_ERROR("Invalid file size [%u]!", fsize);
        ret = fsize;
        goto err;
    }
    fseek(fp, 0, SEEK_SET);

    pbuff = malloc(fsize);
    if (pbuff == NULL) {
        LOG_ERROR("Failed to malloc size [%u]!", fsize);
        ret = fsize;
        goto err;
    }
    memset(pbuff, 0, fsize);

    while ((rsize = fread(pbuff, 1, fsize, fp)) <= 0) {
        if (errno == EINTR || errno == EAGAIN) {
            errno = 0;
            continue;
        }
        break;
    }

    if (rsize != fsize) {
        LOG_ERROR("Failed to read file to buffer, size [%u:%u]!", rsize, fsize);
        ret = rsize;
        goto err;
    }

    LOG_DEBUG("Read file [%s] to addr [%p] size [%u]", file, pbuff, fsize);

    *fbuff = pbuff;
    *pflen = fsize;
    ret = 0;
err:
    if (fp != NULL) {
        fclose(fp);
    }
    if ((ret != 0) && (pbuff != NULL)) {
        free(pbuff);
    }
    return ret;
}

#ifndef _USE_GUI_
static void usage(const char *exe_name) {
    fprintf(stderr, "Usage: %s [options]\r\n", exe_name);
    fprintf(stderr, "Options:\r\n");
    fprintf(stderr, "\t-h,--help                           Show this help message.\r\n");
#if (defined(WIN32))
    fprintf(stderr, "\t-i <NUMBOR>,--index=<NUMBOR>        Interface index to use (see below).\r\n");
    print_iface_list();
#else
    fprintf(stderr, "\t-i <STRING>,--ifname=<STRING>       Interface name to use.\r\n");
#endif
    fprintf(stderr, "\t-d <STRING>,--dest=<STRING>         Dest multicast addr (Default %s).\r\n", DEF_MCAST_ADDR);
    fprintf(stderr, "\t-p <NUMBOR>,--dport=<NUMBOR>        Dest port (Default %d).\r\n", DEF_DST_PORT);
    fprintf(stderr, "\t-s <NUMBOR>,--sport=<NUMBOR>        Src port (Default %d).\r\n", DEF_SRC_PORT);
    fprintf(stderr, "\t-f <STRING>,--file=<STRING>         File name to send.\r\n");
    fprintf(stderr, "\t-t <NUMBOR>,--delay=<NUMBOR>        Time to wait (millisecond).\r\n");
    fprintf(stderr, "\t-l <NUMBOR>,--loop=<NUMBOR>         Loop count (-1 to loop forever).\r\n");

    fprintf(stderr, "Example:\r\n");
#if (defined(WIN32))
    fprintf(stderr, "\t%s -i 1 -f file.bin -t 10\r\n", exe_name);
    fprintf(stderr, "\t%s -i 1 -d %s -p %d -s %d -f file.bin -t 10\r\n", exe_name, DEF_MCAST_ADDR, DEF_DST_PORT,
            DEF_SRC_PORT);
#else
    fprintf(stderr, "\t%s -i eth0 -f file.bin -t 10\r\n", exe_name);
    fprintf(stderr, "\t%s -i eth0 -d %s -p %d -s %d -f file.bin -t 10\r\n", exe_name, DEF_MCAST_ADDR, DEF_DST_PORT,
            DEF_SRC_PORT);
#endif
}
#endif

#ifdef _USE_GUI_

static int onOpenFileClicked(Ihandle *self) {
    MCUPG_GLOBAL_T *pdat = &G_ARGS;
    Ihandle *filedlg = NULL;
    char *filename = NULL;

    filedlg = IupFileDlg();
    if (filedlg == NULL) {
        goto err;
    }
    IupSetAttribute(filedlg, IUP_DIALOGTYPE, IUP_OPEN);
    IupSetAttribute(filedlg, IUP_EXTFILTER, "Binary Files|*.img;*.bin|All Files|*.*|");

    IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(filedlg, IUP_STATUS) < 0) {
        LOG_ERROR("Failed to open file dialog!");
        goto err;
    }

    filename = IupGetAttribute(filedlg, IUP_VALUE);
    if (filename != NULL) {
        IupSetStrAttribute(pdat->S.uiFileLable, IUP_VALUE, filename);
        strncpy(pdat->D.fileName, filename, MCUPG_BUFF_LEN);
        LOG_DEBUG("FileName %s", pdat->D.fileName);
    } else {
        IupSetStrAttribute(pdat->S.uiFileLable, IUP_VALUE, "(cancelled)");
    }

err:
    if (filedlg != NULL) {
        IupDestroy(filedlg);
    }
    return IUP_DEFAULT;
}

static int onIfacesListSelected(Ihandle *self) {
    int32_t ifIndex = 0;
    iface_t *pIface = NULL;
    MCUPG_GLOBAL_T *pdat = &G_ARGS;

    ifIndex = atoi(IupGetAttribute(self, IUP_VALUE));
    if ((ifIndex <= 0) || (ifIndex > pdat->S.pIfaces.num)) {
        LOG_ERROR("Invalid ifIndex %d", ifIndex);
        return IUP_DEFAULT;
    }
    ifIndex -= 1;

    LOG_DEBUG("ifIndex %d", ifIndex);
    pIface = &(pdat->S.pIfaces.list[ifIndex]);
    LOG_DEBUG("pIface name %s", pIface->ac_name);

    memset(pdat->D.ifName, 0, MCUPG_BUFF_LEN + 1);
    strncpy(pdat->D.ifName, pIface->ac_name, MCUPG_BUFF_LEN);
    LOG_DEBUG("ifName %s", pdat->D.ifName);
    return IUP_DEFAULT;
}

static int onDelayTimeSelected(Ihandle *self) {
    MCUPG_GLOBAL_T *pdat = &G_ARGS;
    int32_t delayIndex = 0;
    delayIndex = atoi(IupGetAttribute(self, IUP_VALUE));
    if ((delayIndex <= 0) || (delayIndex > ARRAY_SIZE(G_DELAY_LIST))) {
        LOG_ERROR("Invalid Delay Time %d", delayIndex);
        return IUP_DEFAULT;
    }
    delayIndex -= 1;

    pdat->D.delayTime = atoi(G_DELAY_LIST[delayIndex]);
    LOG_DEBUG("delayTime %d", pdat->D.delayTime);
    return IUP_DEFAULT;
}

void globalArgsUnLoad(void) {
    LOG_DEBUG("unload start");
    if (G_ARGS.D.fileBuff != NULL) {
        free(G_ARGS.D.fileBuff);
    }
    mcast_server_close(&(G_ARGS.D.serverSck));
    memset(&(G_ARGS.D), 0, sizeof(G_ARGS.D));
    G_ARGS.D.bArgsLoaded = false;
    LOG_DEBUG("unload end");
}

int32_t globalArgsLoad(void) {
    int32_t ret = 0;
    char *filename = NULL;
    LOG_DEBUG("load start");

    if ((G_ARGS.S.uiMainWin == NULL) || (G_ARGS.S.uiStartBtn == NULL) || (G_ARGS.S.uiStopBtn == NULL) ||
        (G_ARGS.S.uiFileLable == NULL) || (G_ARGS.S.uiIfaceCombobox == NULL) || (G_ARGS.S.uiDelayCombobox == NULL) ||
        (G_ARGS.S.uiProgressBar == NULL) || (G_ARGS.S.pIfaces.num <= 0)) {
        LOG_ERROR("UI init error!");
        return -1;
    }

    globalArgsUnLoad();

    G_ARGS.D.destAddr = DEF_MCAST_ADDR;
    G_ARGS.D.destPort = DEF_DST_PORT;
    G_ARGS.D.srcPort = DEF_SRC_PORT;
    G_ARGS.D.delayTime = 0;
    G_ARGS.D.loopCnt = 1;
    G_ARGS.D.fileSize = 0;

    onIfacesListSelected(G_ARGS.S.uiIfaceCombobox);
    onDelayTimeSelected(G_ARGS.S.uiDelayCombobox);

    filename = IupGetAttribute(G_ARGS.S.uiFileLable, IUP_VALUE);
    if ((filename == NULL) || (strlen(filename) <= 0)) {
        LOG_ERROR("Invalid filename!");
        return -1;
    }
    strncpy(G_ARGS.D.fileName, filename, MCUPG_BUFF_LEN);
    ret = read_file_to_buff(G_ARGS.D.fileName, (uint8_t **)&(G_ARGS.D.fileBuff), &(G_ARGS.D.fileSize));
    if ((ret != 0) || (G_ARGS.D.fileBuff == NULL) || (G_ARGS.D.fileSize <= 0)) {
        LOG_ERROR("Failed to load file [%s]", G_ARGS.D.fileName);
        return ((ret == 0) ? -1 : ret);
    }

    ret = mcast_server_init(&(G_ARGS.D.serverSck), G_ARGS.D.ifName, G_ARGS.D.srcPort);
    if (ret != 0) {
        LOG_ERROR("Failed to init mcast server!");
        return ret;
    }
    G_ARGS.D.bArgsLoaded = true;
    LOG_DEBUG("load end");
    return 0;
}

static int onStartClicked(Ihandle *self) {
    MCUPG_GLOBAL_T *pdat = &G_ARGS;
    LOG_DEBUG("start!");
    if (globalArgsLoad() != 0) {
        LOG_ERROR("Failed to load args!");
        globalArgsUnLoad();
        return IUP_DEFAULT;
    }
    pdat->D.bThreadRun = true;
    LOG_DEBUG("end!");
    return IUP_DEFAULT;
}

static int onEndClicked(Ihandle *self) {
    LOG_DEBUG("start!");
    MCUPG_GLOBAL_T *pdat = &G_ARGS;
    pdat->D.bThreadRun = false;
    LOG_DEBUG("end!");
    return IUP_DEFAULT;
}

static int setButtonStatus(bool enable) {
    if (enable) {
        IupSetAttribute(G_ARGS.S.uiStartBtn, IUP_ACTIVE, IUP_NO);
        IupSetAttribute(G_ARGS.S.uiStopBtn, IUP_ACTIVE, IUP_YES);
    } else {
        IupSetAttribute(G_ARGS.S.uiStartBtn, IUP_ACTIVE, IUP_YES);
        IupSetAttribute(G_ARGS.S.uiStopBtn, IUP_ACTIVE, IUP_NO);
    }
}

DWORD WINAPI McupgServerRun(LPVOID lpProgram) {
    int32_t ret = 0;
    while (true) {
        if (G_ARGS.D.bThreadRun) {
            setButtonStatus(true);
            ret = mcast_server_run(G_ARGS.D.serverSck, G_ARGS.D.destAddr, G_ARGS.D.destPort, G_ARGS.D.fileBuff,
                                   G_ARGS.D.fileSize, G_ARGS.D.loopCnt, G_ARGS.D.delayTime);
            if (ret != 0) {
                LOG_ERROR("Failed to send file [%s], %d!", G_ARGS.D.fileName, ret);
                globalArgsUnLoad();
            }
        } else {
            setButtonStatus(false);
        }
        Sleep(10);
    }
}
#endif

int main(int argc, char *argv[]) {
    int ret = 0;
#ifndef _USE_GUI_
    int opt = 0, opt_index = 0;
#else
    Ihandle *lblf;
    Ihandle *btnf;
    Ihandle *lsti;
    Ihandle *frmi;
    Ihandle *lstd;
    Ihandle *frmd;
    Ihandle *btns;
    Ihandle *btne;
    Ihandle *prgb;
    Ihandle *ver;

    Ihandle *hbox1;
    Ihandle *hbox2;
    Ihandle *hbox3;
    Ihandle *hbox4;
    Ihandle *hbox5;

    Ihandle *vbox;
    Ihandle *dlg;

    DWORD dwMcupgRunTId;
    HANDLE hMcupgRunT;
#endif
    socket_t sck = SOCKET_INITIALIZE;
    char *ifname = NULL;
    char *file = NULL;
    char *dest_addr = NULL;
    int32_t dest_port = 0;
    int32_t src_port = 0;
    int32_t delay = 0;
    int32_t loop = 0;

    uint8_t *pbuff = NULL;
    uint32_t fsize = 0;

#ifdef _USE_GUI_
    uint32_t idx = 0;
    ifaces_t *ifaces = &(G_ARGS.S.pIfaces);

    ret = get_iface_list(ifaces);
    if (ret != 0) {
        LOG_ERROR("Failed to get iface list!");
        goto err;
    }
#endif

#ifndef _USE_GUI_
    // clang-format off
    static struct option long_options[] = { {"help", no_argument, 0, 'h'},
                                           {"dest", required_argument, 0, 'd'},
                                           {"dport", required_argument, 0, 'p'},
#if (defined(WIN32))
                                           {"index", required_argument, 0, 'i'},
#else
                                           {"ifname", required_argument, 0, 'i'},
#endif
                                           {"sport", required_argument, 0, 's'},
                                           {"file", required_argument, 0, 'f'},
                                           {"delay", required_argument, 0, 't'},
                                           {"loop", required_argument, 0, 'l'},
                                           {0, 0, 0, 0} };
    // clang-format on

    while ((opt = getopt_long(argc, argv, "d:p:i:s:f:t:l:h", long_options, &opt_index)) != -1) {
        switch (opt) {
            case 0:
                if (strcmp("dest", long_options[opt_index].name) == 0) {
                    dest_addr = optarg;
                }
                if (strcmp("dport", long_options[opt_index].name) == 0) {
                    dest_port = atol(optarg);
                }
#if (defined(WIN32))
                if (strcmp("index", long_options[opt_index].name) == 0) {
                    ifname = optarg;
                }
#else
                if (strcmp("ifname", long_options[opt_index].name) == 0) {
                    ifname = optarg;
                }
#endif
                if (strcmp("sport", long_options[opt_index].name) == 0) {
                    src_port = atol(optarg);
                }
                if (strcmp("file", long_options[opt_index].name) == 0) {
                    file = optarg;
                }
                if (strcmp("delay", long_options[opt_index].name) == 0) {
                    delay = atol(optarg);
                }
                if (strcmp("loop", long_options[opt_index].name) == 0) {
                    loop = atol(optarg);
                }
                break;
            case 'd':
                dest_addr = optarg;
                break;
            case 'p':
                dest_port = atol(optarg);
                break;
            case 'i':
                ifname = optarg;
                break;
            case 's':
                src_port = atol(optarg);
                break;
            case 'f':
                file = optarg;
                break;
            case 't':
                delay = atol(optarg);
                break;
            case 'l':
                loop = atol(optarg);
                break;
            case 'h':
                ret = 1;
                goto err;
            default:
                LOG_ERROR("Unknown option -- %c", opt);
                ret = 1;
                goto err;
        }
    }
#else
    IupOpen(&argc, &argv);

    btnf = IupButton("Open File", NULL);
    IupSetAttribute(btnf, IUP_EXPAND, IUP_YES);
    IupSetCallback(btnf, IUP_ACTION, (Icallback)onOpenFileClicked);

    lblf = IupText(NULL);
    IupSetAttribute(lblf, IUP_EXPAND, IUP_YES);
    G_ARGS.S.uiFileLable = lblf;

    hbox1 = IupHbox(btnf, lblf, NULL);
    // IupSetAttribute(hbox1, IUP_SIZE, "100");
    // IupSetAttribute(hbox1, IUP_GAP, "10");
    // IupSetAttribute(hbox1, IUP_MARGIN, "10x10");

    lsti = IupList(NULL);
    IupSetAttribute(lsti, IUP_EXPAND, IUP_YES);
    IupSetAttribute(lsti, IUP_DROPDOWN, IUP_YES);
    IupSetAttribute(lsti, IUP_VALUE, "1");
    IupSetCallback(lsti, IUP_VALUECHANGED_CB, (Icallback)onIfacesListSelected);
    for (idx = 0; idx < ifaces->num; idx++) {
        // LOG_ERROR("iface[%d]: %s, %s", idx, ifaces->list[idx].ac_name, ifaces->list[idx].ac_ipaddr);
        IupSetAttribute(lsti, UINT32TOSTR(idx + 1), ifaces->list[idx].ac_ipaddr);
    }
    G_ARGS.S.uiIfaceCombobox = lsti;

    frmi = IupFrame(IupVbox(lsti, NULL));
    IupSetAttribute(frmi, IUP_EXPAND, IUP_YES);
    IupSetAttribute(frmi, IUP_TITLE, "Interface List");

    lstd = IupList(NULL);
    IupSetAttribute(lstd, IUP_EXPAND, IUP_YES);
    IupSetAttribute(lstd, IUP_DROPDOWN, IUP_YES);
    IupSetAttribute(lstd, IUP_VALUE, "2");
    IupSetCallback(lstd, IUP_VALUECHANGED_CB, (Icallback)onDelayTimeSelected);
    for (idx = 0; idx < (ARRAY_SIZE(G_DELAY_LIST)); idx++) {
        // LOG_ERROR("delay[%s]: %s", UINT32TOSTR(idx + 1), (G_DELAY_LIST[idx]));
        IupSetAttribute(lstd, UINT32TOSTR(idx + 1), (G_DELAY_LIST[idx]));
    }
    G_ARGS.S.uiDelayCombobox = lstd;

    frmd = IupFrame(IupVbox(lstd, NULL));
    IupSetAttribute(frmd, IUP_EXPAND, IUP_YES);
    IupSetAttribute(frmd, IUP_TITLE, "Delay Time");

    hbox2 = IupHbox(frmi, frmd, NULL);
    // IupSetAttribute(hbox2, IUP_SIZE, "100");
    // IupSetAttribute(hbox2, IUP_GAP, "10");
    // IupSetAttribute(hbox2, IUP_MARGIN, "10x10");

    prgb = IupProgressBar();
    IupSetAttribute(prgb, IUP_EXPAND, IUP_YES);
    // IupSetAttribute(prgb, IUP_DASHED, IUP_YES);
    IupSetAttribute(prgb, IUP_MAX, "100");
    IupSetAttribute(prgb, IUP_VALUE, "0");
    G_ARGS.S.uiProgressBar = prgb;

    hbox3 = IupHbox(prgb, NULL);
    // IupSetAttribute(hbox3, IUP_SIZE, "100");
    // IupSetAttribute(hbox3, IUP_GAP, "10");
    // IupSetAttribute(hbox3, IUP_MARGIN, "10x10");

    btns = IupButton("Start", NULL);
    IupSetAttribute(btns, IUP_EXPAND, IUP_YES);
    IupSetCallback(btns, IUP_ACTION, (Icallback)onStartClicked);
    G_ARGS.S.uiStartBtn = btns;

    btne = IupButton("End", NULL);
    IupSetAttribute(btne, IUP_EXPAND, IUP_YES);
    IupSetCallback(btne, IUP_ACTION, (Icallback)onEndClicked);
    G_ARGS.S.uiStopBtn = btne;

    hbox4 = IupHbox(btns, btne, NULL);
    // IupSetAttribute(hbox4, IUP_SIZE, "100");
    // IupSetAttribute(hbox4, IUP_GAP, "10");
    // IupSetAttribute(hbox4, IUP_MARGIN, "10x10");

    ver = IupLabel(MCUPG_UI_VERSION);

    hbox5 = IupHbox(ver, NULL);
    // IupSetAttribute(hbox5, IUP_SIZE, "100");
    // IupSetAttribute(hbox5, IUP_GAP, "10");
    // IupSetAttribute(hbox5, IUP_MARGIN, "10x10");

    vbox = IupVbox(hbox1, hbox2, hbox3, hbox4, hbox5, NULL);
    IupSetAttribute(vbox, IUP_ALIGNMENT, IUP_ACENTER);
    IupSetAttribute(vbox, IUP_GAP, "5");
    IupSetAttribute(vbox, IUP_MARGIN, "5x5");

    dlg = IupDialog(vbox);
    IupSetAttribute(dlg, IUP_TITLE, "McupgServer");
    IupSetAttribute(dlg, IUP_SIZE, "480x");
    IupSetAttribute(dlg, IUP_RESIZE, IUP_NO);
    IupSetAttribute(dlg, IUP_MINBOX, IUP_NO);
    IupSetAttribute(dlg, IUP_MAXBOX, IUP_NO);
    // IupSetAttribute(dlg, IUP_MENUBOX, IUP_NO);
    IupSetAttribute(dlg, IUP_BORDER, IUP_NO);

    G_ARGS.S.uiMainWin = dlg;

    hMcupgRunT = CreateThread(NULL, 0, McupgServerRun, NULL, 0, &dwMcupgRunTId);
    if (hMcupgRunT == NULL) {
        LOG_ERROR("Failed to create McupgServerRun thread!");
        ret = -1;
        goto err;
    }
#endif

#ifndef _USE_GUI_
    if (file == NULL) {
        LOG_ERROR("Invalid file name!");
        ret = -1;
        goto err;
    }

    if (dest_addr == NULL) {
        dest_addr = DEF_MCAST_ADDR;
    }

    if (dest_port <= 0) {
        dest_port = DEF_DST_PORT;
    }

    if (src_port <= 0) {
        src_port = DEF_SRC_PORT;
    }

    if (delay <= 0) {
        delay = 0;
    }

    if (loop <= 0) {
        loop = -1;
    }

    LOG_DEBUG("file[%s], ifname[%s], dest_addr[%s], dest_port[%d], src_port[%d], delay[%d], loop[%d]", file,
              ((ifname == NULL) ? "NULL" : ifname), dest_addr, dest_port, src_port, delay, loop);

    ret = mcast_server_init(&sck, ifname, src_port);
    if (ret != 0) {
        LOG_DEBUG("Failed to init mcast server!");
        goto err;
    }

    ret = read_file_to_buff(file, &pbuff, &fsize);
    if (ret != 0) {
        LOG_ERROR("Failed to read file [%s] to buff!", file);
        goto err;
    }

    ret = mcast_server_run(sck, dest_addr, dest_port, pbuff, fsize, loop, delay);
    if (ret != 0) {
        LOG_ERROR("Failed to send file [%s], %d!", file, ret);
        goto err;
    }
#else
    IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
    IupMainLoop();
#endif

err:
#ifndef _USE_GUI_
    fprintf(stderr, NEW_LINE);
    mcast_server_close(&sck);
    if (pbuff != NULL) {
        free(pbuff);
    }
    if (ret != 0) {
        usage(argv[0]);
    }
#else
    globalArgsUnLoad();
    IupClose();
#endif
    return ret;
}
