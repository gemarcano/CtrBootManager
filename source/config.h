#ifndef _config_h_
#define _config_h_

#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#define CONFIG_MAX_ENTRIES 12U

/** @brief Represents an entry in a boot entry in a configuration file.
 *
 *  title - Holds a title for the boot entry.
 *  path - Holds the actual path to disk for the entry.
 *  auto_key - Holds the key, based on <3ds/services/hid.h> that causes this
 *      particular entry to boot when timeout is 0.
 *  offset - The offset to the exploit payload to execute.
 */
typedef struct ctrbm_config_entry_ {
    char *title;
    char *path;
    uint32_t auto_key;
    uintptr_t offset;
} ctrbm_config_entry;

/** @brief Represents a boot configuration.
 *
 *  timeout - The amount of time to wait before automatically booting, in
 *      seconds.
 *  autobootfix - The number of vblanks to wait for after starting the boot, as
 *      an attempt to wait for the system to be ready for booting.
 *  default_entry - The entry to load by default when a timeout occurs.
 *  meny_key - The key, based on <3ds/services/hid.h>, that triggers the menu
 *      to show up in the case that the timeout is set to zero (autoboot).
 *  n_entries - The number of entries in the configuration.
 *  entries - The actual boot entries in the configuration.
 */
typedef struct ctrm_config_
{
    time_t timeout;
    uint8_t autobootfix;
    uint8_t default_entry;
    uint32_t menu_key;
    size_t n_entries;
    ctrbm_config_entry entries[CONFIG_MAX_ENTRIES]; //Make this be decided dynamically FIXME
} ctrbm_config;

/** @brief Initializes a single configuration entry with the given parameters.
 *
 *  @param [in,out] entry Pointer to entry to initialize.
 *  @param [in] title String containing the title of the entry. This is copied
 *      into the entry.
 *  @param [in] path String containing the path to the entry on disk. This is
 *      copied into the entry.
 *  @param [in] auto_key Represents the key to use to have this entry be auto-
 *      selected in the case the config timeout=0.
 *  @param [in] offset The offset of the payload.
 */
void ctrbm_config_entry_init(
    ctrbm_config_entry *entry,
    const char *title,
    const char *path,
    uint32_t auto_key,
    uintptr_t offset);

/** @brief Deallocates any resources being held by the entry. 
 *
 *  @param [in,out] entry Pointer to the entry to destroy.
 */
void ctrbm_config_entry_destroy(ctrbm_config_entry *entry);

/** @brief Initializes a configuration.
 *
 *  @param [in,out] config Pointer to the configuration to initialize.
 */
void ctrbm_config_init(ctrbm_config *config);

/** @brief Frees the resources being used up bby the given configuration.
 *
 *  @param [in,out] config Pointer to the configuration to destroy.
 */
void ctrbm_config_destroy(ctrbm_config *config);

/** @brief Adds an entry to the configuration.
 * 
 *  @param [in,out] config Pointer to config to add entry to.
 *  
 *  @param [in] title String containing the title of the entry. This is copied
 *      into the entry.
 *  @param [in] path String containing the path to the entry on disk. This is
 *      copied into the entry.
 *  @param [in] auto_key Represents the key to use to have this entry be auto-
 *      selected in the case the config timeout=0.
 *  @param [in] offset The offset of the payload.
 *
 *  @returns True on success, false otherwise. On failure, nothing is added.
 */
bool ctrbm_config_add_entry(
    ctrbm_config *config,
    const char *title,
    const char *path,
    uint32_t auto_key,
    uintptr_t offset );

/** @brief Removes an entry from the given configuration.
 *
 *  @param [in,out] config Pointer to the configuration to act upon.
 *  @param [in] index Index of entry to remove.
 *  @param [out] entry Pointer to an entry to be used as the destination of the
 *      removed entry. If NULL, this parameter is ignored.
 *
 *  @returns True on success, false on failure. It will fail if the index is
 *      beyond the last added entry.
 */
bool ctrbm_config_remove_entry(
    ctrbm_config *config, size_t index, ctrbm_config_entry *apEntry);

/** @brief Sets the default values for CtrBootManager's configuration to the
 *      given configuration.
 *  @parm [in,out] config Pointer to the configuration to operate on.
 *
 *  @post The values on the configuration pointed by config are updated as thus:
 *      timeout = 3
 *      autobootfix = 8
 *      default_entry = 0
 *      menu_key = 4, or KEY_SELECT in 3ds/hid.h
 */
void ctrbm_config_set_defaults(ctrbm_config *config);

/** @brief Reads a configuration file from the disk.
 *
 *  The configuration format must be as follows (specified like a regex):
 *      boot_config [=|:] {
 *        timeout = [0-9]+;?
 *        autobootfix = [0-9]+;?
 *        recovery = [0-9]+;?
 *        default = [0-9]+;?
 *        entries [=|:] (
 *          {
 *            title [:|=] ".+";?
 *            path [:|=] ".+";?
 *            offset [:|=] "0x[0-9]+";?
 *          }
 *          ... //There can be up to 12 (FIXME) entries.
 *        );?
 *      };?
 *
 *  @param [in,out] config The configuration file to updated with the data from
 *      disk.
 *  @param [in] path Path to the configuration file.
 *
 *  @returns True on success, false on failure. This function fails if it can't
 *      find the file, or if the configuration fails to validate.
 */
bool ctrbm_config_read_from_disk(ctrbm_config *config, const char* path);

/** @brief Writes a configuration file to the disk.
 *
 *  See ctrbm_config_read_from_disk for details about the file format.
 *
 *  @param [in] config The configuration file to save to disk.
 *  @param [in] path Path where to save the configuration file.
 *
 *  @returns True on succes, false on failure. Failure conditions include not
 *      being able to open the path for writing.
 */
bool ctrbm_config_write_to_disk(const ctrbm_config *config, const char* path);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif // _config_h_

