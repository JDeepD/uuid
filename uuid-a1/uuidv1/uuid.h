#ifndef UUID_H
#define UUID_H

typedef struct {
    char nodeID[6];
} uuid_node_t;

typedef struct {
    uint32_t            time_low;
    uint16_t            time_mid;
    uint16_t            time_hi_and_version;
    uint8_t             clock_seq_hi_and_reserved;
    uint8_t             clock_seq_low;
    uuid_node_t         node;
    
} uuid_t;

/* 64 bit, even though timestamp will be only of 60 bits */ 
typedef uint64_t uuid_time_t;
/* 16 bit, even though clock seq will be of 13 bits */ 
typedef uint16_t clock_seq_t;

/* data type for UUID generator persistent state */
typedef struct {
    uuid_time_t     ts;          /* saved timestamp */
    uuid_node_t     node[6];     /* saved node ID */
    uint16_t        cs;          /* saved clock sequence */
} uuid_state;


#endif
