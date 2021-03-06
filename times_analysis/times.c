/* Gabriele Venturato (125512)
 * Progetto del laboratorio di ASD anno 2015/2016
 * Programma per lo studio dei tempi.
 * NB: alcuni algoritmi sono stati presi dal pdf degli appunti AA 2015/2016
 * per avere una corrispondenza diretta con tali appunti sono stati mantenuti i
 * nomi italiani degli algoritmi.
 */

#include <time.h>
#include <math.h>
#include "../myprojectlib.h"

#define K 0.05      // error tolerance
#define CN 20       // sample set dimension
#define FROM_N 0    // start test from this input dimension
#define TO_N 150    // test until this input dimension
#define TEST_STEP 1 // step for input dimension cycle
#define INPUT_FILENAME "input.dot"

typedef enum { worst, average, best, wrongcase } graphcase;

/* ----------------------------- DATA STRUCTURES ---------------------------- */
// gloabl variables
double seed = 0; // seed to generate pseudo-random numbers

// to manage return value of function "misurazione"
typedef struct resrow_T {
  double e;
  double delta;
} resrow;

/* -------------------------- FUNCTIONS DECLARATION ------------------------- */
void myrandom_init(double s);
double myrandom_next();
double granularita();
double tempoMedioNetto(void (*prepara)(int N), int d, double tMin);
int calcolaRipTara(void (*prepara)(int N), int d, double tMin);
int calcolaRipLordo(void (*prepara)(int N), int d, double tMin);
resrow *misurazione(void (*prepara)(int N), int d, int n, double za, double tMin, double D);
void projectsolver();
void create_graph_worst(int n);
void create_graph_average(int n);
void create_graph_best(int n);
void times_test(void (*prepara)(int N), char *filename);
graphcase get_graphcase(char *gc);
void free_graph_memory(graph *G);

/* -------------------------- FUNCTIONS DEFINITION -------------------------- */
/* @s is a double seed to inizialize the pseudo-random generator
 */
void myrandom_init(double s) {
  seed = abs((int)s % 2147483647);
}

/* RETURN the next pseudo-random value in interval [0,1]
 * this function NEEDS to be called after myrandom_init() to work properly.
 * Algorithm 8
 */
double myrandom_next() {
  double hi,lo,test;
  int a = 16087,
      m = 2147483647,
      q = 127773,
      r = 2836;
  hi = ceil(seed/q);
  lo = seed-q*hi;
  test = a*lo-r*hi;
  if( test<0.0 )
    seed = test+m;
  else
    seed = test;
  return (double) seed/m;
}

/* RETURN the system granularity is seconds
 * Algorithm 4
 */
double granularita() {
  double  t0 = clock(),
          t1 = clock();
  printf("\nt0 %f",t0);
  while( t0==t1 ) {
    t1=clock();
    printf("\nt1 %f\n",t1);
  }
  return (t1-t0)/CLOCKS_PER_SEC;
}

/* @prepara pointer to the function "prepara"
 * @d input dimension
 * @tMin
 * Algorithm 7
 */
double tempoMedioNetto(void (*prepara)(int N), int d, double tMin) {
  int ripTara=0, ripLordo=0;
  double t0, t1, tara, lordo;
  ripTara = calcolaRipTara(prepara, d, tMin);
  ripLordo = calcolaRipLordo(prepara, d, tMin);

  // printf("\nripTara: %d\tripLordo: %d", ripTara, ripLordo);

  t0 = clock();
  for(int i=0; i<ripTara; i++) (*prepara)(d);
  t1 = clock();
  tara = (t1-t0)/CLOCKS_PER_SEC;

  t0 = clock();
  for(int i=0; i<ripLordo; i++) {
    (*prepara)(d);
    projectsolver();
  }
  t1 = clock();
  lordo = (t1-t0)/CLOCKS_PER_SEC;

  return (double) ((lordo/ripLordo)-(tara/ripTara));
}

/* @prepara pointer to the function "prepara"
 * @d input dimension
 * @tMin
 * Algorithm 5: Tara version
 */
int calcolaRipTara(void (*prepara)(int N), int d, double tMin) {
  double t0=0, t1=0;
  int rip=1, max=0, min=0, cicliErrati=5;

  while( ((t1-t0)/CLOCKS_PER_SEC) <= tMin ) {
    rip = rip*2;
    t0 = clock();
    for(int i=0; i<rip; i++) (*prepara)(d);
    t1 = clock();
  }

  max = rip;
  min = rip/2;
  while( (max-min) >= cicliErrati ) {
    rip = (max+min)/2;
    t0 = clock();
    for(int i=0; i<rip; i++) (*prepara)(d);
    t1 = clock();
    if( ((t1-t0)/CLOCKS_PER_SEC) <= tMin ) min = rip;
    else max = rip;
  }

  return max;
}

/* @prepara pointer to the function "prepara"
 * @d input dimension
 * @tMin
 * Algorithm 5: Lordo version
 */
int calcolaRipLordo(void (*prepara)(int N), int d, double tMin) {
  double t0=0, t1=0;
  int rip=1, max=0, min=0, cicliErrati=0;

  while( ((t1-t0)/CLOCKS_PER_SEC) <= tMin ) {
    rip = rip*2;
    t0 = clock();
    for(int i=0; i<rip; i++) {
      (*prepara)(d);
      projectsolver();
    }
    t1 = clock();
  }

  max = rip;
  min = rip/2;
  cicliErrati = 5;
  while( (max-min) >= cicliErrati ) {
    rip = (max+min)/2;
    t0 = clock();
    for(int i=0; i<rip; i++) {
      (*prepara)(d);
      projectsolver();
    }
    t1 = clock();
    if( ((t1-t0)/CLOCKS_PER_SEC) <= tMin ) min = rip;
    else max = rip;
  }

  return max;
}

/* @prepara pointer to a function which prepare the input of dimension N
 * @d is the input dimension
 * @n is the set dimension
 * @za its value comes from normal distribution (more details in the pdf)
 * @tMin minimum execution time needed to maintain the error<=K
 * @D is the delta upper bound
 * Algorithm 9
 */
resrow *misurazione(void (*prepara)(int N), int d, int n, double za, double tMin, double D) {
  double m=0, t=0, sum2=0, delta=0, e=0, s=0;
  int cn=0;
  resrow *res;

  do
  {
    for(int i=0; i<n; i++) {
      m = tempoMedioNetto(prepara, d, tMin);
      t += m;
      sum2 = sum2 + (m*m);
    }

    cn += n;
    e = t/cn;
    s = sqrt(sum2/cn-(e*e));
    delta = (1/(sqrt(cn))) * za * s;
  } while(delta > D);

  res = (resrow *) malloc(sizeof(resrow));
  res->e = e;
  res->delta = delta;
  return res;
}

/* This is the algorithm that solve the problem in the project file.
 * Only two differences:
 * - it takes the input from a file (filname defined as costant) and not stdin
 * - it doesn't print out the output graph
 */
void projectsolver() {
  int nedges;
  FILE *fp = fopen(INPUT_FILENAME, "r");

  graph *G = build_graph_from_file(fp);
  sccset *SCCset = SCC_finder(G);
  vertex *root = add_missing_edges(G, &nedges, SCCset);
  BFS(G, root);
  free_graph_memory(G);

  fclose(fp);
}

/* @n graph dimension (number of vertices)
 * this function generates the worst case graph: complete graph
 */
void create_graph_worst(int n) {
  FILE *fp = fopen(INPUT_FILENAME,"w");

  fprintf(fp, "digraph G {\n");
  for(int i=0; i<n; i++) {
    for(int j=0; j<n; j++) {
      fprintf(fp, "%d->%d;\n", i, j);
    }
  }
  fprintf(fp,"}");

  fclose(fp);
}

/* @n graph dimension (number of vertices)
 * this function generates the average case graph: randomly built graph
 */
void create_graph_average(int n) {
  FILE *fp = fopen(INPUT_FILENAME,"w");

  fprintf(fp, "digraph G {\n");
  for(int i=0; i<n; i++) {
    fprintf(fp, "%d;\n", i);
    for(int j=0; j<n; j++) {
      double r = myrandom_next();
      if( r < 0.5 ) {
        fprintf(fp, "%d->%d;\n", i, j);
      }
    }
  }
  fprintf(fp,"}");

  fclose(fp);
}

/* @n graph dimension (number of vertices)
 * this function generates the best case graph: a graph without edges
 */
void create_graph_best(int n) {
  FILE *fp = fopen(INPUT_FILENAME,"w");

  fprintf(fp, "digraph G {\n");
  for(int i=0; i<n; i++) {
    fprintf(fp, "%d;\n", i);
  }
  fprintf(fp,"}");

  fclose(fp);
}

/* @prepara pointer to a function which prepare the input of dimension N
 * @tMin minimum execution time needed to maintain the error<=K
 * @filename for the output data
 * this function execute misurazione() from FROM_N to TO_N vertices as input
 *  dimension passed to prepara() function.
 */
void times_test(void (*prepara)(int N), char *filename) {
  resrow *r;
  double g,tMin;

  myrandom_init(123456789);

  g = granularita();
  tMin = g/K;
  printf("\nGranularity: %f\ntMin calculated: %f\n", g, tMin);
  printf("\nTest in input which dimension from 0 to %d\n", TO_N);
  printf("For each step of dimension, perform the test in a set of %d elements.\n", CN);

  FILE *fp = fopen(filename, "w");

  for(int i=FROM_N; i<=TO_N; i=i+TEST_STEP) {
    r = misurazione(prepara, i, CN, 1.96, tMin, 0.02);
    fprintf(fp, "%d %f %f\n", i, r->e, r->delta);

    printf("\nN %d/%d\n\tE: %f\tdelta: %f\n", i, TO_N, r->e, r->delta);
  }

  printf("\n\nTest completed! Results saved in \"%s\"\n\n", filename);

  fclose(fp);
}

/* @gc is a string
 * this function aim is to take the string passed as parameter to the main() and
 * convert it into the equivalent graphcase type
 */
graphcase get_graphcase(char *gc) {
  if( strcmp(gc,"worst")==0 ) return worst;
  else if( strcmp(gc,"average")==0 ) return average;
  else if( strcmp(gc,"best")==0 ) return best;
  else return wrongcase;
}

/* @G is a graph
 * this function aim is to clean the memory occupied by a graph in order to not
 * full the RAM while doing heavy tests
 */
void free_graph_memory(graph *G) {
  edge *e, *etemp;
  vertex *v, *vtemp;

  v = G->vertices;
  while(v!=NULL) {
    e = v->edges;
    while(e!=NULL) {
      etemp = e;
      e = e->next;
      free(etemp);
    }
    vtemp = v;
    v = v->next;
    free(vtemp);
  }
}

/* ---------------------------------- MAIN ---------------------------------- */
int main(int argc, char **argv) {

  if( argc != 2 ) {
    printf("Error! Program usage: ./times <graphcase>\nwhere graphcase is one of: worst, average, best\n\n");
    return 1;
  } else {
    switch ( get_graphcase(argv[1]) ) {
      case worst:
        times_test(create_graph_worst, "worst.txt");
        break;
      case average:
        times_test(create_graph_average, "average.txt");
        break;
      case best:
        times_test(create_graph_best, "best.txt");
        break;
      default:
        printf("\n\nError! Program usage: ./times <graphcase>\nwhere graphcase is one of: worst, average, best\n\n");
        return 1;
    }
  }

  return 0;
}
