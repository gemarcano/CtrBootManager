#include <3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hb_menu/netloader.h"
#include "config.h"
#include "scanner.h"
#include "utility.h"
#include "menu.h"

//Shouldn't these extern declarations be in a header?
extern char boot_app[512];
extern bool boot_app_enabled;

extern void scanMenuEntry(menuEntry_s *me);

int bootApp(char *executablePath, executableMetadata_s *em, char *arg);

//Override the default from ctrulib
void __appInit() {
    srvInit();
    aptInit();
    fsInit();
    sdmcInit();
    openSDArchive();
    hidInit();
    acInit();
    ptmuInit();
    amInit();
    gfxInitDefault();
}

//Override the default from ctrulib
void __appExit() {
    gfxExit();
    netloader_exit();
    amExit();
    ptmuExit();
    acExit();
    hidExit();
    closeSDArchive();
    sdmcExit();
    fsExit();
    aptExit();
    srvExit();
}

int main(int argc, char *argv[]) {

    if (netloader_init() != 0) {
        // fix SOC_Initialize
        strcpy(boot_app, "/boot.3dsx");
        boot_app_enabled = true;
    }

    ctrbm_config config;
    ctrbm_config_init(&config);
    ctrbm_config_set_defaults(&config);
    bool config_loaded = ctrbm_config_read_from_disk(&config, "/boot.cfg");
    if (!config_loaded) {
        debug("Configuration file not loaded! Either the file does not exist, or it is malformatted...");
        debug("Initializing with default settings...");
    }

    //This only impacts the N3DS
    osSetSpeedupEnable(true);

    // offset potential issues caused by homebrew that just ran (from hb_menu) (like what?)
    aptOpenSession();
    APT_SetAppCpuTimeLimit(0); //According to http://3dbrew.org/APT:SetApplicationCpuTimeLimit, this is oh so wrong...
    aptCloseSession();

    if (!boot_app_enabled) { // fix SOC_Initialize
        if (config_loaded || config.n_entries == 0) { // recovery
            while (aptMainLoop()) {
                if (menu_more() == 0)
                    break;
            }
        } else {
            while (aptMainLoop()) {
                if (menu_boot() == 0)
                    break;
            }
        }
    }

    menuEntry_s *me = malloc(sizeof(menuEntry_s));
    strncpy(me->executablePath, boot_app, 128);
    initDescriptor(&me->descriptor);
    static char xmlPath[128];
    snprintf(xmlPath, 128, "%s", boot_app);
    int l = strlen(xmlPath);
    xmlPath[l - 1] = 0;
    xmlPath[l - 2] = 'l';
    xmlPath[l - 3] = 'm';
    xmlPath[l - 4] = 'x';
    if (fileExists(xmlPath))
        loadDescriptor(&me->descriptor, xmlPath);
    scanMenuEntry(me);

    return bootApp(me->executablePath, &me->descriptor.executableMetadata, me->arg);
}
