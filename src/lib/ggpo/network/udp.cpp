/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "types.h"
#include "udp.h"

#ifdef _WIN32
SOCKET
CreateSocket(uint16 bind_port, int retries)
{
   SOCKET s;
   sockaddr_in sin;
   uint16 port;
   int optval = 1;

   s = socket(AF_INET, SOCK_DGRAM, 0);
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof optval);
   setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char *)&optval, sizeof optval);

   // non-blocking...
   u_long iMode = 1;
   ioctlsocket(s, FIONBIO, &iMode);

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   for (port = bind_port; port <= bind_port + retries; port++) {
      sin.sin_port = htons(port);
      if (bind(s, (sockaddr *)&sin, sizeof sin) != SOCKET_ERROR) {
         Log("Udp bound to port: %d.\n", port);
         return s;
      }
   }
   closesocket(s);
   return INVALID_SOCKET;
}

Udp::~Udp(void)
{
   if (_socket != INVALID_SOCKET) {
      closesocket(_socket);
      _socket = INVALID_SOCKET;
   }
}


#else

SOCKET CreateSocket(uint16_t bind_port, int retries) {
    SOCKET s;
    sockaddr_in sin;
    uint16_t port;
    int optval = 1;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct linger linger_opt;
    linger_opt.l_onoff = 0;
    linger_opt.l_linger = 0;
    setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*)&linger_opt, sizeof(linger_opt));

    // non-blocking...
    int iMode = 1;
    ioctl(s, FIONBIO, &iMode);

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    for (port = bind_port; port <= bind_port + retries; port++) {
        sin.sin_port = htons(port);
        if (bind(s, (sockaddr*)&sin, sizeof(sin)) != -1) {
            printf("Udp bound to port: %d.\n", port);
            return s;
        }
    }
    close(s);
    return -1;
}

Udp::~Udp() {
    close(_socket);
}
#endif

Udp::Udp() :
   _socket(INVALID_SOCKET),
   _callbacks(NULL)
{
}

void
Udp::Init(uint16 port, Poll *poll, Callbacks *callbacks)
{
   _callbacks = callbacks;

   _poll = poll;
   _poll->RegisterLoop(this);

   Log("binding udp socket to port %d.\n", port);
   _socket = CreateSocket(port, 0);
}

#ifdef _WIN32
void
Udp::SendTo(char *buffer, int len, int flags, struct sockaddr *dst, int destlen)
{
   struct sockaddr_in *to = (struct sockaddr_in *)dst;

   int res = sendto(_socket, buffer, len, flags, dst, destlen);
   if (res == SOCKET_ERROR) {
      DWORD err = WSAGetLastError();
      Log("unknown error in sendto (erro: %d  wsaerr: %d).\n", res, err);
      ASSERT(FALSE && "Unknown error in sendto");
   }
   char dst_ip[1024];
   Log("sent packet length %d to %s:%d (ret:%d).\n", len, inet_ntop(AF_INET, (void *)&to->sin_addr, dst_ip, ARRAY_SIZE(dst_ip)), ntohs(to->sin_port), res);
}
#else
void Udp::SendTo(char* buffer, int len, int flags, struct sockaddr* dst, int destlen) {
    struct sockaddr_in* to = (struct sockaddr_in*)dst;

    int res = sendto(_socket, buffer, len, flags, dst, destlen);
    if (res == -1) {
        perror("sendto");
        // Handle send error
        return;
    }
    
    char dst_ip[INET_ADDRSTRLEN];
    const char* ip_str = inet_ntop(AF_INET, &(to->sin_addr), dst_ip, INET_ADDRSTRLEN);
    if (ip_str == NULL) {
        perror("inet_ntop");
        // Handle IP conversion error
        return;
    }

    printf("sent packet length %d to %s:%d (ret:%d).\n", len, ip_str, ntohs(to->sin_port), res);
}
#endif

bool
Udp::OnLoopPoll(void *cookie)
{
   uint8          recv_buf[MAX_UDP_PACKET_SIZE];
   sockaddr_in    recv_addr;
   int            recv_addr_len;

   for (;;) {
      recv_addr_len = sizeof(recv_addr);
      #ifdef _WIN32
      int len = recvfrom(_socket, (char *)recv_buf, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
      #else
      int len = recvfrom(_socket, (char *)recv_buf, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr *)&recv_addr, (socklen_t*)&recv_addr_len);
      #endif


      // TODO: handle len == 0... indicates a disconnect.

      if (len == -1) {
         #ifdef _WIN32
         int error = WSAGetLastError();
         if (error != WSAEWOULDBLOCK) {
         #else
         int error = errno;
         if(error != EWOULDBLOCK) {
         #endif
            Log("recvfrom WSAGetLastError returned %d (%x).\n", error, error);
         }
         break;
      } else if (len > 0) {
         char src_ip[1024];
         Log("recvfrom returned (len:%d  from:%s:%d).\n", len, inet_ntop(AF_INET, (void*)&recv_addr.sin_addr, src_ip, ARRAY_SIZE(src_ip)), ntohs(recv_addr.sin_port) );
         UdpMsg *msg = (UdpMsg *)recv_buf;
         _callbacks->OnMsg(recv_addr, msg, len);
      } 
   }
   return true;
}


void
Udp::Log(const char *fmt, ...)
{
   char buf[1024];
   size_t offset;
   va_list args;

   #ifdef _WIN32
   strcpy_s(buf, "udp | ");
   #else
   strcpy(buf, "udp | ");
   #endif
   offset = strlen(buf);
   va_start(args, fmt);
   vsnprintf(buf + offset, ARRAY_SIZE(buf) - offset - 1, fmt, args);
   buf[ARRAY_SIZE(buf)-1] = '\0';
   ::Log(buf);
   va_end(args);
}
