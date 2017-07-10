#include <stdio.h>

#include "../lickdir.h"
#include "../llist.h"

typedef enum {
    S_UNLABELED,
    S_HEADER,
    S_FOOTER,
    S_SECTION,
} type_e;

typedef struct {
    type_e type;
    char *id;
    const char *content;
    size_t content_len;
} section_t;

/**
 * @brief Creates a new section
 * @param type the type of section
 * @param id the id of the section
 * @param content the content of the section. Must exist until the section is
 * freed
 * @return the new section
 */
section_t *new_section(type_e type, const char *id, const char *content);
/**
 * @brief Free memory used by a section_t
 * @param s the section to free
 */
void free_section(section_t *s);

#define LIST_NAME section_node_t
#define LIST_TYPE section_t
#define FREE_LIST_FN free_section
#include "../llist-type.h"

/**
 * @brief Gets a list of sections from a flat-file menu
 * @param f a NULL-terminated string, the contents of a flat-file menu. Must
 * exist until the return value is freed
 * @return a list of menu sections
 */
section_node_t *get_sections(const char *f);
/**
 * @brief Appends the section sec to the list of sections secs
 * @param secs a list of section_t
 * @param sec the section to add to the end of the list
 * @return the list of sections, with sec
 */
section_node_t *append_section(section_node_t *secs, section_t *sec);
/**
 * @brief Given the sections of a menu, removes the section named sec
 * @param secs the menu as a list of sections
 * @param sec the name of the section
 */
section_node_t *remove_section(section_node_t *secs, const char *sec);
/**
 * @brief Write a list of sections to a file
 * @param f the file to write to
 * @param secs the list of sections to write
 */
void write_sections(FILE *f, section_node_t *secs);

/**
 * @brief append a section to a flat menu file
 * @param menu_path the path to the menu file
 * @param id the ID of the section
 * @param section the content of the section
 * @param lick the LICK directory
 */
int flat_append_section(const char *menu_path, const char *id,
        const char *section, lickdir_t *lick);
/**
 * @brief removes a section from a flat menu file
 * @param menu_path the path to the menu file
 * @param id the id of the section
 * @param lick the LICK directory
 * @return 1 on success
 */
int flat_remove_section(const char *id, const char *menu, lickdir_t *lick);
