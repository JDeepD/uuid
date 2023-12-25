#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

int main() {
    struct ifreq ifr;
    char *interface = "wlan0";  // Change this to your network interface

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1) {
        perror("ioctl");
        close(sock);
        return 1;
    }

    close(sock);

    printf("MAC Address: ");
    for (int i = 0; i < 6; ++i) {
        printf("%.2X", (unsigned char)ifr.ifr_hwaddr.sa_data[i]);
        if (i < 5) printf(":");
    }
    printf("\n");

    return 0;
}

