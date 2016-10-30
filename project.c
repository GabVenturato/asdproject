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
  int id;
} vertex;

typedef struct edgeT {
  struct vertexT *connectsTo;
  struct edgeT *next;
} edge;

typedef struct graphT {
  struct vertexT *vertices;
} graph;


/* -------------------------- FUNCTIONS DECLARATION ------------------------- */
graph build_graph_from_stdin();
graph add_element_from_dot_line(char *line, vertex *graphtofill);

/* -------------------------- FUNCTIONS DEFINITION -------------------------- */
/*
graph *build_graph_from_stdin() {
  graph G = (graph) malloc(sizeof(graph));
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  G->vertices = NULL; // set empty vertices list

  getline(&line, &len, stdin); // skip first line
  while((read = getline(&line, &len, stdin)) != -1 ) {
    graph = add_element_from_dot_line(line, graph);
  }

  free(line);
  return G;
}

vertex *add_element_from_dot_line(char *line, vertex *graphtofill) {
  char *vertex_one, *vertex_two, current;
  int index = 0, vertex_one_found = 0;
  do {
    current = line[index];
    while( isalnum(current) ) {
      int charcount = 0;
      if( !vertex_one_found ) {
        vertex_one[charcount++] = current;
      } else {
        vertex_two[charcount++] = current;
      }
      current = line[++index];
    }
    if( strlen(vertex_one) > 0 ) {
        vertex_one_found = 1;
    }
  } while( current != '\n' && current != ';' );

  if( vertex_one_found ) {

  }
  return graphtofill;
}
*/

void add_vertix(vertex *v, graph *G) {
  vertex *vertices = G->vertices;

  if( vertices != NULL )
    while( vertices->next != NULL ) { vertices = vertices->next; }

  vertices->next = v;
}

/* ---------------------------------- MAIN ---------------------------------- */
void main(int argc, char **argv) {
  vertex *test;

  graph *G = (graph *) malloc( sizeof(graph) );
  vertex *v1, *v2;
  v1 = (vertex *) malloc( sizeof(vertex *) );
  v2 = (vertex *) malloc( sizeof(vertex *) );
  v1->next = v2;
  v1->edges = NULL;
  strcpy(v1->label, "TEST1");
  v1->id = 1;

  v2->next = NULL;
  v2->edges = NULL;
  strcpy(v2->label, "TEST2");
  v2->id = 2;

  G->vertices = v1;

  vertex *v3 = (vertex *) malloc(sizeof(vertex *));
  v3->next = NULL;
  v3->id = 3;
  strcpy(v3->label, "TEST3");

  //add_vertix(v3, G);

  for( vertex *v = G->vertices; v!=NULL; v = v->next ) {
    printf("Vertice %s il cui id è: %d\n", v->label, v->id);
  }

  exit(1);
}
