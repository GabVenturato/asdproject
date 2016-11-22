/* Gabriele Venturato (125512)
 * Progetto del laboratorio di ASD anno 2015/2016
 * Soluzione del problema.
 */

#include "myprojectlib.h"

/* ---------------------------------- MAIN ---------------------------------- */
void main(int argc, char **argv) {
	int nedges;

  graph *G = build_graph_from_stdin();
  sccset *SCCset = SCC_finder(G);
	vertex *root = add_missing_edges(G, &nedges);
	BFS(G, root);
	print_graph_in_stdout(G, root, nedges);

  exit(1);
}
