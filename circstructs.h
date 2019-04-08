#ifndef _CIRCSTRUCTS_H_
#define _CIRCSTRUCTS_H_

typedef struct component component;
typedef struct node node;

struct component {
	int id;
	char type;
	node *ina;
	node *inb;
	node *inc;
	node *outa;
	node *outb;
	node *outc;
};
struct node {
	int id;
	unsigned char activation;
	component* dependent;
};

typedef struct queueitem queueitem;
struct queueitem {
	component *comp; // pointer to the component occupying the place in the queue
	queueitem *next; // pointer to the next queueitem
};

#endif