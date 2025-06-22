/**
 * @file scan.c
 *
 * @brief Scanning network with subnet mask
 */
#include "scan.h"

// Calculate mask from input arguments or use default
static void _get_mask(mask_t* mask, int argc, char** argv) {
  char mask_tmpl[STRL] = {'\0'};
  strcpy(mask_tmpl, (argc < 2) ? SUBNET : argv[1]);

  char* p = strtok(mask_tmpl, "/");
  strcpy(mask->ip, p);

  p = strtok(NULL, "/");
  mask->len = atoi(p);
}

// Calculate ports from input arguments or use default file
static void _get_ports(portlist_t* pl, int argc, char** argv) {
  char path_tmpl[STRL] = {'\0'};
  strcpy(path_tmpl, (argc < 3) ? PORTSFILE : argv[2]);
  read_portlist_from_file(pl, path_tmpl);
}

// Calculate network and bandwidth from mask
static void _count_mask(mask_t* mask) {
  struct in_addr a;
  inet_pton(AF_INET, mask->ip, &a);
  uint32_t ip = ntohl(a.s_addr);
  uint32_t msk = 0xFFFFFFFF << (32 - mask->len);  // mask
  mask->network = ip & msk;                       // network
  mask->bandwidth = mask->network | ~msk;  // address range
}

// find next IP address in the subnet
pthread_mutex_t ip_lock = PTHREAD_MUTEX_INITIALIZER;
static uint32_t _next_ip(net_t* net) {
  static uint32_t current = 0;

  pthread_mutex_lock(&ip_lock);

  if (current == 0) {
    current = net->mask.network + 1;
  } else {
    current++;
  }

  pthread_mutex_unlock(&ip_lock);

  return (current < net->mask.bandwidth ? current : 0);
}

// convert IP address from uint32_t to string
static void _convert_ip_from_uint_to_str(char* buffer, uint32_t h) {
  struct in_addr ip_a = {0};
  ip_a.s_addr = htonl(h);  // сетевой порядок байтов
  inet_ntop(AF_INET, &ip_a, buffer, INET_ADDRSTRLEN);
}

// Scan provided port on provided host
static int _scan_one_host_one_port(const char* ip, const int port) {
  // Create scoker descriptor
  int sckt = socket(AF_INET, SOCK_STREAM, 0);
  if (sckt < 0) {
    exit(EXIT_FAILURE);
  }

  // This struct will contain all neccessary information
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;                 // TCP/IP
  addr.sin_port = htons(port);               // big/little endian conversion
  inet_pton(AF_INET, ip, &(addr.sin_addr));  // convert IP from string to binary

  // Set timeout for connection attempts
  struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
  setsockopt(sckt, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  setsockopt(sckt, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

  // Connection attempt and result return
  int result = connect(sckt, (struct sockaddr*)&addr, sizeof(addr));
  close(sckt);
  return result;
}

// Scan all ports for IP address and print if any port is open
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
static void _show_ip_for_all_open_ports(uint32_t ip, portlist_t * pl) {
  char buffer[INET_ADDRSTRLEN] = {'\0'};
  _convert_ip_from_uint_to_str(buffer, ip);
  for(int i = 0; i < pl->len; i++) {
    int closed = _scan_one_host_one_port(buffer, pl->data[i].number);
    
    pthread_mutex_lock(&print_lock);
    if(!closed) {
      printf("%s:%d\t%s\t%s\n", buffer, pl->data[i].number, pl->data[i].protocol, pl->data[i].description);
    } else {
      // printf("%s:%d\t--\n", buffer, pl->data[i].number);
    }
    pthread_mutex_unlock(&print_lock);
  }
}

// Print IP address responds on port 80
static void _show_ip_if_alive(net_t* net, uint32_t ip) {
  char buffer[INET_ADDRSTRLEN] = {'\0'};
  _convert_ip_from_uint_to_str(buffer, ip);

  int dead = _scan_one_host_one_port(buffer, net->pl.data[0].number);
  if (!dead) printf("%s %d\n", buffer, ip);
}

// Thread for checking all IP addresses in the subnet
// and printing those that respond on port 80
void* _show_all_ip_if_alive_in_thread(void* arg) {
  net_t* net = arg;
  uint32_t ip = 0;
  while (0 != (ip = _next_ip(net))) {
    _show_ip_if_alive(net, ip);
  }
  return NULL;
}

// Thread for checking all ports on all IP addresses in the subnet
void* _show_all_ip_if_found_in_thread(void* arg) {
  net_t* net = arg;
  uint32_t ip = 0;
  while (0 != (ip = _next_ip(net))) {
    _show_ip_for_all_open_ports(ip, &(net->pl));
  }
  return NULL;
}

// Entry point of the program
int main(int argc, char** argv) {
  // Initialize network map
  net_t net = {0};

  _get_mask(&(net.mask), argc, argv);  // - subnet mask
  _get_ports(&(net.pl), argc, argv); 
  _count_mask(&(net.mask));

  // Single threaded scan
  if (0) {
    uint32_t current_ip = 0;
    while (0 != (current_ip = _next_ip(&net))) {
      _show_ip_if_alive(&net, current_ip);
    }
  }

  // Multi-threaded scan
  // A if alive on port 80
  if (0) {
    // Start threads
    pthread_t th[TNUM] = {0};
    for (int i = 0; i < TNUM; i++) {
      pthread_create(&th[i], NULL, _show_all_ip_if_alive_in_thread, &net);
    }

    // wait for threads to finish
    for (int i = 0; i < TNUM; i++) {
      pthread_join(th[i], NULL);
    }
  }

  // B if found on any port
  // This is the main part of the program, where we scan all IPs in the subnet
  // and print those that respond on any port from the port list.
  if (1) {
    // Start threads
    pthread_t th[TNUM] = {0};
    for (int i = 0; i < TNUM; i++) {
      pthread_create(&th[i], NULL, _show_all_ip_if_found_in_thread, &net);
    }

    // wait for threads to finish
    for (int i = 0; i < TNUM; i++) {
      pthread_join(th[i], NULL);
    }
  }

  return EXIT_SUCCESS;
}