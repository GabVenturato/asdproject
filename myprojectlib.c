/* Gabriele Venturato (125512)
 * Progetto del laboratorio di ASD anno 2015/2016
 * Header file: implementation
 */

#include "myprojectlib.h"

// global variables
int totvertices = 0;      // stores the number of total vertices in the graph
int totscc = 0;           // stores the number of total sccomponents

/* --------------------------- FUNCTIONS DEFINITION ------------------------- */
/* STDIN description in dot file from stdin
 * RETURN graph filled with dot file information
 * The dot file is needed to be in a specific format. (see project assignment
 * description).
 */
graph *build_graph_from_stdin() {
  graph *G = (graph *) malloc(sizeof(graph));
  char *line = NULL;
  size_t len = 0;

  G->vertices = NULL;            // set empty vertices list

  getline(&line, &len, stdin);   // skip first line
  while(getline(&line, &len, stdin) != -1 ) {
    add_element_from_dot_line(line, G);
  }

  free(line);
  return G;
}

/* @filename filename of a dot file where there is the input
 * RETURN graph filled with dot file information
 * The dot file is needed to be in a specific format. (see project assignment
 * description).
 */
graph *build_graph_from_file(FILE *fp) {
  graph *G = (graph *) malloc(sizeof(graph));
  char *line = NULL;
  size_t len = 0;

  G->vertices = NULL;            // set empty vertices list
  totvertices = 0;               // clean total number of vertices
  totscc = 0;                    // clean total number of scc
  getline(&line, &len, fp);      // skip first line
  while(getline(&line, &len, fp) != -1 ) {
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
  } while( current!='\0' && current!='\n' && current!=';' && !v2_found );

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
  v->id = totvertices;  // use the current number of total edges as identifier
  v->depth = -1;        // default value for depth is -1
  // attach it to G, at the top of vertices list
  v->next = G->vertices;
  v->edges = NULL;
  v->tedges = NULL;
  v->sccref = NULL;
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
 * NOTE: the output could be in a better format using a BFS visit, starting from
 * root vertex, to print out the graph.
 */
void print_graph_in_stdout(graph *G, vertex *root, int nedges) {
  char color[64], style[64], edge_decoration[256];

  printf("digraph out {\n");
  for( vertex *v = G->vertices; v!=NULL; v=v->next ) {
    // first of all print the node with its label
    if( v==root ) {
      printf("%s [label=\"root=%s; |E'|-|E|=%d\"];\n", v->label, root->label, nedges);
    } else {
      printf("%s [label=\"d(%s,%s)=%d\"];\n", v->label, root->label, v->label, v->depth);
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
  bool visited[totvertices];    // keep track of visited vertices (here use id)
  vlist *ftimevertices = NULL;  // list to fill with vertices
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
  bool visited[totvertices];  // keep track of visited vertices (here use id)
  sccset *SCCset = (sccset *) malloc(sizeof(sccset));
  SCCset->sccomponents = NULL;
  vertex *u;

  // set all vertices as "not visited"
  for(int i=0; i<totvertices; i++) {
    visited[i] = false;
  }

  vlist *li = ftimevertices;  // use vertices list ordered by finish visit time
  while( li!=NULL ) {
    u = li->v;
    if( !visited[u->id] ) {
      // build the scc with its root vertex
      scc *s = (scc *) malloc(sizeof(scc));
      s->root = u;
      s->id = totscc;
      u->sccref = s;          // store in vertex in which scc it is
      s->isreached = false;   // set all scc as "not reached" (field used later)
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
      v->sccref = sccref;  // add vertex to its own scc
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
  sccset *s = DFS_SCC(G, ftimevertices);
  vlist_free(ftimevertices);  // clean memory used by ftimevertices list
  return s;
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
    i = v->sccref;            // retreive the scc in which v is
    for(edge *e=v->edges; e!=NULL; e=e->next) {
      vertex *u = e->connectsTo;
      j = u->sccref;          // retreive the scc in which u is

      // note that (v,u) is the edge analyzed
      if( i!=j ) {            // if they aren't in the same scc
        j->isreached = true;  // scc of u is reached from another one
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
 * NOTE: it could be improved removing the auxiliary list "scc_not_reached"
 */
vertex *add_missing_edges(graph *G, int *nedges, sccset *SCCset) {
  vertex *root=NULL, *v;
  vlist *scc_not_reached=NULL, *li;
  scc *temp;

  scc_reachability(G);      // set reachability flags in graph

  // put the scc root vertex in the list if it is not reached
  scc *s=SCCset->sccomponents;
  while(s!=NULL) {
    if( !s->isreached ) scc_not_reached = vlist_push(scc_not_reached, s->root);
    temp = s;
    s = s->next;
    free(temp);              // free memory used by scc set
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

/* @G is a graph
 * @s is the source vertex for BFS visit
 * this function perform a BFS visit in order to find the shortest-paths tree
 * rooted in s. it also calculate de depth for each node in the shortest-paths
 * tree.
 * it uses the queue data structure, see vqueue functions for more details.
 */
void BFS(graph *G, vertex *s) {
  bool visited[totvertices];  // keep track of visited vertices (here use id)
  vqueue *q=NULL;

  if( s!=NULL ) {             // if the input graph is empty
    // set all vertices as "not visited"
    for(int i=0; i<totvertices; i++) {
      visited[i] = false;
    }

    visited[s->id] = true;
    s->depth = 0;
    vqueue_push(&q,s);
    while( q!=NULL ) {
      vertex *u = vqueue_pop(&q);
      for(edge *e=u->edges; e!=NULL; e=e->next) {
        vertex *v = e->connectsTo;
        if( !visited[v->id] ) {
          visited[v->id] = true;
          v->depth = u->depth +1;
          strcpy(e->style, "dashed");
          vqueue_push(&q,v);
        }
      }
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

/* @q is a pointer to the head of the queue
 * @v is the vertex to push in
 * this function push the new element ad the end (tail) of the queue. it updates
 * the "last" parameter only in the head of the queue.
 */
void vqueue_push(vqueue **q, vertex *v) {
  vqueue *el = (vqueue *) malloc(sizeof(vqueue));
  el->v = v;
  el->next = NULL;
  el->last = el;
  if( *q==NULL ) {  // if the queue is empty
    *q = el;
  } else {          // else update the two pointers to store the new element
    (*q)->last->next = el;
    (*q)->last = el;
  }
}

/* @q is a pointer to the head of the queue
 * RETURN the vertex removed from the queue
 * this function remove the first element of the queue (from the head). it keeps
 * updated the pointer to the last element only in the head of the queue.
 * it also takes care to free the memory of the removed element.
 */
vertex *vqueue_pop(vqueue **q) {
  vertex *v = NULL;
  vqueue *f = NULL;
  if( (*q)!=NULL ) {
    v = (*q)->v;
    if( (*q)->next!=NULL )
      (*q)->next->last = (*q)->last;  // update the pointer to the last queue el
    f = *q;                           // save the head pointer to free memory
    (*q) = (*q)->next;
    free(f);
  }
  return v;
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
  printf("vertex dimension: %lubyte\n", (CHAR_BIT*sizeof(vertex))/8);
  printf("edge dimension: %lubyte\n", (CHAR_BIT*sizeof(edge))/8);
}
