#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "circstructs.h"
#include "readin.h"

char *g_name;
node *g_nodes;
component *g_components;
component g_current_component;
queueitem *g_changep; // pointer to the change queue
int g_N_nodes;
int g_N_comps;

char* b8(char val) {
	static char b[9] = "00000000\0";
	char x = val;
	for (int i = 7; i > 0; i--) {
		b[i] = '0' + (x&1);
		x >>= 1;
	}
	return b;
}

void print_node(node *a) {
	//printf("Node #%d at address %p. Val: %s. Dependent address: %p\n", a->id, (void*) a, b8(a->activation), (void*) a->dependent);
	int d = (a->dependent == NULL) ? 0 : (a->dependent)->id;
	printf("Node #%3d at address %p. Val: %s. Dependent id: %3d\n", a->id, (void*) a, b8(a->activation), d);
}

void print_component(component *a) {
	// Looks like pointer references are stabilizing - switching to id readouts for better readability
	//printf("Component #%d at address %p. Type %c. Ins: %p %p %p. Outs: %p %p %p \n", 
	//	a->id, (void*) a, a->type, (void*) a->ina, (void*) a->inb, (void*) a->inc, (void*) a->outa, (void*) a->outb, (void*) a->outc);
	int inaid = (a->ina == NULL) ? 0 : (a->ina)->id;
	int inbid = (a->inb == NULL) ? 0 : (a->inb)->id;
	int incid = (a->inc == NULL) ? 0 : (a->inc)->id;
	int outaid = (a->outa == NULL) ? 0: (a->outa)->id;
	int outbid = (a->outb == NULL) ? 0: (a->outb)->id;
	int outcid = (a->outc == NULL) ? 0: (a->outc)->id;
	printf("Component #%3d at address %p. Type %c. Ins: %d %d %d. Outs: %d %d %d \n", 
		a->id, (void*) a, a->type, inaid, inbid, incid, outaid, outbid, outcid);
}

void print_queue(queueitem *qp) {
	printf("Begin queue readout (queue beginning at %p):\n", (void*) qp);
	while (qp != NULL) {
		print_component(qp->comp);
		qp = qp->next;
	}
	printf("End queue readout\n");
}

void print_nodevals() {
	for (int i = 0; i < g_N_nodes; i++) {
		printf("ID %3d %s\n", g_nodes[i].id, b8(g_nodes[i].activation));
	}
}

void simerror(char *msg) {
	printf("Upper level simulation error. Error message follows:\n%s\n", msg);
	printf("Exiting...\n");
	exit(1);
}

void component_error(component *c, char *msg) {
	printf("Component-related error. Component data:\n");
	print_component(c);
	printf("Error message follows:\n%s\n", msg);
	printf("Exiting...\n");
	exit(1);
}

void node_error(node *n, char *msg) {
	printf("Node-related error. Node data:\n");
	print_node(n);
	printf("Error message follows:\n%s\n", msg);
	printf("Exiting...\n");
	exit(1);
}

queueitem* next_component(queueitem *queuepointer) {
	if (queuepointer == NULL) {
		//Queue is empty. Return NULL queuepointer and set global component current_component to NULL
		g_current_component = (component) {0, '\0', NULL, NULL, NULL, NULL, NULL, NULL};
		return NULL;
	}
	// This'll be a little hard to follow, but queueitem is a perfectly serviceable struct for returning both
	// 	the component pointer and the pointer to the new start of the linked list
	queueitem *qp = queuepointer->next;
	g_current_component = *(queuepointer->comp);
	free(queuepointer);
	return qp;
}

queueitem* queue_component(queueitem *queuepointer, component *compp) {
	// Add a component to the end of the queue iff that component is not already present
	if (!queuepointer) {
		queuepointer = (queueitem*) malloc(sizeof(queueitem));
		if(queuepointer == NULL) simerror("Memory error allocating next queueitem\n");
		*queuepointer = (queueitem) {compp, NULL};
		return queuepointer;
	}
	queueitem *qp = queuepointer;
	int not_present = 1;
	while(queuepointer->next) {
		if (queuepointer->comp == compp) not_present = 0; // Need to check if the pointers are properly compared here.
		// Switching to id-based identification may be a solution if this fails
		queuepointer = queuepointer->next;
	}
	if (queuepointer->comp == compp) not_present = 0; // To check the last one it stops on
	if (not_present) {
		// Only append to linked list if the component was not already listed
		// Have now reached the end of the queue. allocate memory for the new item and link from the last node.
		queueitem *newitem = (queueitem*) malloc(sizeof(queueitem));
		if(queuepointer == NULL) simerror("Memory error allocating next queueitem\n");
		*newitem = (queueitem) {compp, NULL};
		queuepointer->next = newitem;
	}
	// Beginning of list remains unchanged - return it
	return qp;
}

void queue_all() {
	//Adds all components to queue
	for (int i = 0; i < g_N_comps; i++) {
		g_changep = queue_component(g_changep, &g_components[i]);
	}
}

unsigned char f_AND(unsigned char a, unsigned char b, unsigned char c) {
	return (a & b) & c;
}

unsigned char f_OR(unsigned char a, unsigned char b, unsigned char c) {
	return (a | b) | c;
}

unsigned char f_XOR(unsigned char a, unsigned char b, unsigned char c) {
	// Returns 1 for an odd number of on inputs
	return (a ^ b) ^ c;
}

node** feed(component gate, node *vals, int verbosity) {
	// Propagates inputs through an AND gate. Returns a pointer to the changed nodes, terminated by a NULL node.
	unsigned char a, b, c;
	unsigned char pass;

	switch (gate.type) {
		case 'A':
			pass = 255;
			break;
		case 'B':
			pass = 255;
			break;
		default:
			pass = 0;
	}

	// Non-interfering third byte differs between gate types
	a = (gate.ina == NULL) ? pass : gate.ina->activation;
	b = (gate.inb == NULL) ? pass : gate.inb->activation;
	c = (gate.inc == NULL) ? pass : gate.inc->activation;

	unsigned char res;

	switch (gate.type) {
		case 'A':
			res = f_AND(a, b, c);
			break;
		case 'O':
			res = f_OR(a, b, c);
			break;
		case 'X':
			res = f_XOR(a, b, c);
			break;
		case 'N':
			//NOR
			res = ~f_OR(a, b, c);
			break;
		case 'B':
			//NAND
			res = ~f_AND(a, b, c);
		default:
			component_error(&gate, "Unsupported gate type\n");
	}

	if (verbosity) { printf("Ins %d %d %d Out %d\n", a, b, c, res); }

	node **changed_locs = (node**) malloc(3 * sizeof(node*));
	if (changed_locs == NULL) simerror("Memory error allocating changed_locs\n");
	for (int i = 0; i <= 2; i++) {
		changed_locs[i] = NULL;
	}
	int num_changed = 0;

	if (gate.outa != NULL) {
		if (gate.outa->activation != res) {
			changed_locs[0] = gate.outa;
			gate.outa->activation = res;
			num_changed++;
		}
	}
	if (gate.outb != NULL) {
		if (gate.outb->activation != res) {
			changed_locs[1] = gate.outb;
			gate.outb->activation = res;
			num_changed++;
		}
	}
	if (gate.outc != NULL) {
		if (gate.outc->activation != res) {
			changed_locs[2] = gate.outc;
			gate.outc->activation = res;
			num_changed++;
		}
	}

	node **out = (node**) malloc((num_changed + 1) * sizeof(node*));
	if (out == NULL) simerror("Memory error allocating out\n");
	int loc = 0;
	for (int i = 0; i <= 2; i++) {
		if (changed_locs[i] != NULL) {
			out[loc] = changed_locs[i];
			loc++;
		}
	}
	free(changed_locs);
	out[num_changed] = NULL;
	return out;
}

void set_node(node *nodep, char val) {
	// Sets a single node value, updating the change queue if necessary
	component *compp = NULL;
	if (nodep->activation != val) {
		nodep->activation = val;
		compp = nodep->dependent;
	}
	if (compp != NULL) {
		g_changep = queue_component(g_changep, compp);
	}
}

int propagate_change(int verbosity) {
	// Retrieves the next queued change and propogates it through the dependent gate, if applicable
	// Pushes to change queue as necessary
	// Returns nonzero if action was performed, 0 if queue is empty
	if (g_changep == NULL) {printf(""); return 0;}
	g_changep = next_component(g_changep);
	component curcomp = g_current_component;
	node **changes;

	if (verbosity) {
	printf("Propagating through --- ");
	print_component(&curcomp);
	}

	changes = feed(curcomp, g_nodes, verbosity);

	// Follow the pointers in changes, already guaranteed by the feed function to be changed nodes
	for(int i = 0; changes[i] != NULL; i++) {
		if (changes[i]->dependent != NULL) {
			// Won't queue dependents if node has no dependents
			if (verbosity) { 
				printf("Queueing --- ");
				print_component(changes[i]->dependent);
			}
			g_changep = queue_component(g_changep, changes[i]->dependent);
			if (verbosity) { print_queue(g_changep); }
		}
	}
	free(changes);
	return 1;
}

int run_until_stable(int verbosity) {
	// Returns number of iterations to stabilize
	int i = 0;
	int success = propagate_change(verbosity);
	if (!success) {
		printf("propagate_change() found empty queue on first iteration\n");
		return 0;
	}
	while (success) {
		success = propagate_change(verbosity);
		i++;
	}
	return i;
}

void setup() {
	printf("Setting up... ");

	char **circ_file = RI_load_file("circuit.lgl");

	g_name = RI_name(circ_file[0]);
	g_N_nodes = RI_nodes(circ_file[1]);
	g_N_comps = RI_comps(circ_file[2]);
	free(circ_file[0]);
	free(circ_file[1]);
	free(circ_file[2]);

	//g_N_nodes = 16;
	//g_N_comps = 9; // Near future - get these value from the component structure that the program is loading from

	g_nodes = (node*) malloc(g_N_nodes * sizeof(node));
	if (g_nodes == NULL) { simerror("Memory error allocating nodes\n"); }
	g_components = (component*) malloc((g_N_comps + 1) * sizeof(component));
	if (g_components == NULL) { simerror("Memory error allocating components\n"); }

	g_current_component = (component) {0, '\0', NULL, NULL, NULL, NULL, NULL, NULL};
	g_changep = NULL;

	// Build component array

	for (int i = 3; circ_file[i]; i++) {
		g_components[i-3] = RI_line_to_comp(circ_file[i], g_nodes, i-2);
		free(circ_file[i]);
	}
	free(circ_file);

	/*
	g_components[0] = (component) {1, 'O', &g_nodes[0], NULL, NULL, &g_nodes[3], &g_nodes[4], NULL};
	g_components[1] = (component) {2, 'O', &g_nodes[1], NULL, NULL, &g_nodes[5], &g_nodes[6], NULL};
	g_components[2] = (component) {3, 'O', &g_nodes[2], NULL, NULL, &g_nodes[7], &g_nodes[8], NULL};
	g_components[3] = (component) {4, 'X', &g_nodes[3], &g_nodes[5], NULL, &g_nodes[9], NULL, NULL};
	g_components[4] = (component) {5, 'A', &g_nodes[4], &g_nodes[6], NULL, &g_nodes[13], NULL, NULL};
	g_components[5] = (component) {6, 'O', &g_nodes[9], NULL, NULL, &g_nodes[10], &g_nodes[11], NULL};
	g_components[6] = (component) {7, 'X', &g_nodes[7], &g_nodes[10], NULL, &g_nodes[14], NULL, NULL};
	g_components[7] = (component) {8, 'A', &g_nodes[8], &g_nodes[11], NULL, &g_nodes[12], NULL, NULL};
	g_components[8] = (component) {9, 'O', &g_nodes[12], &g_nodes[13], NULL, &g_nodes[15], NULL, NULL};
	*/

	// Initialize node values
	// Pulling all to 0 should be fine even in cooler cases
	for (int i = 0; i < g_N_nodes; i++) {
		g_nodes[i].id = i + 1;
		g_nodes[i].activation = 0;
		g_nodes[i].dependent = NULL;
	}

	// Fill out dependency table
	for (int i = 0; i < g_N_comps; i++) {
		if (g_components[i].ina != NULL) { 
			g_components[i].ina->dependent = &g_components[i]; 
		}
		if (g_components[i].inb != NULL) { 
			g_components[i].inb->dependent = &g_components[i]; 
		}
		if (g_components[i].inc != NULL) { 
			g_components[i].inc->dependent = &g_components[i]; 
		}
	}

	printf("Structure loaded.\n");

	printf("Propagating all gates from 0... ");
	queue_all();
	int iters = run_until_stable(0);
	printf("Stability in %d iterations.\n", iters);

	printf("Nodes and components as initialized:\n");
	for (int i = 0; i < g_N_nodes; i++) { print_node(&g_nodes[i]); }
	for (int i = 0; i < g_N_comps; i++) { print_component(&g_components[i]); }
	printf("End setup\n");
}

int main() {
	setup();

	int iters = 0;

	set_node(&g_nodes[0], 0);
	set_node(&g_nodes[1], 1);
	set_node(&g_nodes[2], 0);
	printf("Turned on nodes\n");

	print_nodevals();
	iters = run_until_stable(1);
	printf("%d\n", iters);
	print_nodevals();

	return 1;
}



