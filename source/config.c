#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <limits.h>
#include <libconfig.h>
#include "config.h"

void ctrbm_config_entry_init(
    ctrbm_config_entry *apEntry,
    const char *apTitle,
    const char *apPath,
    uint32_t aAuto_key,
    uintptr_t aOffset)
{
    size_t size = strlen(apTitle) + 1;
    apEntry->title = malloc(size);
    strncpy(apEntry->title, apTitle, size);

    size = strlen(apPath) + 1;
    apEntry->path = malloc(size);
    strncpy(apEntry->path, apPath, size);
    
    apEntry->auto_key = aAuto_key;
    apEntry->offset = aOffset;
}

void ctrbm_config_entry_destroy(ctrbm_config_entry *apEntry) {
    if (apEntry->title) {
        free(apEntry->title);
    }
    apEntry->title = NULL;
    
    if (apEntry->path) {
        free(apEntry->path);
    }
    apEntry->path = NULL;
    apEntry->auto_key = 0;
    apEntry->offset = 0;
}

void ctrbm_config_init(ctrbm_config *apConfig) {
    apConfig->timeout = 0;
    apConfig->autobootfix = 0;
    apConfig->default_entry = 0;
    apConfig->menu_key = 0;
    apConfig->n_entries = 0;
    memset(apConfig->entries, 0, sizeof(ctrbm_config_entry)*12); //FIXME make this be dynamic
}

void ctrbm_config_destroy(ctrbm_config *apConfig) {
    apConfig->timeout = 0;
    apConfig->autobootfix = 0;
    apConfig->default_entry = 0;
    for (size_t i = 0; i  < apConfig->n_entries; ++i) {
        ctrbm_config_entry_destroy(&apConfig->entries[i]);
    }
    apConfig->n_entries = 0;
}

bool ctrbm_config_add_entry(
    ctrbm_config *apConfig,
    const char *apTitle,
    const char *apPath,
    uint32_t aAuto_key,
    uintptr_t aOffset) 
{
    bool result = false;
    if (apConfig && apConfig->n_entries < 12) {
        ctrbm_config_entry_init(
            &apConfig->entries[apConfig->n_entries++],
	    apTitle,
	    apPath,
	    aAuto_key,
	    aOffset);
	result = true;
    }
    return result;
}

bool ctrbm_config_remove_entry(
    ctrbm_config *apConfig, size_t index, ctrbm_config_entry *apEntry)
{
    bool result = false;
    if (apConfig && apConfig->n_entries && index < apConfig->n_entries) {
	
	if (apEntry) {
            *apEntry = apConfig->entries[index];
	}
	else {
            ctrbm_config_entry_destroy(&apConfig->entries[index]);
	}
	//Move all entries past index back one.
	for (size_t i = index; i < apConfig->n_entries - 1; i++)
	{
		apConfig->entries[i] = apConfig->entries[i+1];
	}
	apConfig->n_entries--;
	result = true;
    }
    return result;
}

void ctrbm_config_set_defaults(ctrbm_config *apConfig) {
    apConfig->timeout = 3;
    apConfig->autobootfix = 8;
    apConfig->default_entry = 0;
    apConfig->menu_key = 4; //KEY_SELECT in <3ds/services/hid.h>
}

static void loadEntry(
    ctrbm_config_entry *apEntry, const config_setting_t *apEntryLib)
{
    const char *title, *path, *offset;
    int key = 0;
    
    if (!(config_setting_lookup_string(apEntryLib, "title", &title)
	  && config_setting_lookup_string(apEntryLib, "path", &path)))
	return;

    size_t sizeOfString = strlen(title);
    if (apEntry->title) {
        free(apEntry->title);
    }
    apEntry->title = malloc(sizeOfString + 1); //+1 for NULL char
    //what if malloc fails? FIXME
    strncpy(apEntry->title, title, sizeOfString + 1);

    sizeOfString = strlen(path);
    if (apEntry->path) {
        free(apEntry->path);
    }
    apEntry->path = malloc(sizeOfString + 1); //+1 for NULL char
    //what if malloc fails? FIXME
    strncpy(apEntry->path, path, sizeOfString + 1);
    
    apEntry->auto_key = 0;
    if (config_setting_lookup_int(apEntryLib, "key", &key) && key >= 0) {
	apEntry->auto_key = (uint32_t)key;
    }
    if (config_setting_lookup_string(apEntryLib, "offset", &offset)) {
	apEntry->offset = (uintptr_t)strtoul(offset, NULL, 16);
    }
}

static void getSetting(config_setting_t *apRoot, const char *name, int *output) {
    if (config_setting_lookup_int(apRoot, name, output)) {
        if (*output < 0) {
            *output = 0;
        }
    }
}

static bool validateEntry(config_setting_t *entry) {
    if (!config_setting_lookup(entry, "title")
	  || !config_setting_lookup(entry, "path"))
	return false;

   return true;
}

static bool validateConfig(const config_t *config) {
    config_setting_t *setting_boot = config_lookup(config, "boot_config");
    config_setting_t *setting_entries =
        config_setting_lookup(setting_boot, "entries");
    if (!setting_boot || !setting_entries ||
        !config_setting_lookup(setting_boot, "timeout") ||
	!config_setting_lookup(setting_boot, "autobootfix") ||
	!config_setting_lookup(setting_boot, "default") ||
        !config_setting_lookup(setting_boot, "recovery"))
    {
        return false;
    }
   
    size_t count = (size_t)config_setting_length(setting_entries);
    if (count > CONFIG_MAX_ENTRIES)
        count = CONFIG_MAX_ENTRIES;

    for (size_t i = 0; i < count; ++i) {
        config_setting_t *entry =
	    config_setting_get_elem(setting_entries, i);
        if (!validateEntry(entry))
	    return false;
    }
    return true;
}

//FIXME error checking?
static bool loadConfig(ctrbm_config *apConfig, const config_t *apConfigLib) {
    
    if (!validateConfig(apConfigLib))
        return false;
    
    config_setting_t *setting_boot = config_lookup(apConfigLib, "boot_config");
    int configOutput;
    getSetting(setting_boot, "timeout", &configOutput);
    apConfig->timeout = configOutput;
	
    getSetting(setting_boot, "autobootfix", &configOutput);
    apConfig->autobootfix = configOutput;
        
    getSetting(setting_boot, "default", &configOutput);
    apConfig->default_entry = configOutput;
        
    getSetting(setting_boot, "recovery", &configOutput);
    apConfig->menu_key = configOutput;

    config_setting_t *setting_entries =
        config_lookup(apConfigLib, "boot_config.entries");

    size_t count = (size_t)config_setting_length(setting_entries);
    if (count > CONFIG_MAX_ENTRIES)
        count = CONFIG_MAX_ENTRIES;

    for (size_t i = 0; i < count; ++i) {
        config_setting_t *entry =
            config_setting_get_elem(setting_entries, i);
        loadEntry(&apConfig->entries[i], entry);
        apConfig->n_entries++;
    }

    // prevent invalid boot index
    if (apConfig->default_entry >= apConfig->n_entries) {
        apConfig->default_entry = apConfig->n_entries;
    } 

    return true;
}

static void saveNewEntry(
    config_setting_t *apEntryLib, const ctrbm_config_entry *apEntry) {

    config_setting_t *setting;
    // add title
    setting = config_setting_add(apEntryLib, "title", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, apEntry->title);
    // add path
    setting = config_setting_add(apEntryLib, "path", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, apEntry->path);
    // add key
    setting = config_setting_add(apEntryLib, "key", CONFIG_TYPE_INT);
    config_setting_set_int(setting, apEntry->auto_key);
    // add offset
    setting = config_setting_add(apEntryLib, "offset", CONFIG_TYPE_STRING);
    const size_t hexDigits = sizeof(apEntry->offset)*CHAR_BIT/4 + sizeof("0x");
    char *offset_str = malloc(hexDigits);
    snprintf(offset_str, hexDigits, "%"PRIXPTR, apEntry->offset);
    config_setting_set_string(setting, offset_str);
    free(offset_str);
}

static config_t saveNewConfig(const ctrbm_config *apConfig) {
    config_t config;
    config_init(&config);
    
    config_setting_t *setting_root = config_root_setting(&config);
    
    // create main group
    config_setting_t *setting_boot =
        config_setting_add(setting_root, "boot_config", CONFIG_TYPE_GROUP);

    // create timeout setting
    config_setting_t *setting = setting =
        config_setting_add(setting_boot, "timeout", CONFIG_TYPE_INT);
    config_setting_set_int(setting, (int)apConfig->timeout);

    // create autobootfix setting
    setting = config_setting_add(setting_boot, "autobootfix", CONFIG_TYPE_INT);
    config_setting_set_int(setting, apConfig->autobootfix);

    // create recovery setting
    setting = config_setting_add(setting_boot, "recovery", CONFIG_TYPE_INT);
    config_setting_set_int(setting, apConfig->menu_key);

    // create default setting
    setting = config_setting_add(setting_boot, "default", CONFIG_TYPE_INT);
    config_setting_set_int(setting, apConfig->default_entry);

    // create entries group setting
    setting = config_setting_add(setting_boot, "entries", CONFIG_TYPE_LIST);

    //Now proceed to copy all entries
    for (size_t i = 0; i < apConfig->n_entries; ++i) {
        config_setting_t *entry =
	    config_setting_add(setting, NULL, CONFIG_TYPE_GROUP);
	saveNewEntry(entry, &apConfig->entries[i]);
    }

    return config;
}

bool ctrbm_config_write_to_disk(const ctrbm_config *apConfig, const char* path){
    config_t config = saveNewConfig(apConfig);
    bool result = false;
    if (config_write_file(&config, path)) {
        result = true;
    }
    config_destroy(&config);
    return result;
}

bool ctrbm_config_read_from_disk(ctrbm_config *apConfig, const char* path) {
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, path)) {
        return false;
    }
    bool result = loadConfig(apConfig, &cfg);
    config_destroy(&cfg);
    return result;
}
