// (C) Copyright David James Pearce and Gary Haggard, 2007. 
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied 
// warranty, and with no claim as to its suitability for any purpose.
//
// Email: david.pearce@mcs.vuw.ac.nz

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <set>
#include <vector>
#include <getopt.h>
#include <cmath>
#include "misc/triple.hpp"

using namespace std;

typedef enum { UNUSED, TREE, MATCH, NONTREE } node_t;
typedef vector<triple<unsigned int, unsigned int, unsigned int> > graph_t;

class node {
public:  
  node_t type;
  unsigned int match_id;  
  unsigned int con_id;
  unsigned int del_id;
  graph_t graph;
};

// ---------------------------------------------------------------
// Simple Input File Parser
// ---------------------------------------------------------------

void match(char c, unsigned int &pos, string const &str) {
  if(pos >= str.length() || str[pos] != c) { throw runtime_error(string("syntax error -- expected '") + c + "', got '" + str[pos] + "'"); }
  ++pos;
}

int parse_number(unsigned int &pos, string const &str) {
  int s = pos;
  while(pos < str.length() && isdigit(str[pos])) {
    pos = pos + 1;
  }
  stringstream ss(str.substr(s,pos));
  int r;
  ss >> r;
  return r;
}

graph_t parse_graph(unsigned int &pos, string const &str) {
  match('{',pos,str);
  bool first=true;
  graph_t graph;

  while(pos < str.length()) {
    if(str[pos] == '}') { return graph; }
    if(!first) { match(',',pos,str); }
    first=false;

    unsigned int h,t,c=1;
    h = parse_number(pos,str);
    match('-',pos,str);
    t = parse_number(pos,str);
    if(str[pos] == '(') {
      match('(',pos,str);
      c = parse_number(pos,str);
      match(')',pos,str);
    }
    
    graph.push_back(make_triple(h,t,c));
  }
  
  throw runtime_error(string("syntax error"));
}

vector<node> read_input(istream &input) {
  vector<node> data;
  node empty;
  empty.type = UNUSED;

  while(!input.eof()) {
    string line;
    input >> line;
    unsigned int pos(0);
    if(isdigit(line[0])) {
      unsigned int id = parse_number(pos,line);
      if(data.size() <= id) { data.resize(id+1,empty); }
      if(line[pos] == '=') {
	match('=',pos,line);
	if(isdigit(line[pos])) {
	  // this is a match node
	  unsigned int match_id = parse_number(pos,line);
	  data[id].type = MATCH;
	  data[id].match_id = match_id;
	} else {
	  if(data[id].type == UNUSED) {
	    data[id].type = TREE;
	  }
	  data[id].graph = parse_graph(pos,line);	  
	}
      } else if(line[pos] == '-') {
	match('-',pos,line);
	match('e',pos,line);
	match('=',pos,line);
	unsigned int del_id = parse_number(pos,line);
	data[id].type = NONTREE;
	data[id].del_id = del_id;      
      } else if(line[pos] == '/') {
	match('/',pos,line);
	match('e',pos,line);
	match('=',pos,line);
	unsigned int con_id = parse_number(pos,line);
	data[id].type = NONTREE;
	data[id].con_id = con_id;
      } else {
	throw runtime_error(string("syntax error on '") + line[pos] + "'");
      }
    } else {
      return data;
    }
  }

  return data;
}

// ---------------------------------------------------------------
// Misc helper Methods
// ---------------------------------------------------------------

unsigned int num_vertices(graph_t const &g) {
  std::set<unsigned int> V;
  for(graph_t::const_iterator i(g.begin());i!=g.end();++i) {
    V.insert(i->first);
    V.insert(i->second);
  }
  return V.size();
}

unsigned int num_edges(graph_t const &g) {
  unsigned int e(0);
  for(graph_t::const_iterator i(g.begin());i!=g.end();++i) {
    e += i->third;
  }
  return e;
}

bool is_multigraph(graph_t const &g) {
  for(graph_t::const_iterator i(g.begin());i!=g.end();++i) {
    if(i->third > 1) { return true; }
  }
  return false;
}

double mean(vector<unsigned int> const &data) {
  unsigned long sum(0);
  for(vector<unsigned int>::const_iterator i(data.begin());
      i!=data.end();++i) {
    sum += *i;
  }
  return ((double)sum) / data.size();
}

double sdev(vector<unsigned int> const &data) {
  double x(mean(data));
  double S(0);

  for(vector<unsigned int>::const_iterator i(data.begin());
      i!=data.end();++i) {
    S += (*i - x) * (*i - x);
  }

  S = sqrt(S/data.size());
  return S;
}

// ---------------------------------------------------------------
// Analysis Methods
// ---------------------------------------------------------------

vector<unsigned int> match_v_sizes(vector<node> const &data) {
  vector<unsigned int> sizes;
  // now, perform some simple analysis
  for(unsigned int i=0;i!=data.size();++i) {
    if(data[i].type == MATCH) {
      graph_t const &g(data[i].graph);
      sizes.push_back(num_vertices(g));
    }
  }      
  return sizes;
}

vector<unsigned int> match_e_sizes(vector<node> const &data) {
  vector<unsigned int> sizes;
  // now, perform some simple analysis
  for(unsigned int i=0;i!=data.size();++i) {
    if(data[i].type == MATCH) {
      graph_t const &g(data[i].graph);
      sizes.push_back(num_edges(g));
    }
  }      
  return sizes;
}

unsigned int count(node_t type, vector<node> const &data) {
  unsigned int c(0);
  for(unsigned int i=0;i!=data.size();++i) {
    if(data[i].type == MATCH) { c++; }
  }
  return c;
}

unsigned int count_identical_matches(vector<node> const &data) {
  unsigned int c(0);
  for(unsigned int i=0;i!=data.size();++i) {
    if(data[i].type == MATCH && data[i].graph == data[data[i].match_id].graph) {
      c++; 
    }
  }
  return c;
}


// ---------------------------------------------------------------
// Format Conversion Methods
// ---------------------------------------------------------------

void write_dot(vector<node> const &data, ostream &out) {
  out << "digraph {" << endl;
  for(unsigned int i=1;i!=data.size();++i) {
    if(data[i].type != MATCH) {
      double size = 1 + num_vertices(data[i].graph);
      size /= 10;    
      out << "\t" << i << " [fontsize=9,width=" << size << ",height=" << size;
      if(is_multigraph(data[i].graph)) {
	out << ",style=filled";
      }
      out << "]" << endl;
    }
  }
  
  for(unsigned int i=0;i!=data.size();++i) {
    if(data[i].type == NONTREE) {
      unsigned int del = data[i].del_id;
      unsigned int con = data[i].con_id;
      unsigned int mid;
      string dstyle="";
      string cstyle="";
      // bypass match nodes
      if(data[del].type == MATCH) { 
	mid = data[del].match_id; 
	if(data[del].graph == data[mid].graph) {
	  dstyle = "[arrowhead=dot]";
	} else {
	  dstyle = "[arrowhead=odot]";
	}
	del = mid;
      }
      if(data[con].type == MATCH) { 
	mid = data[con].match_id; 
	if(data[con].graph == data[mid].graph) {
	  cstyle = ",arrowhead=dot";
	} else {
	  cstyle = ",arrowhead=odot";
	}
	con = mid;
      }
      // draw arcs
      out << "\t" << i << " -> " << del << dstyle << endl;
      out << "\t" << i << " -> " << con << " [style=dashed" << cstyle << "]" << endl;
    } 
  }
  out << "}" << endl;  
}

// This method attempts to draw *all* the little graphs
// as well.  It really will only work on small computation 
// trees.
void write_full_dot(vector<node> const &data, ostream &out) {
  out << "graph {" << endl;
  out << "\tcompound=true;" << endl;
  out << "\tnodesep=0.05;" << endl;
  out << "\tranksep=0.05;" << endl;

  // first, we create the subgraph nodes
  
  vector<unsigned int> first(data.size(),0);
  vector<unsigned int> last(data.size(),0);
  unsigned int vindex=0;
  for(unsigned int i=1;i<data.size();++i) {
    if(data[i].type != MATCH && data[i].type != UNUSED) {
      out << "\tsubgraph cluster" << i << " {" << endl;
      out << "\t\t" << "style=invis;" << endl;
      out << "\t\tnode [label=\"\",width=0.05,height=0.05]" << endl;
      graph_t const &graph(data[i].graph);
      unsigned int l = 0;
      unsigned int f = 100000;
      for(unsigned int j=0;j!=graph.size();++j) {	
	out << "\t\t" << (vindex+graph[j].first) << "--" << (vindex+graph[j].second);
	// make multi-edges appear in bold
	if(graph[j].third > 1) { out << " [style=bold]"; }
	out << ";" << endl;
	l = max(l,max(graph[j].first,graph[j].second));
	f = min(f,min(graph[j].first,graph[j].second));
      }
      last[i] = vindex+l;
      first[i] = vindex+f;
      vindex += l+1;
      out << "\t}" << endl;
    }
  }

  // second, we create the edges between subgraphs
  for(unsigned int i=1;i!=data.size();++i) {
    if(data[i].type == NONTREE) {
      unsigned int del = data[i].del_id;
      unsigned int con = data[i].con_id;
      if(data[del].type == MATCH) { del = data[del].match_id; }
      if(data[con].type == MATCH) { con = data[con].match_id; }
      out << "\t" << last[i] << " -- " << first[del] << " [minlen=5,ltail=cluster" << i << ",lhead=cluster" << del << ",arrowhead=normal]" << endl;
      out << "\t" << last[i] << " -- " << first[con] << " [minlen=5,ltail=cluster" << i << ",lhead=cluster" << con << ",arrowhead=normal,style=dashed]" << endl;
    }
  }

  out << "}" << endl;  
}


// ---------------------------------------------------------------
// Main Method
// ---------------------------------------------------------------

#define OPT_HELP 0
#define OPT_DOT 1
#define OPT_STATS 2
#define OPT_FULL 10

struct option long_options[]={
  {"help",no_argument,NULL,OPT_HELP},
  {"dot",no_argument,NULL,OPT_DOT},
  {"stats",no_argument,NULL,OPT_STATS},
  {"full",no_argument,NULL,OPT_FULL},
  NULL
};

int main(int argc, char* argv[]) {
  char *descriptions[]={
    "        --help                    display this information",
    " -d     --dot                     convert input tree to dot format",
    " -f     --full                    convert (full) input tree",
    " -s     --stats                   show various stats on tree",
    NULL
  };

  unsigned int v;

  unsigned int mode = OPT_HELP;
  bool full_mode=false;

  while((v=getopt_long(argc,argv,"sfd",long_options,NULL)) != -1) {
    switch(v) {      
    case OPT_HELP:
      mode = OPT_HELP;
      break;
    case 'd':
    case OPT_DOT:
      mode = OPT_DOT;
      break;
    case 's':
    case OPT_STATS:
      mode = OPT_STATS;
      break;
    case 'f':
    case OPT_FULL:
      full_mode=true;
      break;
    }
  }
  
  try {
    switch(mode) {
    case OPT_HELP:
      cout << "usage: " << argv[0] << " [options] <input tree file>" << endl;
      cout << "options:" << endl;
      for(char **ptr=descriptions;*ptr != NULL; ptr++) {
	cout << *ptr << endl;
      }    
      exit(1);    
    case OPT_DOT:
      {
	vector<node> data=read_input(cin);
	if(full_mode) {
	  write_full_dot(data,cout);
	} else {
	write_dot(data,cout);
	}
	exit(1);
      }
    case OPT_STATS:
      vector<node> data=read_input(cin);
      vector<unsigned int> match_vs(match_v_sizes(data));
      vector<unsigned int> match_es(match_e_sizes(data));
      cout << "Average match had " << setprecision(2) << mean(match_vs) << " vertices (+/-" << sdev(match_vs) << ")";
      cout << ", and " << mean(match_es) << " edges (+/-" << sdev(match_es) << ")." << endl;
      unsigned int nmatches(count(MATCH,data));
      unsigned int nisomatches(count_identical_matches(data));
      double isoratio = round((((double) nisomatches) / nmatches) * 100);
      cout << "There were " << nmatches << " matches, of which " << nisomatches << " (" << isoratio << "%) were identical." << endl;    
    }
  } catch(bad_alloc const &e) {
    cout << "error: insufficient memory!" << endl;
  } catch(exception const &e) {
    cout << "error: " << e.what() << endl;
  }      
}
