#ifndef STATE_H
#define STATE_H

typedef enum {
    SAFE_TO_APPROACH,
    READY_TO_LAUNCH,
    CRAWL,
    FAULT // must be last
} State;

#endif