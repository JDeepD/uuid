#ifndef UUID1_H
#define UUID1_H

/* 4 bit UUID version(1) */
#define UUID_VERSION 0b0001

/* Variant as specified in RFC 4122 */
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

typedef uint64_t    uuid_time_t;
typedef uint16_t    uuid_cs_t;

typedef struct {
    uint8_t         nodeID[6];
} uuid_node_t;

typedef struct {
    uint32_t        time_low;
    uint16_t        time_mid;
    uint16_t        time_hi_and_version;
    uint8_t         clock_seq_low;
    uint8_t         clock_seq_hi_and_reserved;
    uuid_node_t     node;
} uuid_t;

typedef struct {
    uuid_time_t     timestamp;
    uuid_node_t     node;
    uuid_cs_t       clock_seq;
} uuid_state;

#endif
