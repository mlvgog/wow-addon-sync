#ifndef SYNC_H
#define SYNC_H

#include <time.h>

typedef struct {
    char local_wtf[1024];
    char local_interface[1024];
    char remote_wtf[1024];
    char remote_interface[1024];
    volatile float wtf_progress;
    volatile float interface_progress;
    volatile int wtf_total;
    volatile int wtf_count;
    volatile int interface_total;
    volatile int interface_count;
    volatile int state;
} SyncState;

int load_config(const char *filename, char *local_wtf, char *local_interface,
                char *remote_wtf, char *remote_interface);
void sync_folder(const char *local, const char *remote, const char *name, SyncState *sync, volatile float *progress);

int sync_main(int argc, char *argv[], SyncState *sync);
#endif
