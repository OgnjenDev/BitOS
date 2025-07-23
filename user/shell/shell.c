#include "shell.h"
#include "../../include/print.h"
#include "../../include/string.h"
#include "../../fs/filesystem.h"
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define NULL 0
#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

char* cmdline = "shell:# ";

// I/O port helpers
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// Convert BCD to binary
int bcd_to_bin(uint8_t val) {
    return ((val / 16) * 10) + (val & 0x0F);
}

// Print integer (basic)
void print_int(int num) {
    char buffer[16];
    int i = 0;

    if (num == 0) {
        print("0");
        return;
    }

    if (num < 0) {
        print("-");
        num = -num;
    }

    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i--) {
        char str[2] = {buffer[i], '\0'};
        print(str);
    }
}

// CMOS register read
uint8_t read_cmos_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

// RTC time function
void get_time() {
    int hour = bcd_to_bin(read_cmos_register(0x04));
    int minute = bcd_to_bin(read_cmos_register(0x02));
    int second = bcd_to_bin(read_cmos_register(0x00));

    print("Time: ");
    print_int(hour); print(":");
    if (minute < 10) print("0");
    print_int(minute); print(":");
    if (second < 10) print("0");
    print_int(second);
    print("\n");
}

// RTC date function
void get_date() {
    int day = bcd_to_bin(read_cmos_register(0x07));
    int month = bcd_to_bin(read_cmos_register(0x08));
    int year = bcd_to_bin(read_cmos_register(0x09));

    print("Date: ");
    if (day < 10) print("0");
    print_int(day); print(".");
    if (month < 10) print("0");
    print_int(month); print(".");
    print_int(2000 + year);  // Assume 2000+
    print("\n");
}

// Shutdown for QEMU
void shutdown() {
    print("Shutting down...\n");
    outw(0x604, 0x2000);  // QEMU ACPI shutdown
}

// Built-in help
void help() {
    print("Basic Commands:\n");
    print("about\t-\tabout the system\n");
    print("clear\t-\tclear the screen\n");
    print("help\t-\tbasic help\n");
    print("hello\t-\tgreet the user\n");
    print("version\t-\tshow OS version\n");
    print("reboot\t-\tsimulate system reboot\n");
    print("time\t-\tshow current time\n");
    print("date\t-\tshow current date\n");
    print("author\t-\tshow developer name\n");
    print("joke\t-\tdisplay a random joke\n");
    print("echo\t-\trepeat input\n");
    print("shutdown\t-\tturn off the system (QEMU)\n");
    print("ls\t-\tlist files\n");
    print("cd\t-\tchange directory\n");
    print("touch\t-\tcreate new file\n");
    print("write\t-\twrite to file\n");
    print("cat\t-\tprint file content\n");
    print("snake\t-\tplay Snake game\n");
}

// Helper to split command line into words by space
int split_words(char* input, char* argv[], int max_args) {
    int argc = 0;
    char* token = input;
    while (*token != '\0' && argc < max_args) {
        while (*token == ' ') token++;
        if (*token == '\0') break;
        argv[argc++] = token;
        while (*token != ' ' && *token != '\0') token++;
        if (*token == ' ') {
            *token = '\0';
            token++;
        }
    }
    return argc;
}

// Simple delay
void delay(int count) {
    for (int i = 0; i < count * 100000; i++) {
        __asm__ volatile("nop");
    }
}

// Snake game
#define SNAKE_WIDTH 20
#define SNAKE_HEIGHT 10
#define KEY_UP    'w'
#define KEY_DOWN  's'
#define KEY_LEFT  'a'
#define KEY_RIGHT 'd'
#define KEY_QUIT  'q'

void run_snake() {
    int x = SNAKE_WIDTH/2, y = SNAKE_HEIGHT/2;
    int dir_x = 1, dir_y = 0;
    bool gameover = false;

    while (!gameover) {
        clear_();
        for (int i = 0; i < SNAKE_WIDTH + 2; i++) print("#");
        print("\n");

        for (int row = 0; row < SNAKE_HEIGHT; row++) {
            print("#");
            for (int col = 0; col < SNAKE_WIDTH; col++) {
                if (row == y && col == x) {
                    print("O");
                } else {
                    print(" ");
                }
            }
            print("#\n");
        }

        for (int i = 0; i < SNAKE_WIDTH + 2; i++) print("#");
        print("\nUse WASD to move, q to quit\n");

        char* in = (char*)readStr();
        if (in && in[0] != '\0') {
            char c = in[0];
            if (c == KEY_UP && dir_y != 1) {
                dir_x = 0; dir_y = -1;
            } else if (c == KEY_DOWN && dir_y != -1) {
                dir_x = 0; dir_y = 1;
            } else if (c == KEY_LEFT && dir_x != 1) {
                dir_x = -1; dir_y = 0;
            } else if (c == KEY_RIGHT && dir_x != -1) {
                dir_x = 1; dir_y = 0;
            } else if (c == KEY_QUIT) {
                gameover = true;
                break;
            }
        }

        x += dir_x;
        y += dir_y;

        if (x < 0 || x >= SNAKE_WIDTH || y < 0 || y >= SNAKE_HEIGHT) {
            gameover = true;
        }

        delay(10);
    }

    print("\nGame Over!\n");
}

// Main shell loop
void shell() {
    fs_init();

    char input_buffer[256];
    char* argv[10];
    int argc;

    while (1) {
        print_color(cmdline, 0xf9);
        char* input = (char*)readStr();
        if (input == NULL) continue;

        strncpy(input_buffer, input, sizeof(input_buffer)-1);
        input_buffer[sizeof(input_buffer)-1] = '\0';

        print("\n");

        argc = split_words(input_buffer, argv, 10);
        if (argc == 0) continue;

        char* cmd = argv[0];

        if (strcmp("help", cmd) == 1) help();
        else if (strcmp("clear", cmd) == 1) clear_();
        else if (strcmp("about", cmd) == 1) print("Maya is an x86 based OS\n");
        else if (strcmp("hello", cmd) == 1) print("Hello, user!\n");
        else if (strcmp("version", cmd) == 1) print("Maya OS version 0.1.3\n");
        else if (strcmp("reboot", cmd) == 1) print("Rebooting system... (not really)\n");
        else if (strcmp("time", cmd) == 1) get_time();
        else if (strcmp("date", cmd) == 1) get_date();
        else if (strcmp("author", cmd) == 1) print("Developed by: Your Name Here\n");
        else if (strcmp("joke", cmd) == 1) print("Why did the kernel panic? Because it couldn't handle the pressure!\n");
        else if (strcmp("shutdown", cmd) == 1) shutdown();
        else if (strcmp("snake", cmd) == 1) run_snake();
        else if (strcmp("echo", cmd) == 1) {
            if (argc > 1) {
                for (int i = 1; i < argc; i++) {
                    print(argv[i]);
                    print(" ");
                }
                print("\n");
            } else {
                print("Usage: echo text_to_repeat\n");
            }
        }
        else if (strcmp("ls", cmd) == 1) fs_ls();
        else if (strcmp("cd", cmd) == 1) {
            if (argc > 1) {
                if (!fs_cd(argv[1])) {
                    print("Neuspešan prelazak u direktorijum.\n");
                }
            } else {
                print("Usage: cd directory\n");
            }
        }
        else if (strcmp("touch", cmd) == 1) {
            if (argc > 1) {
                if (fs_create_file(argv[1])) {
                    print("File created.\n");
                } else {
                    print("Failed to create file (exists or limit reached).\n");
                }
            } else {
                print("Usage: touch filename\n");
            }
        }
        else if (strcmp("write", cmd) == 1) {
            if (argc > 2) {
                fs_node_t* file = fs_find_file(argv[1]);
                if (file) {
                    char data[256] = {0};
                    int pos = 0;
                    for (int i = 2; i < argc; i++) {
                        int len = strlen(argv[i]);
                        if (pos + len + 1 < sizeof(data)) {
                            strcpy(&data[pos], argv[i]);
                            pos += len;
                            data[pos++] = ' ';
                        } else {
                            break;
                        }
                    }
                    if (pos > 0) data[pos-1] = '\0';
                    if (fs_write(file, data, strlen(data))) {
                        print("File written.\n");
                    } else {
                        print("Write failed.\n");
                    }
                } else {
                    print("File not found.\n");
                }
            } else {
                print("Usage: write filename text\n");
            }
        }
        else if (strcmp("cat", cmd) == 1) {
            if (argc > 1) {
                fs_node_t* file = fs_find_file(argv[1]);
                if (file) {
                    print(file->content);
                    print("\n");
                } else {
                    print("File not found.\n");
                }
            } else {
                print("Usage: cat filename\n");
            }
        }
        else {
            print("command not found\n");
        }
    }
}
