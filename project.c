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

/* -------------------------- FUNCTIONS DEFINITION -------------------------- */
/* graph: description in dot file from stdin
 * OUT: graph filled with dot file information
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

/* line: is a string representing a line of the dot file
 * G: is the graph where to put information retreived from the line
 */
void add_element_from_dot_line(char *line, graph *G) {
  char *v1_label, *v2_label, current;
  int index=0, v1_found=0, charcount=0;
  vertex *v1, *v2;

  do {
    current = line[index];
    while( isalnum(current) ) {
      charcount = 0;
      if( !v1_found ) {
        v1_label[charcount++] = current;
      } else {
        v2_label[charcount++] = current;
      }
      current = line[++index];
    }
    if( strlen(v1_label) > 0 ) {
        v1_found = 1;
        v1_label[charcount] = '\0';
    }
    if( strlen(v2_label) > 0 ) {
        v2_label[charcount] = '\0';
    }
  } while( current != '\n' && current != ';' );

  if( v1_found ) {
    // v1 = add_vertex(v1_label, G);
  }
  if( strlen(v2_label) > 0 ) {
    // v2 = add_vertex(v2_label, G);
    // add_edge(v1, v2, G);
  }
}

/* label: string for the new vertex label
 * G: is a graph
 * OUT: pointer to the vertex just added. if already in graph return the pointer
 *      to that (old) vertex
 */
vertex *add_vertex(char *label, graph *G) {
  vertex *vertices = G->vertices;

  // check if present in graph
  if( vertices != NULL ) {
    while( vertices->next!=NULL ) {
      if(strcmp(vertices->label, label) == 0) {
        return vertices;
      }
      vertices = vertices->next;
    }
  }

  // if not present create vertex, fill it with appropriate information, and add
  vertex *v = (vertex *) malloc(sizeof(vertex));
  strcpy(v->label, label);
  v->next = NULL;
  vertices->next = v;

  return v;
}

/* from,to: needed to be pointers to vertices already in G
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

/* ---------------------------------- MAIN ---------------------------------- */
void main(int argc, char **argv) {
  vertex *test;

  graph *G;
  // vertex *v1, *v2;
  // v1 = (vertex *) malloc( sizeof(vertex *) );
  // v2 = (vertex *) malloc( sizeof(vertex *) );
  // v1->next = v2;
  // v1->edges = NULL;
  // strcpy(v1->label, "TEST1");
  // v1->id = 1;
  //
  // v2->next = NULL;
  // v2->edges = NULL;
  // strcpy(v2->label, "TEST2");
  // v2->id = 2;
  //
  // G->vertices = v1;
  //
  // vertex *v3 = (vertex *) malloc(sizeof(vertex *));
  // v3->next = NULL;
  // v3->id = 3;
  // strcpy(v3->label, "TEST3");

  //add_vertex(v3, G);

  G = build_graph_from_stdin();

  // for( vertex *v = G->vertices; v!=NULL; v = v->next ) {
  //   printf("%s connects to:", v->label);
  //   for( edge *e = v->edges; e!=NULL; e=e->next ) {
  //     printf(" %s", e->connectsTo->label);
  //   }
  //   printf("\n");
  // }

  exit(1);
}
