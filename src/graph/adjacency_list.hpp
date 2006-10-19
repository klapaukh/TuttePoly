#ifndef ADJACENCY_LIST_HPP
#define ADJACENCY_LIST_HPP

#include <vector>
#include <list>

using namespace std;

// This graph type is simply the most basic implementation
// you could think of.

class adjacency_list {
public:
  typedef list<int>::const_iterator vertex_iterator;
  typedef vector<int>::const_iterator edge_iterator;
private:
  list<int> vertices;
  vector<vector<int> > edges;  
public:
  adjacency_list(int n) : edges(n)  { 
    for(int i=0;i!=n;++i) {
      vertices.push_back(i);
    }
  }

  // there is no add vertex!
  bool remove(int v) { vertices.remove(v); }

  void add_edge(int from, int to) {
    edges[from].push_back(to);
    edges[to].push_back(from);
  }

  void remove_edge(int from, int to) {
    // this is horribly inefficient
    for(vector<int>::iterator i(edges[from].begin());i!=edges[from].end();++i) {
      if(*i == to) {
	// first match!
	edges[from].erase(i);
	break;
      }
    }

    for(vector<int>::iterator i(edges[to].begin());i!=edges[to].end();++i) {
      if(*i == from) {
	// first match!
	edges[to].erase(i);
	break;
      }
    }
  }

  // These do nothing yet!!!
  int remove_loops() { return 0; }
  void constract_edge(int from, int to) { }

  vertex_iterator vert_begin() const { return vertices.begin(); }
  vertex_iterator vert_end() const { return vertices.end(); }
  
  edge_iterator edges_begin(int f) const { return edges[f].begin(); }
  edge_iterator edges_end(int f) const { return edges[f].end(); }
};

#endif
