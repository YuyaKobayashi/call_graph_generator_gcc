#include <fstream>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <string.h>
#include <stack>
#include <iterator>

using namespace std;

class FuncNode
{
	public:
		string func;
		string path;
		FuncNode(const string& f = string(""), const string& p = string("")):
			func(f), path(p) { };
};

class CallEdge
{
	public:
		FuncNode src, dst;
		CallEdge(const FuncNode& s, const FuncNode& d):
			src(s), dst(d) { };

};

namespace std
{
	template<> struct hash<FuncNode>
	{
		size_t operator()(const FuncNode &n) const
		{
			return hash<string>()(n.func) * 2 + 
			 3 *hash<string>()(n.path);
		};
	};

	template<> struct hash<CallEdge>
	{
		size_t operator()(const CallEdge &e) const
		{
			return hash<FuncNode>()(e.src)*2 + 3*  hash<FuncNode>()(e.dst);
		};
	};

	template<> struct equal_to<FuncNode>
	{
		bool operator()( const FuncNode& n1, const FuncNode& n2) const
		{
			return n1.func == n2.func 
				&& n1.path == n2.path;
		};
	};
	template<> struct equal_to<CallEdge>
	{
		bool operator()( const CallEdge& e1, const CallEdge& e2) const
		{
			return equal_to<FuncNode>()(e1.src, e2.src)
			 && equal_to<FuncNode>()(e1.dst, e2.dst);
		};
	};
}
typedef unordered_set<CallEdge> SetOfCallEdge;
typedef unordered_set<FuncNode> SetOfFuncNode;

class CallGraph
{
	public:
		SetOfCallEdge e;
		SetOfFuncNode n;
		void add(CallEdge e)
		{
			this->e.insert(e);
		}

		void add(FuncNode n)
		{
			this->n.insert(n);
		}

};

static const char ENTER[] = "enter:";
static const char EXIT[] = "exit:";

//the return value shows which of "enter:" of "exit:" the entry is.
// returning -1 indicates an error.
// @arg(buf) must be '\0' ended string.
static int parse_line_funcHist(const char* buf, FuncNode& n)
{
	const char *b = buf;
	while(b[0] == '\t')
		b++;

	//check which of "enter:" or "exit:" the entry is 
	int ret;
	if(strncmp(b, ENTER, strlen(ENTER))==0){
		ret   = 1;
		b    += strlen(ENTER);
	} else {
		ret = 0;
		b    += strlen(EXIT);
	}

	char _name[4096]="";
	//get the name of the function
	const char *s = strchr(b ,'@');
	if(s == NULL) return -1; //error check
	strncpy(_name, b, s-b);
	n.func = string(_name);
	
	//get the path of a file including the function
	strncpy(_name, s+1, sizeof(_name));
	n.path = string(_name);

	return ret;
}

static void write_call_graph(const CallGraph& cg, ofstream& f)
{
	f << "digraph g {" <<endl;
	for(auto it = cg.e.cbegin(); it != cg.e.cend(); it++){
		FuncNode n = (*it).src;
		f << "\"" << n.func << "@" << n.path << "\"";
		f << " -> ";
		n = (*it).dst;
		f << "\"" << n.func << "@" << n.path << "\"";
		f << ";" << endl;
	}

	//finalized
	f << "}" <<endl;
}

int main(int argc, char* argv[])
{

	CallGraph cg;

	//read each graph
	for(int i=1; i<argc; i++){
		stack<FuncNode> cs; //call stack
		ifstream f(argv[i]);
		if(!f.is_open()) return -1;
		/*
		 * foreach l in getline(f):
		 * 		node = parse(l)
		 * 		if( node is enter: obj):
		 * 			create new edge;
		 * 			add node and edge;
		 * 			push node to the stack
		 *
		 * 		else: //node is exit: obj
		 * 			pop stack
		 */
		static const size_t LBUF = 4096;
		char buf[LBUF]="";
		while(f.getline(buf, LBUF) && f.good()){
			FuncNode n;
			int is_enter;

			is_enter = parse_line_funcHist(buf, n);
			if(is_enter == -1) return -1;
			else if(is_enter == 1){
				/* "enter:" case */
				if(cs.size() > 0){
					const FuncNode prefn = cs.top();
					CallEdge e(prefn, n);
					cg.add(e);
				}
//				cg.add(n); tentatively this is not necessary for creating dot file

				cs.push(n);
			} else {
				/* "exit:" case */
				cs.pop();
			}

		}

		f.close();
	}

	/*** write dot file ***/
	//open output file
	ofstream of("call_graph.dot");
	if(!of.is_open()) return -1;
	write_call_graph(cg, of);

	of.close();
	return 0;
}
