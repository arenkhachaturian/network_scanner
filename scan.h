#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "config.h"

// Static string maximum length
#undef STRL
#define STRL 1000

// Thread number
#define TNUM 255

// Default values
#define SUBNET "172.24.112.0/20"
#define PORTSFILE "ports.config"
#define HOSTLIST 256

// Subnet mask (type)
typedef struct mask_t {
  char ip[INET_ADDRSTRLEN];
  int len;
  uint32_t network;
  uint32_t bandwidth;
} mask_t;

// Host (type)
typedef struct host_t {
  char ip[INET_ADDRSTRLEN];
  bool pl[PORTLIST_LEN];
} host_t;

// Host list (type)
typedef struct hostlist_t {
  int len;
  host_t data[HOSTLIST];
} hostlist_t;

// network (type)
typedef struct net_t {
  mask_t mask;
  portlist_t pl;
  hostlist_t h;
} net_t;