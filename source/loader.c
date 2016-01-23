#include <3ds.h>
#include <string.h>
#include <assert.h>

#include "brahma.h"
#include "utility.h"

//These are global variables...
char boot_app[512];
bool boot_app_enabled;

static_assert(sizeof(boot_app) == 512, "Size of the array has been changed!");

int load_3dsx(char *path) {
    memset(boot_app, 0, sizeof(boot_app));
    strncpy(boot_app, path, sizeof(boot_app));
    boot_app_enabled = true;
    return 0;
}

int load_bin(char *path, long offset) {

    if (brahma_init()) {
        int rc = load_arm9_payload_offset(path, (u32)offset, 0x10000);
        if (rc != 1) {
            debug("Err: Couldn't load arm9 payload...\n");
            return -1;
        }

        firm_reboot();
        brahma_exit();

    } else {
        debug("Err: Couldn't init brahma...\n");
        return -1;
    }

    return 0;
}

int load(char *path, long offset) {
    const char *ext = get_filename_ext(path);
    if (strcasecmp(ext, "bin") == 0
        || strcasecmp(ext, "dat") == 0) {
        return load_bin(path, offset);
    } else if (strcasecmp(ext, "3dsx") == 0) {
        return load_3dsx(path);
    } else {
        debug("Invalid file: %s\n", path);
        return -1;
    }
}
