#ifndef _READIN_H_
#define _READIN_H_

void readerror(char *msg);
void RI_print_lines(char** lines);
char** RI_load_file(char *filename);
char* RI_name(char *line);
int RI_nodes(char *line);
int RI_comps(char *line);

char* RI_line_to_comp(char *line);

#endif