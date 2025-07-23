#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/irq.h"
#include "../include/print.h"
#include "../include/input.h"
#include "../include/about.h"
#include "../user/shell/shell.h"
#include "../user/taskbar/taskbar.h"

void main(){
clear(0xf0);
print_color("Loading BitOS..........................[OK]\n", 0xf9);
print_color("Loading BitOS Kernel..........................[OK]\n", 0xf9);
print_color("Loading BitOS Shell..........................[OK]\n", 0xf9);
irq_install();
timer_install();
keyboard_install();

int i=0;
for(i=0;i<COLS;i++){
print("_");
}

os_about();
taskbar();
shell();
	
}
