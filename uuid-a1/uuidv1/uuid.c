/*
This is the naive (most inefficient) implementation of UUIDv1
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include "uuid.h"

// 4 bit UUID version(1)
#define UUID_VERSION 0b0001

// Variant as specified in RFC 4122
#define UUID_VARIANT 0b100

/* 
Network interface for node identifier generation
Make sure Interface Name Size (IFNAMSIZ) is at max IFNAMSIZ-1
IFNAMSIZ is typically 16 including the null character.

TODO: A helper function for getting the network interface programmatically
*/
#define NETWORK_INTERFACE "wlan0"

// TODO: Acquire mutex lock
#define LOCK

// TODO: Release mutex lock
#define UNLOCK

void get_system_time(uuid_time_t *uuid_timestamp);
void get_clock_seq(clock_seq_t *clock_seq);
int generate_802MAC_node_id(uuid_node_t *node);
int generate_random_node_id(uuid_node_t *node);
void generate_node_identifier(uuid_node_t *node);
int read_state(clock_seq_t *clockseq, uuid_time_t* timestamp, uuid_node_t *node);
int generate_uuid1(uuid_t* uuid);

int main() {
    uuid_t* uuid = (uuid_t *)malloc(sizeof(uuid_t));
    LOCK;
    generate_uuid1(uuid);
    UNLOCK;
    printf("Hello World\n");
}

void get_clock_seq(clock_seq_t *clock_seq, uuid_time_t* timestamp) {
    if(!st) {
        /* TODO : Set random clock sequencce */
        return;
    }
    else if (st.ts < timestamp) return;
    else {
        *clockseq = (*clockseq + 1) % 16384;
    }
}


static uuid_state st;
int read_state(clock_seq_t *clockseq, uuid_time_t* timestamp, uuid_node_t *node) {
    // init will persist across calls to read_state;
    static int init = 0;
    FILE *fp;
    if (!init) {
        fp = fopen("state", "rb");
        if(fp == NULL) return 0;
        fread(&st, sizeof st, 1, fp);
        fclose(fp);
        init = 1;
    }
    *clockseq = st.cs;
    *timestamp = st.ts;
    *node = st.node;
    return 1;
}

int generate_802MAC_node_id(uuid_node_t *node) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)  return -1;
    struct ifreq ifr;
    strncpy(ifr.ifr_name, NETWORK_INTERFACE, IFNAMSIZ-1);
    /* 
      SIOCGIFHWADDR: It stands for Socket I/O Control Get Interface Hardware Address. 
      This ioctl command is used to query the hardware (MAC) address associated with 
      a network interface.
    */
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1) {
        close(sock);
        return -1;
    }
    for(int i = 0; i < nodelen; i++) {
        node->nodeID[i] = (char)ifr.ifr_hwaddr.sa_data[i];
    }
    return 0;
}


int generate_uuid1(uuid_t *uuid) {
    uuid_time_t timestamp;
    unsigned char node[NODE_LEN];
    get_system_time(&timestamp);
    if(generate_802MAC_node_id(node, NODE_LEN) == -1) {
        perror("Error in generating IEEE 802 MAC address. Using random node identifier instead");
        /*
        TODO: Use high quality random node identifier  
        */
        return -1;
    }

    
    /* lower order 32 bits of timestamp */
    uuid->time_low = (uint32_t)(timestamp & 0xFFFFFFFF);
    /* middle order 16 bits of timestamp */
    uuid->time_mid = (uint16_t)((timestamp >> 32) & 0xFFFF);
    /* higher order 12 bits of timestamp + 4 bit UUID version */
    uuid->time_hi_and_version = (uint16_t)((timestamp >> 48) & 0x0FFF);
    uuid->time_hi_and_version |= (UUID_VERSION << 12);

    /* Node Identifier : IEEE 802 MAC if available, else random */
    uuid->node = node;
}
