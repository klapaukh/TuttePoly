#ifndef CONFIG_H
#define CONFIG_H

// the following is needed for Nauty and determines
// the maximum graph size
#define MAXN 0
#include "nauty.h"

#include "graph/adjacency_list.hpp"
#include "graph/spanning_graph.hpp"
#include "poly/simple_poly.hpp"
#include "poly/factor_poly.hpp"
#include <ext/hash_map>

// The vertex type dicates the maximum number of
// vertices allowed in a graph.
typedef unsigned short vertex_t;

#endif
