#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "circstructs.h"

void readerror(char *msg){
	printf("Error in readin.c\nMessage follows:\n%s\n", msg);
	printf("Exiting...\n");
	exit(1);
}

int is_valid_gate(char g){
	char* valid_chars = "AOXNB";
	int valid = 0;
	for (char *c = valid_chars; *c; c++){
		if (g == *c){
			valid = 1;
		}
	}
	return valid;
}

int get_val(char *c){
	//gets value of a decimal integer string, the beginning of which is pointed to, ending with a space (or \0 terminated)
	int acc = 0;
	while ((*c != ' ') && c) {
		acc *= 10;
		acc += *c - '0';
		c++;
	}
	return acc;
}

char** split_spaces(char *l){
	// l assumed to be null-terminated, free of leading and trailing spaces, free of consecutive spaces
	// First pass - count spaces
	int spaces = 0;
	char *temp = l;
	while (*temp) {
		if (*temp == ' ') spaces++;
		temp++;
	}
	char **out = (char**) malloc((spaces + 2) * sizeof(char*));

	temp = l;
	int i = 0;
	int was_space = 1;
	while (*temp) {
		if ((*temp != ' ') && was_space) {
			out[i] = temp;
			i++;
			was_space = 0;
		}
		if (*temp == ' ') {
			was_space = 1;
		}
		temp++;
	}
	out[spaces + 1] = NULL;
	return out;
}

void RI_print_lines(char** lines) {
	for( ; *lines; lines++) { printf("%s\n", *lines); }
}

char** RI_load_file(char *filename){
	// Returns a pointer to a null-terminated array of pointers to lines of text.
	// Syntax checking should occur here. <-- lol no
	char **out = NULL;
	char buff[255];
	char *temp = NULL;
	int pos = 0;
	int line = 0;
	int newlines = 0;
	char c = ' ';
	FILE *f = fopen(filename, "r");
	if (!f) readerror("fopen failed to return pointer to file.");

	// First pass - get number of newlines
	c = fgetc(f);
	while(!feof(f)) {
		if (c == '\n') { newlines ++; }
		c = fgetc(f);
	}
	rewind(f);

	out = (char**) malloc((newlines + 2) * sizeof(char*)); // newlines + 1 lines and 1 NULL at end to terminate
	out[newlines + 1] = NULL;

	c = fgetc(f);
	while (!feof(f) && (c != '\0')) {
		if (c != '\n') {
			buff[pos] = c;
			pos++;
		}
		else {
			temp = (char*) malloc((pos + 1) * sizeof(char));
			for (int i = 0; i < pos; i++) {
				temp[i] = buff[i];
			}
			temp[pos] = '\0';
			out[line] = temp;
			line++;
			pos = 0;
		}
		c = fgetc(f);
	}
	if (pos) {
		// No trailing newlne (should always be this, actually) - need to allocate the remaining buffer
		temp = (char*) malloc((pos + 1) * sizeof(char));
		for (int i = 0; i < pos; i++) {
			temp[i] = buff[i];
		}
		temp[pos] = '\0';
		out[line] = temp;
	}
	out[newlines + 1] = NULL;
	printf("\nNULL terminated at %d\n", newlines + 1);
	fclose(f);
	return out;
}

char* RI_name(char *line){
	char* start = "CIRCUIT";
	char* c = line;
	char* name = NULL;
	// Get to the end of the start
	while(*start) {
		if (*start != *c) { readerror("Line fed as name definition does not begin with CIRCUIT"); }
		start++;
		c++;
	}
	// Find remaining length
	char *temp = c;
	int len = 0;
	while(*temp) {
		len += 1;
		temp++;
	}
	name = (char*) malloc((len + 1) * sizeof(char));
	int i = 0;
	while(*c) {
		name[i] = *c;
		c++;
		i++;
	}
	name[i] = '\0';
	return name;
}

int RI_nodes(char *line){
	char* start = "NODES:";
	char* c = line;
	char* dec = NULL;

	while(*start) {
		if (*start != *c) { readerror("Line fed as node number definition does not begin with NODES:"); }
		start++;
		c++;
	}

	char *temp = c;
	int len = 0;
	while(*temp) {
		len += 1;
		temp++;
	}
	dec = (char*) malloc((len + 1) * sizeof(char));
	int i = 0;
	while(*c) {
		dec[i] = *c;
		c++;
		i++;
	}
	dec[i] = '\0';
	
	return atoi(dec);
}
int RI_comps(char *line){
	char* start = "GATES:";
	char* c = line;
	char* dec = NULL;

	while(*start) {
		if (*start != *c) { readerror("Line fed as gate number definition does not begin with GATES:"); }
		start++;
		c++;
	}

	char *temp = c;
	int len = 0;
	while(*temp) {
		len += 1;
		temp++;
	}
	dec = (char*) malloc((len + 1) * sizeof(char));
	int i = 0;
	while(*c) {
		dec[i] = *c;
		c++;
		i++;
	}
	dec[i] = '\0';
	
	return atoi(dec);
}

component RI_line_to_comp(char *line, node *nodes, int comp_id){
	if (!is_valid_gate(*line)) readerror("Unsupported gate type");
	component out = (component) {comp_id, *line, NULL, NULL, NULL, NULL, NULL, NULL};
	char **segments = split_spaces(line);
	if (*segments[1] != 'X') out.ina = nodes + get_val(segments[1]);
	if (*segments[2] != 'X') out.inb = nodes + get_val(segments[2]);
	if (*segments[3] != 'X') out.inc = nodes + get_val(segments[3]);
	if (*segments[5] != 'X') out.outa = nodes + get_val(segments[5]);
	if (*segments[6] != 'X') out.outb = nodes + get_val(segments[6]);
	if (*segments[7] != 'X') out.outc = nodes + get_val(segments[7]);
	free(segments);
	return out;
}