#include "filesystem.h"
#include "../include/print.h"
#include "../include/string.h"

static fs_node_t files[MAX_FILES];
static int file_count = 0;
static fs_node_t* head = NULL;

void fs_init() {
    file_count = 0;
    head = NULL;
}

int fs_create_file(const char* name) {
    if (file_count >= MAX_FILES) return 0;

    // Provera da li fajl već postoji
    for (fs_node_t* cur = head; cur; cur = cur->next) {
        if (strcmp(cur->name, name)) {
            return 0; // postoji već
        }
    }

    fs_node_t* node = &files[file_count++];
    // Ručno kopiranje imena
    int i = 0;
    while (name[i] != '\0' && i < MAX_FILENAME - 1) {
        node->name[i] = name[i];
        i++;
    }
    node->name[i] = '\0';

    node->content[0] = '\0';
    node->size = 0;

    // ubacimo na početak linked liste
    node->next = head;
    head = node;

    return 1;
}

fs_node_t* fs_find_file(const char* name) {
    for (fs_node_t* cur = head; cur; cur = cur->next) {
        if (strcmp(cur->name, name)) {
            return cur;
        }
    }
    return NULL;
}

int fs_write(fs_node_t* file, const char* data, size_t len) {
    if (!file) return 0;
    if (len >= MAX_CONTENT) len = MAX_CONTENT - 1;
    // kopiramo podatke
    for (size_t i = 0; i < len; i++) {
        file->content[i] = data[i];
    }
    file->content[len] = '\0';
    file->size = len;
    return 1;
}

void fs_ls() {
    if (!head) {
        print("Nema fajlova.\n");
        return;
    }
    for (fs_node_t* cur = head; cur; cur = cur->next) {
        print(cur->name);
        print("\n");
    }
}

int fs_cd(const char* dirname) {
    (void)dirname;
    return 1;
}
