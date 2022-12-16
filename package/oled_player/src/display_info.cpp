#include "display_info.h"
#include <arpa/inet.h>
#include <assert.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>


using namespace std;

// https://stackoverflow.com/questions/1570511/c-code-to-get-the-ip-address
string get_ip_address(const char *if_str)
{
  int fd = socket(AF_INET, SOCK_DGRAM, 0);

  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  snprintf(ifr.ifr_name, IFNAMSIZ, "%s", if_str);
  ioctl(fd, SIOCGIFADDR, &ifr);

  string ip_str = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
  close(fd);
  return ip_str;
}

bool eth_connection(char *ifname)
{
  struct ifreq ifr;

  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, ifname);

  int found = false;
  int dummy_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (ioctl(dummy_fd, SIOCGIFFLAGS, &ifr) != -1)
    found = (ifr.ifr_flags & (IFF_UP | IFF_RUNNING)) == (IFF_UP | IFF_RUNNING);
  close(dummy_fd);
  return found;
}

// Get network connection values
bool connection_info::init()
{
  type = TYPE_UNKNOWN;
  if_name.clear();
  ip_addr.clear();
  link = 0;

  // Check for ethernet
  // const char *digits = "0123456789";
  // char ifname[] = "ethN";
  // for (int i = 0; i < 4; i++) {
  //   ifname[3] = digits[i];
  //   if (eth_connection(ifname)) {
  //     type = TYPE_ETH;
  //     if_name = ifname;
  //     break;
  //   }
  // }

  // Check for wifi
  if (type == TYPE_UNKNOWN) {
    FILE *fproc = fopen("/proc/net/wireless", "r");
    if (fproc != NULL) {
      char buf[128];
      for (int i = 0; i < 30; i++) { // Link is in position 30
        if (fscanf(fproc, "%s", buf) == EOF) {
          strcpy(buf, "0");
          break;
        }
        if (i == 27) { // the interface name
          if_name = buf;
          if (buf[0] != '\0') // remove the trailing ':'
            if_name.resize(if_name.size() - 1);
        }
      }
      fclose(fproc);

      if (if_name.size()) {
        type = TYPE_WIFI;
        link = atoi(buf);
      }
    }
  }

  if (type != TYPE_UNKNOWN)
    ip_addr = get_ip_address(if_name.c_str());

  return (type != TYPE_UNKNOWN);
}