#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>

#define MAX_FILES     32
#define MAX_FILENAME  64
#define MAX_CONTENT   512

typedef struct fs_node {
    char name[MAX_FILENAME];
    char content[MAX_CONTENT];
    size_t size;
    struct fs_node* next;  // veza u LANAC fajlova
} fs_node_t;

void fs_init();
int fs_create_file(const char* name);
fs_node_t* fs_find_file(const char* name);
int fs_write(fs_node_t* file, const char* data, size_t len);
void fs_ls();
// za sada dummy cd
int fs_cd(const char* dirname);

#endif
