/* Gabriele Venturato (125512)
 * Progetto del laboratorio di ASD anno 2015/2016
 * Header file: definitions
 */
#ifndef MY_PROJECT_LIB_H_
#define MY_PROJECT_LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>      // to parsing input functions (isalnum)
#include <limits.h>     // to have the CHAR_BIT const, not essential for project

#define MAX_LABEL_LENGTH 128
#define DEFAULT_EDGE_COLOR ""
#define DEFAULT_EDGE_STYLE ""

typedef enum { false, true } bool;

/* ----------------------------- DATA STRUCTURES ---------------------------- */
// vertex and edge are used to implement Adjacency List
typedef struct vertex_T {
  struct vertex_T *next;  // next vertex in the list
  struct edge_T *edges;   // pointer to the edges list
  struct edge_T *tedges;  // pointer to the transposed edges list

  char label[MAX_LABEL_LENGTH];
  int id;                 // integer identificator for internal purpose
  int depth;              // depth of the node (in the tree created later)
  struct scc_T *sccref;   // reference to the scc which the vertex belongs
} vertex;

typedef struct edge_T {
  struct vertex_T *connectsTo;
  struct edge_T *next;
  char color[32];         // to manage the color in output .dot file
  char style[32];         // to manage the style in output .dot file
} edge;

typedef struct graph_T {
  struct vertex_T *vertices;
} graph;

// SCC management
typedef struct scc_T {
  struct vertex_T *root;
  int id;                  // integer identificator for internal purpose
  bool isreached;          // if SCC is reached
  struct scc_T *next;
} scc;

typedef struct sccset_T {
  struct scc_T *sccomponents;
} sccset;

// list of vertices
typedef struct vlist_T {
  struct vertex_T *v;      // pointer to a graph vertex
  struct vlist_T *next;
} vlist;

// vertices queue (used by BFS)
typedef struct vqueue_T {
  struct vertex_T *v;
  struct vqueue_T *next;
  struct vqueue_T *last;   // keep the pointer to the last, to do push in O(1)
} vqueue;


/* -------------------------- FUNCTIONS DECLARATION ------------------------- */
graph *build_graph_from_stdin();
graph *build_graph_from_file(FILE *fp);
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
vertex *add_missing_edges(graph *G, int *nedges, sccset *SCCset);
void BFS(graph *G, vertex *s);

// vertices list management
vlist *vlist_push(vlist *l, vertex *v);
void vlist_free(vlist *l);
void vlist_print(vlist *l); // used in debugging

// vertices queue management
void vqueue_push(vqueue **q, vertex *v);
vertex *vqueue_pop(vqueue **q);

// miscellaneous (for internal or debug use)
void print_transposed_graph_in_stdout(graph *G);
void print_scc(sccset *SCCset);
void print_misc_info();

#endif // MY_PROJECT_LIB_H_
