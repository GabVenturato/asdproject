/* Gabriele Venturato (125512)
 * Progetto del laboratorio di ASD anno 2015/2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>		// to parsing input functions (isalnum)
#include <limits.h>		// to have the CHAR_BIT const, not essential for project

#define MAX_LABEL_LENGTH 1024
#define DEFAULT_EDGE_COLOR ""
#define DEFAULT_EDGE_STYLE ""

typedef enum { false, true } bool;

/* ----------------------------- DATA STRUCTURES ---------------------------- */
// global variables
int totvertices = 0;			// stores the number of total vertices in the graph
int totscc = 0;						// stores the number of total sccomponents

// vertex and edge are used to implement Adjacency List
typedef struct vertex_T {
  struct vertex_T *next;	// next vertex in the list
  struct edge_T *edges;		// pointer to the edges list
	struct edge_T *tedges;	// pointer to the transposed edges list

  char label[MAX_LABEL_LENGTH];
  int id;									// integer identificator for internal purpose
	int depth;							// depth of the node (in the tree created later)
	struct scc_T *sccref;		// reference to the scc which the vertex belongs
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
	int id;									// integer identificator for internal purpose
	bool isreached;					// if SCC is reached
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
void print_graph_in_stdout(graph *G, vertex *root, int nedges);
void transpose_graph(graph *G);
vlist *DFS(graph *G);
vlist *DFS_visit(graph *G, vertex *u, bool *visited, vlist *ftimevertices);
sccset *DFS_SCC(graph *Gt, vlist *ftimevertices);
void DFS_SCC_visit(graph *Gt, vertex *u, bool *visited, scc *sccref);
sccset *SCC_finder(graph *G);
void scc_reachability(graph *G);
vertex *add_missing_edges(graph *G, int *nedges);

// vertices list management
vlist *vlist_push(vlist *l, vertex *v);
void vlist_free(vlist *l);
void vlist_print(vlist *l); // used in debugging

// miscellaneous (for internal or debug use)
void print_transposed_graph_in_stdout(graph *G);
void print_scc(sccset *SCCset);
void print_misc_info();

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

  G->vertices = NULL;						// set empty vertices list

  getline(&line, &len, stdin);	// skip first line
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
 * @root is a pointer to a vertex (the root of the solution)
 * @nedges is an integer representing the number of edges added for the solution
 * PRINT in stdout the graph in dot format. respecting the project requests.
 */
void print_graph_in_stdout(graph *G, vertex *root, int nedges) {
	char color[64], style[64], edge_decoration[256];

	printf("digraph out {\n");
  for( vertex *v = G->vertices; v!=NULL; v=v->next ) {
		// first of all print the node with its label
		if( v==root ) {
			printf("%s [label=\"root=%s; |E'|-|E|=%d\"];\n", v->label, root->label, nedges);
		} else {
			printf("%s [label=\"d(%s,%s)=%d\"];\n", v->label, root->label, v->label, 0);
		}

		// print the node edges, one per line, with its own decorations
		edge *e = v->edges;
		while( e!=NULL ) {
			// clean strings used
			color[0] = '\0';
			style[0] = '\0';
			edge_decoration[0] = '\0';
			// build decorations based on which attributes are defined in the vertex
			if( strcmp(e->style, DEFAULT_EDGE_STYLE) ) sprintf(style, "style=%s", e->style);
			if( strcmp(e->color, DEFAULT_EDGE_COLOR) ) sprintf(color, "color=%s", e->color);
			if( strlen(color) && strlen(style) ) sprintf(edge_decoration, " [%s, %s]", style, color);
			else if( strlen(color) || strlen(style) ) sprintf(edge_decoration, " [%s%s]", style, color);
			// print the edge with decoration built above
      printf("%s -> %s%s;\n", v->label, e->connectsTo->label, edge_decoration);
			e=e->next;
		}
  }
	printf("}\n");
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
 * RETURN the list of vertices ordered by finish visit time
 * this function perform a DFS visit to order graph vertices by finish visit
 * time. more details in DFS_visit() description.
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

/* @Gt is a transposed graph (ie: tedges list correctly filled)
 * @ftimevertices is a list of vertices ordered by finish time of a DFS visit
 * RETURN a pointer to a set that contains all scc of the graph G
 * this function perform a DFS visit in order to find all the SCComponents of
 * the graph G. every scc has an unique id (uniqueness guaranteed by global
 * scc counter).
 * DFS_SCC_visit() is used only here to perform DFS_SCC visit
 */
sccset *DFS_SCC(graph *Gt, vlist *ftimevertices) {
	bool visited[totvertices];	// keep track of visited vertices (here use id)
	sccset *SCCset = (sccset *) malloc(sizeof(sccset));
	SCCset->sccomponents = NULL;
	vertex *u;

	// set all vertices as "not visited"
	for(int i=0; i<totvertices; i++) {
		visited[i] = false;
	}

	vlist *li = ftimevertices;	// use vertices list ordered by finish visit time
	while( li!=NULL ) {
		u = li->v;
		if( !visited[u->id] ) {
			// build the scc with its root vertex
			scc *s = (scc *) malloc(sizeof(scc));
			s->root = u;
			s->id = totscc;
			u->sccref = s;					// store in vertex in which scc it is
			s->isreached = false;		// set all scc as "not reached" (field used later)
			// save the scc just created into the scc set
			s->next = SCCset->sccomponents;
			SCCset->sccomponents = s;
			totscc++;
			DFS_SCC_visit(Gt, u, visited, s);
		}
		li = li->next;
	}

	return SCCset;
}

/* @Gt is a transposed graph (ie: tedges list correctly filled)
 * @u is the vertex analyzed by this DFS_SCC_visit call
 * @visited is the array that keep track of visited vertices
 * @sccref is the scc in which all vertices will be put (note that during
 *         DFS_SCC_visit() execution can't be created new SCComponents)
 * this function perform a DFS_visit that puts all vertices in the appropriate
 * scc that is passed from DFS_SCC main function.
 */
void DFS_SCC_visit(graph *Gt, vertex *u, bool *visited, scc *sccref) {
	visited[u->id] = true;

	for(edge *e=u->tedges; e!=NULL; e=e->next) {
		vertex *v = e->connectsTo;
		if( !visited[v->id] ) {
			v->sccref = sccref;	// add vertex to its own scc
			DFS_SCC_visit(Gt, v, visited, sccref);
		}
	}
}

/* @G is a graph
 * RETURN a set containing all SCComponents of the graph G
 * this function simply put together other functions. see their description for
 * more details.
 */
sccset *SCC_finder(graph *G) {
	vlist *ftimevertices = DFS(G);
	transpose_graph(G);
	return DFS_SCC(G, ftimevertices);
}

/* @G is a graph in which scc are already discovered and saved in the specific
 *    data structure
 * this function check which scc are not reached from any vertex in other scc.
 * this info will be used to connect scc not reached from anyone (if exists),
 * and so to solve the problem.
 */
void scc_reachability(graph *G) {
	scc *i, *j;

	for(vertex *v=G->vertices; v!=NULL; v=v->next) {
		i = v->sccref;						// retreive the scc in which v is
		for(edge *e=v->edges; e!=NULL; e=e->next) {
			vertex *u = e->connectsTo;
			j = u->sccref; 					// retreive the scc in which u is

			// note that (v,u) is the edge analyzed
			if( i!=j ) {						// if they aren't in the same scc
				j->isreached = true;	// scc of u is reached from another one
			}
		}
	}
}

/* @G is a graph
 * @nedges is a pointer to an integer in which will be saved the number of
 *         vertices added
 * RETURN the root vertex
 * this function add missing edges to the graph in order to eventually have a
 * graph G which admits root (see definition in project pdf).
 */
vertex *add_missing_edges(graph *G, int *nedges) {
	vertex *root, *v;
	vlist *scc_not_reached=NULL, *li;
	scc *s;
	bool already_in[totscc];	// to check if a scc is already in scc_not_reached

	scc_reachability(G);			// set reachability flags in graph

	// set that any vertex is already in the list
	for(int i=0; i<totscc; i++) already_in[i] = false;

	// put the scc root vertex in the list if it is not reached and not already in
	for(vertex *v=G->vertices; v!=NULL; v=v->next) {
		s = v->sccref;
		if( !s->isreached && !already_in[s->id] ) {
			scc_not_reached = vlist_push(scc_not_reached, s->root);
			already_in[s->id] = true;
		}
	}

	li = scc_not_reached;
	if( li!=NULL ) {
		// choose as graph root the first not reached vertex
		root = li->v;
		li = li->next;

		// add edges (root,v) for each v not reached (if exists)
		*nedges = 0;
		while( li!=NULL ) {
			v = li->v;
			if( v!=root ) {
				add_edge(root, v, G, "red", DEFAULT_EDGE_STYLE);
				(*nedges)++;
			}
			li = li->next;
		}
	}

	vlist_free(scc_not_reached);
	return root;
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
 * this (ecological) function free the memory occupied by a list
 */
void vlist_free(vlist *l) {
	vlist* tmp;
	while( l!=NULL ) {
		tmp = l;
		l = l->next;
		free(tmp);
	}
}

/* @l is a pointer to the head of the list
 * not used to solve the problem. debugging use
 */
void vlist_print(vlist *l) {
	for(vlist *p=l; p!=NULL; p=p->next) {
		printf("%s(%d) ", (p->v)->label, (p->v)->id);
	}
	printf("\n");
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

/* @SCCset is a set of SCComponents
 * function used only for debugging
 */
void print_scc(sccset *SCCset) {
	printf("SCC founded:\n");
	for(scc *s=SCCset->sccomponents; s!=NULL; s=s->next) {
		printf("id: %4d; root: %s\n", s->id, s->root->label);
	}
	printf("\n");
}

/* this function print in stdout the approximated sizes (in byte) of one vertex
 * and one edge.
 * calculus details:
 * sizeof() returns the dimension in CHAR_BIT units (number of char occupied)
 * CHAR_BIT is the dimension of a char in bit
 * [(char dimension in bit) * (number of char occupied)]/8
 * because of byte=8bit the result is in byte
 */
void print_misc_info() {
	printf("vertex dimension: %dbyte\n", (CHAR_BIT*sizeof(vertex))/8);
	printf("edge dimension: %dbyte\n", (CHAR_BIT*sizeof(edge))/8);
}

/* ---------------------------------- MAIN ---------------------------------- */
void main(int argc, char **argv) {
	int nedges;

  graph *G = build_graph_from_stdin();
  sccset *SCCset = SCC_finder(G);
	vertex *root = add_missing_edges(G, &nedges);
	print_graph_in_stdout(G, root, nedges);

  exit(1);
}
