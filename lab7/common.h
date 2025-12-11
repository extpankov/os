#ifndef COMMON_H
#define COMMON_H

#define MEM_REGION "/shared_segment"
#define LOCK_PATH "/tmp/producer.lock"
#define MAX_BUF 256

typedef struct {
    char content[MAX_BUF];
    pid_t owner;
} SegmentData;

#endif
