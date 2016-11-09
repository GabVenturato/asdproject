/* Gabriele Venturato (125512)
 * Progetto del laboratorio di ASD anno 2015/2016
 */

/* ctype.h - used to parse the input
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LABEL_LENGTH 1024

/* ----------------------------- DATA STRUCTURES ---------------------------- */
// vertex and edge are used to implement Adjacency List
typedef struct vertexT {
  struct vertexT *next;   // next vertex in the list
  struct edgeT *edges;    // pointer to the edges list

  char label[MAX_LABEL_LENGTH];
  int id; // not used (?)
} vertex;

typedef struct edgeT {
  struct vertexT *connectsTo;
  struct edgeT *next;
} edge;

typedef struct graphT {
  struct vertexT *vertices;
} graph;


/* -------------------------- FUNCTIONS DECLARATION ------------------------- */
graph *build_graph_from_stdin();
void add_element_from_dot_line(char *line, graph *G);
vertex *add_vertex(char *label, graph *G);
void add_edge(vertex *from, vertex *to, graph *G);
void print_graph_in_stdout(graph *G);

/* -------------------------- FUNCTIONS DEFINITION -------------------------- */
/* @graph description in dot file from stdin
 * RETURN graph filled with dot file information
 * The dot file is needed to be in a specific format. (see project assignment
 * description).
 */
graph *build_graph_from_stdin() {
  graph *G = (graph *) malloc(sizeof(graph));
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  G->vertices = NULL; // set empty vertices list

  getline(&line, &len, stdin); // skip first line
  while((read = getline(&line, &len, stdin)) != -1 ) {
    add_element_from_dot_line(line, G);
  }

  free(line);
  return G;
}

/* @line is a string representing a line of the dot file
 * @G is the graph where to put information retreived from the line
 */
void add_element_from_dot_line(char *line, graph *G) {
  char v1_label[MAX_LABEL_LENGTH], v2_label[MAX_LABEL_LENGTH], current;
  int index=0, v1_found=0, v2_found=0, charcount=0;
  vertex *v1, *v2;

  // clean labels
  v1_label[0] = '\0';
  v2_label[0] = '\0';

  do { // check line character per character
    current = line[index++];
    charcount = 0; // clean label length counter
    // if there's alphanumeric char start to save the label
    while( isalnum(current) ) {
      if( !v1_found ) { // save in v1 if it is first word found, else in v2
        v1_label[charcount++] = current;
      } else {
        v2_label[charcount++] = current;
      }
      current = line[index++];
    }
    // set found flags and close strings
    if( strlen(v1_label)>0 && !v1_found ) {
      v1_found = 1;
      v1_label[charcount] = '\0';
    }
    if( strlen(v2_label)>0 && !v2_found ) {
      v2_found = 1;
      v2_label[charcount] = '\0';
    }
  } while( current!='\n' && current!=';' && !v2_found );

  // add vertices to the graph
  if( v1_found ) {
    v1 = add_vertex(v1_label, G);
  }
  // if found v2 need to add both vertex (v2) and edge (v1->v2)
  if( strlen(v2_label) > 0 ) {
    v2 = add_vertex(v2_label, G);
    add_edge(v1, v2, G);
  }
}

/* @label string for the new vertex label
 * @G is a graph
 * RETURN pointer to the vertex just added. if already in graph return the pointer
 * 				to that (old) vertex
 */
vertex *add_vertex(char *label, graph *G) {
  vertex *vertices = G->vertices, *prev_v = NULL;

  // check if present in graph
  while( vertices!=NULL ) {
    if(strcmp(vertices->label, label) == 0) {
      return vertices;
    }
    prev_v = vertices;
    vertices = vertices->next;
  }

  // if not present create vertex, fill it with appropriate information, and add
  vertex *v = (vertex *) malloc(sizeof(vertex));
  strcpy(v->label, label);
  v->next = NULL;
  // if it's the first vertex attach it to G, else to the last vertex in G
  if( prev_v==NULL )
    G->vertices = v;
  else
    prev_v->next = v;

  return v;
}

/* @from, @to needed to be pointers to vertices already in G
 */
void add_edge(vertex *from, vertex *to, graph *G) {
  vertex *vertices = G->vertices;

  while( vertices!=NULL && vertices!=from ) { vertices = vertices->next; }

  if( vertices==from ) { // if the vertex exists in the graph G
    // create the new edge and add it in head of edges list
    edge *oldedges = vertices->edges;
    edge *newedge = (edge *) malloc(sizeof(edge));
    newedge->connectsTo = to;
    newedge->next = oldedges;
    vertices->edges = newedge;
  }
}

/* @G is a graph
 * PRINT in stdout the graph in dot format
 */
void print_graph_in_stdout(graph *G) {
	printf("digraph out {\n");
  for( vertex *v = G->vertices; v!=NULL; v=v->next ) {
    printf("%s", v->label);
		edge *e = v->edges;
		if( e!=NULL ) {
			printf(" ->");
			while( e!=NULL ) {
	      printf(" %s", e->connectsTo->label);
				if( e->next != NULL ) printf(",");
				e=e->next;
	    }
		}
    printf(";\n");
  }
	printf("}\n");
}

/* ---------------------------------- MAIN ---------------------------------- */
void main(int argc, char **argv) {

  graph *G = build_graph_from_stdin();
  print_graph_in_stdout(G);

  exit(1);
}
