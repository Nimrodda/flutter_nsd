
#include <stdio.h>



#include <winsock2.h>
#include <iphlpapi.h>
#define sleep(x) Sleep(x * 1000)

//#if (_DEBUG == 0)
#define printf
//#endif

#include "mdns.h"

#include "mdns_impl.h"


static char addrbuffer[64];
static char entrybuffer[256];
static char namebuffer[256];
static char sendbuffer[1024];
static mdns_record_txt_t txtbuffer[128];

static struct sockaddr_in service_address_ipv4;
static struct sockaddr_in6 service_address_ipv6;

static int has_ipv4;
static int has_ipv6;

// Data for our service including the mDNS records
typedef struct {
  mdns_string_t service;
  mdns_string_t hostname;
  mdns_string_t service_instance;
  mdns_string_t hostname_qualified;
  struct sockaddr_in address_ipv4;
  struct sockaddr_in6 address_ipv6;
  int port;
  mdns_record_t record_ptr;
  mdns_record_t record_srv;
  mdns_record_t record_a;
  mdns_record_t record_aaaa;
  mdns_record_t txt_record[2];
} service_t;

static mdns_string_t
ipv4_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in* addr,
  size_t addrlen) {
  char host[NI_MAXHOST] = { 0 };
  char service[NI_MAXSERV] = { 0 };
  int ret = getnameinfo((const struct sockaddr*)addr, (socklen_t)addrlen, host, NI_MAXHOST,
    service, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
  int len = 0;
  if (ret == 0) {
    if (addr->sin_port != 0)
      len = snprintf(buffer, capacity, "%s:%s", host, service);
    else
      len = snprintf(buffer, capacity, "%s", host);
  }
  if (len >= (int)capacity)
    len = (int)capacity - 1;
  mdns_string_t str;
  str.str = buffer;
  str.length = len;
  return str;
}

static mdns_string_t
ipv6_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in6* addr,
  size_t addrlen) {
  char host[NI_MAXHOST] = { 0 };
  char service[NI_MAXSERV] = { 0 };
  int ret = getnameinfo((const struct sockaddr*)addr, (socklen_t)addrlen, host, NI_MAXHOST,
    service, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
  int len = 0;
  if (ret == 0) {
    if (addr->sin6_port != 0)
      len = snprintf(buffer, capacity, "[%s]:%s", host, service);
    else
      len = snprintf(buffer, capacity, "%s", host);
  }
  if (len >= (int)capacity)
    len = (int)capacity - 1;
  mdns_string_t str;
  str.str = buffer;
  str.length = len;
  return str;
}

static mdns_string_t
ip_address_to_string(char* buffer, size_t capacity, const struct sockaddr* addr, size_t addrlen) {
  if (addr->sa_family == AF_INET6)
    return ipv6_address_to_string(buffer, capacity, (const struct sockaddr_in6*)addr, addrlen);
  return ipv4_address_to_string(buffer, capacity, (const struct sockaddr_in*)addr, addrlen);
}

// Callback handling parsing answers to queries sent
static int
query_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
  uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
  size_t size, size_t name_offset, size_t name_length, size_t record_offset,
  size_t record_length, void* plugin) {
  (void)sizeof(sock);
  (void)sizeof(query_id);
  (void)sizeof(name_length);
  (void)sizeof(plugin);
  boolean last = FALSE;


  mdns_string_t fromaddrstr = ip_address_to_string(addrbuffer, sizeof(addrbuffer), from, addrlen);
  const char* entrytype = (entry == MDNS_ENTRYTYPE_ANSWER) ?
    "answer" :
    ((entry == MDNS_ENTRYTYPE_AUTHORITY) ? "authority" : "additional");
  mdns_string_t entrystr =
    mdns_string_extract(data, size, &name_offset, entrybuffer, sizeof(entrybuffer));
  if (record_offset + record_length >= size) {
    printf("last record in this packet\n\r");
    last = TRUE;
  }
  if (rtype == MDNS_RECORDTYPE_PTR) {
    mdns_string_t namestr = mdns_record_parse_ptr(data, size, record_offset, record_length,
      namebuffer, sizeof(namebuffer));
    printf("%.*s : %s %.*s PTR %.*s rclass 0x%x ttl %u length %d\n",
      MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr),
      MDNS_STRING_FORMAT(namestr), rclass, ttl, (int)record_length);
    call_HandlePTRRecord(plugin, data, last,
      MDNS_STRING_ARGS(entrystr), MDNS_STRING_ARGS(namestr));
  }
  else if (rtype == MDNS_RECORDTYPE_SRV) {
    mdns_record_srv_t srv = mdns_record_parse_srv(data, size, record_offset, record_length,
      namebuffer, sizeof(namebuffer));
    printf("%.*s : %s %.*s SRV %.*s priority %d weight %d port %d\n",
      MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr),
      MDNS_STRING_FORMAT(srv.name), srv.priority, srv.weight, srv.port);
    char hostname[256];
    strncpy_s(hostname, sizeof(hostname), MDNS_STRING_ARGS(srv.name));
    char servicename[256];
    strncpy_s(servicename, sizeof(servicename), MDNS_STRING_ARGS(entrystr));
    call_HandleSRVRecord(plugin, data, last, MDNS_STRING_ARGS(entrystr), MDNS_STRING_ARGS(srv.name), srv.port);
  }
  else if (rtype == MDNS_RECORDTYPE_A) {
    struct sockaddr_in addr;
    mdns_record_parse_a(data, size, record_offset, record_length, &addr);
    mdns_string_t addrstr =
      ipv4_address_to_string(namebuffer, sizeof(namebuffer), &addr, sizeof(addr));
    printf("%.*s : %s %.*s A %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
      MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(addrstr));
    call_HandleARecord(plugin, data, last,
      MDNS_STRING_ARGS(entrystr), MDNS_STRING_ARGS(addrstr));
  }
  else if (rtype == MDNS_RECORDTYPE_AAAA) {
    struct sockaddr_in6 addr;
    mdns_record_parse_aaaa(data, size, record_offset, record_length, &addr);
    mdns_string_t addrstr =
      ipv6_address_to_string(namebuffer, sizeof(namebuffer), &addr, sizeof(addr));
    printf("%.*s : %s %.*s AAAA %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
      MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(addrstr));
    call_HandleAAAARecord(plugin, data, last,
      MDNS_STRING_ARGS(entrystr), MDNS_STRING_ARGS(addrstr));
  }
  else if (rtype == MDNS_RECORDTYPE_TXT) {
    size_t parsed = mdns_record_parse_txt(data, size, record_offset, record_length, txtbuffer,
      sizeof(txtbuffer) / sizeof(mdns_record_txt_t));
    for (size_t itxt = 0; itxt < parsed; ++itxt) {
      if (txtbuffer[itxt].value.length) {
        printf("%.*s : %s %.*s TXT %.*s = %.*s\n", MDNS_STRING_FORMAT(fromaddrstr),
          entrytype, MDNS_STRING_FORMAT(entrystr),
          MDNS_STRING_FORMAT(txtbuffer[itxt].key),
          MDNS_STRING_FORMAT(txtbuffer[itxt].value));

      }
      else {
        printf("%.*s : %s %.*s TXT %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
          MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(txtbuffer[itxt].key));
      }
      call_HandleTXTRecord(plugin, data, last && itxt == parsed -1, MDNS_STRING_ARGS(txtbuffer[itxt].key),
        MDNS_STRING_ARGS(txtbuffer[itxt].value));
    }
  }
  else {
    printf("%.*s : %s %.*s type %u rclass 0x%x ttl %u length %d\n",
      MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr), rtype,
      rclass, ttl, (int)record_length);
    call_HandleURecord(plugin, data, last);
  }

  return 0;
}



// Open sockets for sending one-shot multicast queries from an ephemeral port
static int
open_client_sockets(int* sockets, int max_sockets, int port) {
  // When sending, each socket can only send to one network interface
  // Thus we need to open one socket for each interface and address family
  int num_sockets = 0;

  IP_ADAPTER_ADDRESSES* adapter_address = 0;
  ULONG address_size = 8000;
  unsigned int ret;
  unsigned int num_retries = 4;
  do {
    adapter_address = (IP_ADAPTER_ADDRESSES*)malloc(address_size);
    ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST, 0,
      adapter_address, &address_size);
    if (ret == ERROR_BUFFER_OVERFLOW) {
      free(adapter_address);
      adapter_address = 0;
      address_size *= 2;
    }
    else {
      break;
    }
  } while (num_retries-- > 0);

  if (!adapter_address || (ret != NO_ERROR)) {
    free(adapter_address);
    printf("Failed to get network adapter addresses\n");
    return num_sockets;
  }

  int first_ipv4 = 1;
  int first_ipv6 = 1;
  for (PIP_ADAPTER_ADDRESSES adapter = adapter_address; adapter; adapter = adapter->Next) {
    if (adapter->TunnelType == TUNNEL_TYPE_TEREDO)
      continue;
    if (adapter->OperStatus != IfOperStatusUp)
      continue;

    for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast;
      unicast = unicast->Next) {
      if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
        struct sockaddr_in* saddr = (struct sockaddr_in*)unicast->Address.lpSockaddr;
        if ((saddr->sin_addr.S_un.S_un_b.s_b1 != 127) ||
          (saddr->sin_addr.S_un.S_un_b.s_b2 != 0) ||
          (saddr->sin_addr.S_un.S_un_b.s_b3 != 0) ||
          (saddr->sin_addr.S_un.S_un_b.s_b4 != 1)) {
          int log_addr = 0;
          if (first_ipv4) {
            service_address_ipv4 = *saddr;
            first_ipv4 = 0;
            log_addr = 1;
          }
          has_ipv4 = 1;
          if (num_sockets < max_sockets) {
            saddr->sin_port = htons((unsigned short)port);
            int sock = mdns_socket_open_ipv4(saddr);
            if (sock >= 0) {
              sockets[num_sockets++] = sock;
              log_addr = 1;
            }
            else {
              log_addr = 0;
            }
          }
          if (log_addr) {
            char buffer[128];
            mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,
              sizeof(struct sockaddr_in));
            printf("Local IPv4 address: %.*s\n", MDNS_STRING_FORMAT(addr));
          }
        }
      }
      else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
        struct sockaddr_in6* saddr = (struct sockaddr_in6*)unicast->Address.lpSockaddr;
        static const unsigned char localhost[] = { 0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0, 0, 0, 0, 0, 0, 1 };
        static const unsigned char localhost_mapped[] = { 0, 0, 0,    0,    0,    0, 0, 0,
                                                         0, 0, 0xff, 0xff, 0x7f, 0, 0, 1 };
        if ((unicast->DadState == NldsPreferred) &&
          memcmp(saddr->sin6_addr.s6_addr, localhost, 16) &&
          memcmp(saddr->sin6_addr.s6_addr, localhost_mapped, 16)) {
          int log_addr = 0;
          if (first_ipv6) {
            service_address_ipv6 = *saddr;
            first_ipv6 = 0;
            log_addr = 1;
          }
          has_ipv6 = 1;
          if (num_sockets < max_sockets) {
            saddr->sin6_port = htons((unsigned short)port);
            int sock = mdns_socket_open_ipv6(saddr);
            if (sock >= 0) {
              sockets[num_sockets++] = sock;
              log_addr = 1;
            }
            else {
              log_addr = 0;
            }
          }
          if (log_addr) {
            char buffer[128];
            mdns_string_t addr = ipv6_address_to_string(buffer, sizeof(buffer), saddr,
              sizeof(struct sockaddr_in6));
            printf("Local IPv6 address: %.*s\n", MDNS_STRING_FORMAT(addr));
          }
        }
      }
    }
  }

  free(adapter_address);


  return num_sockets;
}



// Open sockets to listen to incoming mDNS queries on port 5353
static int
open_service_sockets(int* sockets, int max_sockets) {
  // When recieving, each socket can recieve data from all network interfaces
  // Thus we only need to open one socket for each address family
  int num_sockets = 0;

  // Call the client socket function to enumerate and get local addresses,
  // but not open the actual sockets
  open_client_sockets(0, 0, 0);

  if (num_sockets < max_sockets) {
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr.sin_family = AF_INET;
#ifdef _WIN32
    sock_addr.sin_addr = in4addr_any;
#else
    sock_addr.sin_addr.s_addr = INADDR_ANY;
#endif
    sock_addr.sin_port = htons(MDNS_PORT);
#ifdef __APPLE__
    sock_addr.sin_len = sizeof(struct sockaddr_in);
#endif
    int sock = mdns_socket_open_ipv4(&sock_addr);
    if (sock >= 0)
      sockets[num_sockets++] = sock;
  }

  if (num_sockets < max_sockets) {
    struct sockaddr_in6 sock_addr;
    memset(&sock_addr, 0, sizeof(struct sockaddr_in6));
    sock_addr.sin6_family = AF_INET6;
    sock_addr.sin6_addr = in6addr_any;
    sock_addr.sin6_port = htons(MDNS_PORT);
#ifdef __APPLE__
    sock_addr.sin6_len = sizeof(struct sockaddr_in6);
#endif
    int sock = mdns_socket_open_ipv6(&sock_addr);
    if (sock >= 0)
      sockets[num_sockets++] = sock;
  }

  return num_sockets;
}



static int
send_mdns_query(const char* service, int record, const void *plugin) {
  int sockets[32];
  int query_id[32];
  int num_sockets = open_client_sockets(sockets, sizeof(sockets) / sizeof(sockets[0]), 0);
  if (num_sockets <= 0) {
    printf("Failed to open any client sockets\n");
    return -1;
  }
  printf("Opened %d socket%s for mDNS query\n", num_sockets, num_sockets ? "s" : "");

  size_t capacity = 2048;
  void* buffer = malloc(capacity);

  size_t records;

  const char* record_name = "PTR";
  if (record == MDNS_RECORDTYPE_SRV)
    record_name = "SRV";
  else if (record == MDNS_RECORDTYPE_A)
    record_name = "A";
  else if (record == MDNS_RECORDTYPE_AAAA)
    record_name = "AAAA";
  else
    record = MDNS_RECORDTYPE_PTR;

  printf("Sending mDNS query: %s %s\n", service, record_name);
  for (int isock = 0; isock < num_sockets; ++isock) {
    query_id[isock] =
      mdns_query_send(sockets[isock], record, service, strlen(service), buffer, capacity, 0);
    if (query_id[isock] < 0)
      printf("Failed to send mDNS query: %d\n", errno);
  }

  // This is a simple implementation that loops for 5 seconds or as long as we get replies
  int res;
  printf("Reading mDNS query replies\n");
  do {
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    int nfds = 0;
    fd_set readfs;
    FD_ZERO(&readfs);
    for (int isock = 0; isock < num_sockets; ++isock) {
      if (sockets[isock] >= nfds)
        nfds = sockets[isock] + 1;
      FD_SET(sockets[isock], &readfs);
    }

    records = 0;
    res = select(nfds, &readfs, 0, 0, &timeout);
    if (res > 0) {
      for (int isock = 0; isock < num_sockets; ++isock) {
        if (FD_ISSET(sockets[isock], &readfs)) {
          records += mdns_query_recv(sockets[isock], buffer, capacity, query_callback,
            (void *)plugin, query_id[isock]);
        }
        FD_SET(sockets[isock], &readfs);
      }
    }
  } while (res > 0);

  free(buffer);

  for (int isock = 0; isock < num_sockets; ++isock)
    mdns_socket_close(sockets[isock]);
  printf("Closed socket%s\n", num_sockets ? "s" : "");

  return 0;
}





int mdns_query(const void *plugin, const char *service_name)
{
  return send_mdns_query(service_name, MDNS_RECORDTYPE_PTR, plugin);
}

