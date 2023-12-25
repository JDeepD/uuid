#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "uuid1.h"

void generate_random_node(uuid_node_t *node) {
    srand((unsigned int)time(NULL));
    uint8_t oct[6];
    /* Need more read, is it dependent on endian-ness? For the time being, assuming Little endian */
    oct[0] = (uint8_t)rand() & 0x7F;
    oct[0] |= (1 << 15); /* Set unicast/multicast bit, since using random node */
    for(int i = 1; i <= 5; i++) {
        oct[i] = (uint8_t)rand() & 0xFF;
    }
    for(int i = 0; i < 6; i++) {
        node->nodeID[i] = oct[i];
    }
}

void generate_random_clockseq(uuid_cs_t *clock_seq) {
    srand((unsigned int)time(NULL));
    *clock_seq = (uuid_cs_t)rand() & 0xFFFF;
}

int get_802_MAC_ADDR(uuid_node_t *node) {
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
    for(int i = 0; i < 6; i++) {
        node->nodeID[i] = (uint8_t)ifr.ifr_hwaddr.sa_data[i];
    }
    return 0;
}

void get_system_time(uuid_time_t *timestamp) {
    struct timeval tp;
    gettimeofday(&tp, (struct timezone *)0);
    /* Offset current time (Unix epoch) to UTC-based(100ns intervals passed from Oct 15, 1582) */
    *timestamp = ((uuid_time_t)tp.tv_sec * 10000000) + ((uuid_time_t)tp.tv_usec * 10) + (uuid_time_t)(0x01B21DD213814000);
}

static uuid_state st;
int read_state(uuid_time_t *timestamp, uuid_cs_t *clock_seq, uuid_node_t *node) {
    static int init = 0;
    FILE *fp;
    if (!init) {
        fp = fopen("state", "rb");
        if(fp == NULL) return 0;
        fread(&st, sizeof st, 1, fp);
        fclose(fp);
        init = 1;
    }
    *timestamp = st.timestamp;
    *clock_seq = st.clock_seq;
    *node = st.node;
    return 1;
}

void write_state(uuid_time_t timestamp, uuid_cs_t clock_seq, uuid_node_t node) {
    static int init = 0;
    static uuid_time_t next_save;
    FILE *fp;
    if (!init) {
        next_save = timestamp;
        init = 1;
    }
    /* Save to volatile store */
    st.timestamp = timestamp;
    st.clock_seq = clock_seq;
    st.node = node;
    /* Save to non-volatile store every 10 seconds */
    if(timestamp >= next_save) {
        fp = fopen("state", "wb");
        fwrite(&st, sizeof st, 1, fp);
        fclose(fp);
        next_save = timestamp + (10 * 10 * 1000 * 1000);
    }
}

void format_uuid(uuid_t *uuid, uuid_time_t timestamp, uuid_cs_t clock_seq, uuid_node_t node) {
    uuid->time_low = (uint32_t)(timestamp & 0xFFFFFFFF);
    uuid->time_mid = (uint16_t)((timestamp >> 32) & 0xFFFF);
    uuid->time_hi_and_version = (uint16_t)((timestamp >> 48) & 0x0FFF);
    uuid->time_hi_and_version |= (UUID_VERSION << 12);
    uuid->clock_seq_low = (uint8_t)(clock_seq & 0xFF);
    uuid->clock_seq_hi_and_reserved = (uint8_t)((clock_seq & 0x3F00) >> 8);
    uuid->clock_seq_hi_and_reserved |= 0x80;
    memcpy(&uuid->node, &node, sizeof uuid->node);
}

int uuid_create(uuid_t *uuid) {
    uuid_time_t timestamp, last_time;
    uuid_cs_t clock_seq;
    uuid_node_t node, last_node;
    int F;
    LOCK;
    get_system_time(&timestamp);
    generate_random_node(&node);
    F = read_state(&last_time, &clock_seq, &last_node);
    if(!F || memcmp(&node, &last_node, sizeof node)) 
        generate_random_clockseq(&clock_seq);
    else if(timestamp < last_time)
        clock_seq++;
    write_state(timestamp, clock_seq, node);
    UNLOCK;
    format_uuid(uuid, timestamp, clock_seq, node);
}

void puid(uuid_t u) {
    int i;

    printf("%8.8x-%4.4x-%4.4x-%2.2x%2.2x-", u.time_low, u.time_mid,
    u.time_hi_and_version, u.clock_seq_hi_and_reserved,
    u.clock_seq_low);
    for (i = 0; i < 6; i++)
        printf("%2.2x", u.node.nodeID[i]);
    printf("\n");
}

void main(int argc, char **argv)
{
    uuid_t u;
    uuid_create(&u);
    printf("uuid_create(): "); puid(u);
}
