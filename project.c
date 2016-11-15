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

typedef enum { false, true } bool;

/* ----------------------------- DATA STRUCTURES ---------------------------- */
// vertex and edge are used to implement Adjacency List
typedef struct vertexT {
  struct vertexT *next;   // next vertex in the list
  struct edgeT *edges;    // pointer to the edges list

  char label[MAX_LABEL_LENGTH];
  int id;									// integer identificator for internal purpose
	int f;									// time when (DFS) visit finish
	struct setT *setnode;		// pointer to the node in disjoint set
} vertex;

typedef struct edgeT {
  struct vertexT *connectsTo;
  struct edgeT *next;
} edge;

typedef struct graphT {
  struct vertexT *vertices;
} graph;

// disjoint set for SCC management
typedef struct setT {
	struct vertexT *v;	// keep a pointer to the referred vertex in the graph
  int rank;
  struct setT *p;
} set;

// SCC array base element to manage a single SCC of the graph
typedef struct SCCelT {
	struct setT *SCC;
	bool isreached;	// if SCC is reached
	int nreach;			// number of (others) SCC reached
} SCCel;


/* -------------------------- FUNCTIONS DECLARATION ------------------------- */
graph *build_graph_from_stdin();
void add_element_from_dot_line(char *line, graph *G, int *count_vertices);
vertex *add_vertex(graph *G, char *label, int *count_vertices);
void add_edge(vertex *from, vertex *to, graph *G);
void print_graph_in_stdout(graph *G);

// function for disjoint sets
set *make_set(vertex *v);
set *find_set(set *x);
void union_set(set *x, set *y);
void link(set *x, set *y);


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
	int count_vertices = 0; // the counter used to assign an id to every vertex

  G->vertices = NULL; // set empty vertices list

  getline(&line, &len, stdin); // skip first line
  while((read = getline(&line, &len, stdin)) != -1 ) {
    add_element_from_dot_line(line, G, &count_vertices);
  }

  free(line);
  return G;
}

/* @line is a string representing a line of the dot file
 * @G is the graph where to put information retreived from the line
 * @count_vertices is a pointer to the vertices number counter
 */
void add_element_from_dot_line(char *line, graph *G, int *count_vertices) {
  char v1_label[MAX_LABEL_LENGTH], v2_label[MAX_LABEL_LENGTH], current;
  int index=0, charcount=0;
  vertex *v1, *v2;
	bool v1_found=false, v2_found=false;

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
      v1_found = true;
      v1_label[charcount] = '\0';
    }
    if( strlen(v2_label)>0 && !v2_found ) {
      v2_found = true;
      v2_label[charcount] = '\0';
    }
  } while( current!='\n' && current!=';' && !v2_found );

  // add vertices to the graph
  if( v1_found ) {
    v1 = add_vertex(G, v1_label, count_vertices);
  }
  // if found v2 need to add both vertex (v2) and edge (v1->v2)
  if( strlen(v2_label) > 0 ) {
    v2 = add_vertex(G, v2_label, count_vertices);
    add_edge(v1, v2, G);
  }
}

/* @label string for the new vertex label
 * @G is a graph
 * @count_vertices is a pointer to the vertices number counter (used to fill id)
 * RETURN pointer to the vertex just added. if already in graph return the
 					pointer to that (old) vertex
 */
vertex *add_vertex(graph *G, char *label, int *count_vertices) {
  vertex *vertices = G->vertices, *prev_v = NULL;

  // check if exists in graph
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
	v->id = *count_vertices;
  v->next = NULL;
  // if it's the first vertex attach it to G, else to the last vertex in G
  if( prev_v==NULL )
    G->vertices = v;
  else
    prev_v->next = v;

	(*count_vertices)++;
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
    printf("%s (%3d)", v->label, v->id);
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

/* @v is a pointer to a graph vertex
 * RETURN a pointer to the new element of the set
 */
set *make_set(vertex *v) {
  set *x;
  x = (set *) malloc(sizeof(set));
  x->p = x;
  x->v = v;
  x->rank = 0;
  return x;
}

/* find_set() implements find with path compression
 * @x is a pointer to a set element
 * RETURN the set representative in which the element (pointed by) x is
 */
set *find_set(set *x) {
  if( x != x->p ) {
    x->p = find_set( x->p );
  }
  return x->p;
}

/* union_set() implements union by rank
 * @x,@y are pointers to elements of a disjoint set
 * DO an union if x and y aren't in the same set
 */
void union_set(set *x, set *y) {
 link( find_set(x), find_set(y) );
}

void link(set *x, set *y) {
	if( x!=y ) {
		if( x->rank > y->rank ) {
	    y->p = x;
	  } else {
	    x->p = y;
	    if( x->rank == y->rank ) {
	      (y->rank)++;
	    }
	  }
	}
}

/* ---------------------------------- MAIN ---------------------------------- */
void main(int argc, char **argv) {

  graph *G = build_graph_from_stdin();
  print_graph_in_stdout(G);

  exit(1);
}
