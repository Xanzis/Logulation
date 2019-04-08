#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

void readerror(char *msg){
	printf("Error in readin.c\nMessage follows:\n%s\n", msg);
	printf("Exiting...\n");
	exit(1)
}

int is_valid_gate(char g){
	char* valid_chars = "AOXNB";
	int valid = 0;
	for (char *c = valid_chars; *c; c++){
		if (g == c){
			valid = 1;
		}
	}
	return valid;
}

void RI_print_lines(char** lines) {
	for( ; *lines; lines++) { printf("%s\n", *lines); }
}

char** RI_load_file(char *filename){
	// Returns a pointer to a null-terminated array of pointers to lines of text.
	// Syntax checking should occur here. <-- lol no
	char **out = NULL;
	char *buff[255];
	char *temp = NULL;
	int pos = 0;
	int line = 0;
	int newlines = 0;
	char c = ' ';
	FILE *f = fopen(filename);
	if (!f) { readerror("fopen failed to return pointer to file."); }

	// First pass - get number of newlines
	c = fgetc(file);
	while(!feof(c)) {
		if (c == '\n') { newlines ++; }
		c = fgetc(file);
	}
	rewind(file);

	out = (char**) malloc((newlines + 2) * sizeof((char*))); // newlines + 1 lines and 1 NULL at end to terminate
	out[newlines + 1] = NULL;

	c = fgetc(file);
	while (!feof(c) && (c != '\0')) {
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
		c = fgetc(file)
	}
	out[line] = NULL; // line incremented since it was last used as an index
	fclose(file);
	return out
}

char* RI_name(char *line){
	char* start = "CIRCUIT";
	char* c = line;
	char* name = NULL;
	// Get to the end of the start
	while(*start) {
		if (start != c) { readerror("Line fed as name definition does not begin with CIRCUIT"); }
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
		if (start != c) { readerror("Line fed as node number definition does not begin with NODES:"); }
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
		if (start != c) { readerror("Line fed as gate number definition does not begin with GATES:"); }
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

char* RI_line_to_comp(char *line){

}