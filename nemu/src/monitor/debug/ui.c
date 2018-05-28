#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "cpu/reg.h"
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

/* add more function*/
static int cmd_si(char *args){
	if(args==NULL)
	{
		cpu_exec(1);
	}
	else
	{
		cpu_exec(atoi(args));
	}
	return 0;
}
	
static int cmd_info(char *args){
	if(strcmp(args,"r")==0)
	{
		printf("EAX=0x%x\n",reg_l(0));
		printf("EDX=0x%x\n",reg_l(1));
		printf("ECX=0x%x\n",reg_l(2));
		printf("EBX=0x%x\n",reg_l(3));
		printf("EBP=0x%x\n",reg_l(4));
		printf("ESI=0x%x\n",reg_l(5));
		printf("EDI=0x%x\n",reg_l(6));
		printf("ESP=0x%x\n",reg_l(7));
		printf("EIP=0x%x\n",cpu.eip);
	}
	else
	{
		printf("Please input correct argument for info command\n");
	}
	return 0;
}

static int cmd_x(char *args){
	int num_byte = atoi(strtok(args," "));
	int address = atoi(strtok(NULL," "));
	int i=0;
	int *p;
	p=(int*)address;
	for(i=0;i<num_byte;i++)
	{
		printf("Adress=%x: %x\n",address,*p);
		p++;
	}
	return 0;
}

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },

	/* TODO: Add more commands */
    { "si", "excute single instruction", cmd_si },   
	{ "info","print the values of registers", cmd_info },
	{ "x", "print N bytes at special address", cmd_x },
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
