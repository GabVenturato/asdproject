/* Gabriele Venturato (125512)
 * Progetto del laboratorio di ASD anno 2015/2016
 */

// ctype.h - used to parse the input (isalnum function)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_LABEL_LENGTH 1024
#define DEFAULT_EDGE_COLOR ""
#define DEFAULT_EDGE_STYLE ""

typedef enum { false, true } bool;

/* ----------------------------- DATA STRUCTURES ---------------------------- */
// global variables
int totvertices = 0;			// stores the number of total vertices in the graph

// vertex and edge are used to implement Adjacency List
typedef struct vertex_T {
  struct vertex_T *next;	// next vertex in the list
  struct edge_T *edges;		// pointer to the edges list
	struct edge_T *tedges;	// pointer to the transposed edges list

  char label[MAX_LABEL_LENGTH];
  int id;									// integer identificator for internal purpose
	int depth;							// depth of the node (in the tree created later)
	struct scc_T *sccref;		// reference to the scc which the vertex belongs
	bool isroot;						// if it is the root that solve the problem
} vertex;

typedef struct edge_T {
  struct vertex_T *connectsTo;
  struct edge_T *next;
	char color[32];					// to manage the color in output .dot file
	char style[32];					// to manage the style in output .dot file
} edge;

typedef struct graph_T {
  struct vertex_T *vertices;
} graph;

// SCC management
typedef struct scc_T {
	struct vertex_T *root;
	bool isreached;					// if SCC is reached
	int nreach;							// number of (others) SCC reached
	struct scc_T *next;
} scc;

typedef struct sccset_T {
	struct scc_T *sccomponents;
} sccset;

// list of vertices
typedef struct vlist_T {
	struct vertex_T *v;			// pointer to a graph vertex
	struct vlist_T *next;
} vlist;


/* -------------------------- FUNCTIONS DECLARATION ------------------------- */
graph *build_graph_from_stdin();
void add_element_from_dot_line(char *line, graph *G);
vertex *add_vertex(graph *G, char *label);
void add_edge(vertex *from, vertex *to, graph *G, char *color, char *style);
void print_graph_in_stdout(graph *G);

// function for disjoint sets
// set *make_set(vertex *v);
// set *find_set(set *x);
// void union_set(set *x, set *y);
// void link(set *x, set *y);

// graph visits
vlist *DFS(graph *G);
vlist *DFS_visit(graph *G, vertex *u, bool *visited, vlist *ftimevertices);
sccset *DFS_SCC(graph *Gt, vlist *ftimevertices);
void DFS_SCC_visit(graph *Gt, vertex *u, bool *visited, scc *sccref);

// vertices list management
vlist *vlist_push(vlist *l, vertex *v);
vertex *vlist_pop(vlist *l);
void vlist_print(vlist *l);

// graph transposing
void transpose_graph(graph *G);
void print_transposed_graph_in_stdout(graph *G);

/* -------------------------- FUNCTIONS DEFINITION -------------------------- */
/* STDIN description in dot file from stdin
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
 * this function scan character per character the line passed as parameter, when
 * it founds an alphanumeric char start to store a string for v1 (first vertex),
 * continue while the first non-alphanumeric character is found. do the same for
 * v2 (second vertex, if present). if v2 exists it needs to create also the
 * edge. this function works only with the input format specified in project
 * assignment.
 */
void add_element_from_dot_line(char *line, graph *G) {
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
    v1 = add_vertex(G, v1_label);

		// if found v2 need to add both vertex (v2) and edge (v1->v2)
	  if( strlen(v2_label) > 0 ) {
	    v2 = add_vertex(G, v2_label);
	    add_edge(v1, v2, G, DEFAULT_EDGE_COLOR, DEFAULT_EDGE_STYLE);
	  }
  }
}

/* @G is a graph
 * @label string for the new vertex label
 * RETURN pointer to the vertex just added. if already in graph return the
 *        pointer to that (old) vertex
 * NOTE the unicity of the identifier is granted from the global variable
 *      "totvertices". identifier unicity not cheked in this function.
 */
vertex *add_vertex(graph *G, char *label) {
  vertex *vertices = G->vertices;

  // check if exists in graph
  while( vertices!=NULL ) {
    if(strcmp(vertices->label, label) == 0) {
      return vertices;
    }
    vertices = vertices->next;
  }

  // if not present create vertex, fill it with appropriate information, and add
  vertex *v = (vertex *) malloc(sizeof(vertex));
  strcpy(v->label, label);
	v->id = totvertices; // use the current number of total edges as identifier
  // attach it to G, at the top of vertices list
	v->next = G->vertices;
	G->vertices = v;
	v->isroot = false;
	totvertices++;
  return v;
}

/* @from, @to needed to be pointers to vertices already inserted in G
 * @G is a graph
 * @color is a string that defines the edge color (REQUIRED length<32)
 * @style is a string that defines the edge style (REQUIRED length<32)
 * NOTE the function doesn't check for duplicated edges because of the
 *      assumption that in the dot input file cannot be duplicated edges
 */
void add_edge(vertex *from, vertex *to, graph *G, char *color, char *style) {
  // create the new edge and add it in head of edges list
  edge *newedge = (edge *) malloc(sizeof(edge));
  newedge->connectsTo = to;
  newedge->next = from->edges;
	strcpy(newedge->color, color);
	strcpy(newedge->style, style);
  from->edges = newedge;
}

/* @G is a graph
 * PRINT in stdout the graph in dot format
 */
void print_graph_in_stdout(graph *G) {
	printf("digraph out {\n");
  for( vertex *v = G->vertices; v!=NULL; v=v->next ) {
    printf("%s", v->label, v->id);
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
// set *make_set(vertex *v) {
//   set *x;
//   x = (set *) malloc(sizeof(set));
//   x->p = x;
//   x->v = v;
// 	v->setnode = x;	// save in the vertex in which element of the set it is saved
//   x->rank = 0;
//   return x;
// }

/* find_set() implements find with path compression
 * @x is a pointer to a set element
 * RETURN the set representative in which the element (pointed by) x is
 */
// set *find_set(set *x) {
//   if( x != x->p ) {
//     x->p = find_set( x->p );
//   }
//   return x->p;
// }

/* union_set() implements union by rank
 * @x,@y are pointers to elements of a disjoint set
 * this function do an union if x and y aren't in the same set
 * link() is used only here by union_set()
 */
// void union_set(set *x, set *y) {
//  link( find_set(x), find_set(y) );
// }
// void link(set *x, set *y) {
// 	if( x!=y ) {
// 		if( x->rank > y->rank ) {
// 	    y->p = x;
// 	  } else {
// 	    x->p = y;
// 	    if( x->rank == y->rank ) {
// 	      (y->rank)++;
// 	    }
// 	  }
// 	}
// }

/* @G is a graph
 * RETURN the list of vertices ordered by finish visit time
 * this function perform a DFS visit in order to order graph vertices by finish
 * visit time. more details in DFS_visit() description.
 * DFS_visit() is used only here to perform the DFS visit
 */
vlist *DFS(graph *G) {
	bool visited[totvertices];		// keep track of visited vertices (here use id)
	vlist *ftimevertices = NULL;	// list to fill with vertices
	vertex *u;

	// set all vertices as "not visited"
	for(int i=0; i<totvertices; i++) {
		visited[i] = false;
	}

	u = G->vertices;
	while( u!=NULL ) {
		if( !visited[u->id] ) {
			ftimevertices = DFS_visit(G, u, visited, ftimevertices);
		}
		u = u->next;
	}

	return ftimevertices;
}

/* @G is a graph
 * @u is a vertex currently analized by the function
 * @visited pointer to the visited array that keep track of visited vertices
 * @ftimevertices is the list to fill with vertices in appropriate order
 * this function is used to check the finish time of the visit in each node and
 * store them in a list that will be used to find SCC of the graph
 */
vlist *DFS_visit(graph *G, vertex *u, bool *visited, vlist *ftimevertices) {
	visited[u->id] = true;

	for(edge *e=u->edges; e!=NULL; e=e->next) {
		vertex *v = e->connectsTo;
		if( !visited[v->id] ) {
			ftimevertices = DFS_visit(G, v, visited, ftimevertices);
		}
	}

	// every time the visit in u finishes, store the vertex in the list and return
	// the updated list
	return vlist_push( ftimevertices, u );
}

sccset *DFS_SCC(graph *Gt, vlist *ftimevertices) {
	bool visited[totvertices];		// keep track of visited vertices (here use id)
	sccset *SCCset = (sccset *) malloc(sizeof(sccset));
	SCCset->sccomponents = NULL;
	vertex *u;

	// set all vertices as "not visited"
	for(int i=0; i<totvertices; i++) {
		visited[i] = false;
	}

	vlist *li = ftimevertices;	// use the list of vertices ordered by finish visit time
	while( li!=NULL ) {
		u = li->v;
		if( !visited[u->id] ) {
			scc *s = (scc *) malloc(sizeof(scc));
			s->root = u;
			u->sccref = s;
			s->next = SCCset->sccomponents;
			SCCset->sccomponents = s;
			DFS_SCC_visit(Gt, u, visited, s);
		}
		li = li->next;
	}

	return SCCset;
}

void DFS_SCC_visit(graph *Gt, vertex *u, bool *visited, scc *sccref) {
	visited[u->id] = true;

	for(edge *e=u->tedges; e!=NULL; e=e->next) {
		vertex *v = e->connectsTo;
		if( !visited[v->id] ) {
			v->sccref = sccref;
			DFS_SCC_visit(Gt, v, visited, sccref);
		}
	}
}

/* @l is a pointer to the head of the list
 * @v is a pointer to a graph vertex
 * RETURN the list with the new element added
 * this function insert the element in the head of the list
 */
vlist *vlist_push(vlist *l, vertex *v) {
	vlist *el;
	el = (vlist *) malloc(sizeof(vlist));
	el->v = v;
	el->next = l;
	return el;
}

/* @l is a pointer to the head of the list
 * RETURN a pointer to the vertex extracted
 * this function extract elements from the head
 */
// vertex *vlist_pop(vlist *l) {
// 	vertex *v = NULL;
// 	vlist* tmp;
// 	if( l!=NULL ) {
// 		v = l->v;
// 		tmp = l;
// 		l = l->next;
// 	}
// 	free(tmp);	// free memory occupied from removed element
// 	return v;
// }

/* @l is a pointer to the head of the list
 */
void vlist_print(vlist *l) {
	for(vlist *p=l; p!=NULL; p=p->next) {
		printf("%s(%d) ", (p->v)->label, (p->v)->id);
	}
	printf("\n");
}

/* @G is a graph
 * transpose a graph and save the transposed edges into the list "tedges" stored
 * in every vertex
 */
void transpose_graph(graph *G) {
	for(vertex *v=G->vertices; v!=NULL; v=v->next) {
		for(edge *e=v->edges; e!=NULL; e=e->next) {
			vertex *u = e->connectsTo;
			// create the new edge (u,v) and add it in head of u transposed edges list
		  edge *newedge = (edge *) malloc(sizeof(edge));
		  newedge->connectsTo = v;
		  newedge->next = u->tedges;
		  u->tedges = newedge;
		}
	}
}

/* @G is a graph
 * like print_graph_in_stdout() but use the transposed edges list to print the
 * transposed graph
 */
void print_transposed_graph_in_stdout(graph *G) {
	printf("digraph out {\n");
  for( vertex *v = G->vertices; v!=NULL; v=v->next ) {
    printf("%s (%3d)", v->label, v->id);
		edge *e = v->tedges;
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

void scc_reachability(graph *G) {
	scc *i, *j;

	for(vertex *v=G->vertices; v!=NULL; v=v->next) {
		i = v->sccref;
		for(edge *e=v->edges; e!=NULL; e=e->next) {
			vertex *u = e->connectsTo;
			j = u->sccref;

			if( i!=j ) {
				j->isreached = true;
				(i->nreach)++;
			}
		}
	}
}

vertex *add_missing_edges(graph *G) {
	vertex *root, *v;
	vlist *scc_not_reached, *li;
	int maxnreach=0;
	scc *s;

	for(vertex *v=G->vertices; v!=NULL; v=v->next) {
		s = v->sccref;
		if( s->nreach > maxnreach ) {
			maxnreach = s->nreach;
			root = v;
		}
		if( !s->isreached ) {
			vlist_push(scc_not_reached, v);
		}
	}

	li = scc_not_reached;
	while( li!=NULL ) {
		v = li->v;
		add_edge(root, v, G, "red", DEFAULT_EDGE_STYLE);
		li = li->next;
	}

	root->isroot = true;
	return root;
}

/* ---------------------------------- MAIN ---------------------------------- */
void main(int argc, char **argv) {

  // graph *G = build_graph_from_stdin();
  // print_graph_in_stdout(G);

	// vlist *ftimevisit = DFS(G);
	// vlist_print(ftimevisit);

	// printf("\n");
	// transpose_graph(G);
	// print_transposed_graph_in_stdout(G);

	// printf("vertex dimension: %dbyte\n", (CHAR_BIT*sizeof(vertex))/8);
	// printf("edge dimension: %dbyte\n", (CHAR_BIT*sizeof(edge))/8);

  exit(1);
}
