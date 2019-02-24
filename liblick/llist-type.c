#define NO_UNDEF
#include "llist.h"

#include <assert.h>

#undef LIST_NAME
#undef LIST_TYPE
#undef FREE_LIST

#define LIST_NAME string_node_t
#define LIST_TYPE char
#define FREE_LIST_FN free
#include "llist-type-inc.c"

#define LIST_NAME drive_node_t
#define LIST_TYPE struct drive_t
#define FREE_LIST_FN free_drive
#include "llist-type-inc.c"

#define LIST_NAME installed_node_t
#define LIST_TYPE struct installed_t
#define FREE_LIST_FN free_installed
#include "llist-type-inc.c"

#define LIST_NAME distro_info_node_t
#define LIST_TYPE struct distro_info_t
#define FREE_LIST_FN free_distro_info
#include "llist-type-inc.c"

#define LIST_NAME distro_node_t
#define LIST_TYPE struct distro_t
#define FREE_LIST_FN free_distro
#include "llist-type-inc.c"

#define LIST_NAME section_node_t
#define LIST_TYPE struct section_t
#define FREE_LIST_FN free_section
#include "llist-type-inc.c"
