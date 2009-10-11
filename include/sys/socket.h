#include <network.h>

#define select net_select
#define socket(x,y,z) net_socket(x,y,z)
#define connect net_connect
#define setsockopt net_setsockopt
#define bind net_bind
#define write net_write
#define send net_send
#define listen net_listen
#define accept net_accept
#define gethostbyname net_gethostbyname
#define getpeername net_getpeername
#define getsockname net_getsockname
#define wait3 net_wait3

extern int errno;

