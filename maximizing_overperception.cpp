
/*
PROGRAM: maximizing_overperception.cpp
AUTHOR: Joseph M. Abruzzo

This code runs the empirical test of the attacker's algorithm on sample network
data. It takes in a graph specified by the user (either com-youtube or com-orkut),
runs the greedy algorithm and top-k degree procedures on the graph data, and 
prints the output to the console, where it can then be plotted.
*/

#include <iostream>
#include <fstream>
#include <chrono>
#include <set>
#include <map>

using namespace std;

const char POUND = '#';
const char PERCENT = '%';


// Change these to change the values of k that the algorithms use. In order,
// the parameters are:
//
// PARAM_K_MIN: the minimum value of k used by the algorithms
// PARAM_K_MAX: the maximum value of k used by the algorithms
// K_INCREMENT: the interval at which the algorithms should print data
const int PARAM_K_MIN = 100;
const int PARAM_K_MAX = 2500;
const int K_INCREMENT = 100;

// Graph file names - un-comment the graph file the user wants to run the
// algorithms on
//const string GRAPH_FILE_NAME = "orkut/com-orkut.ungraph.txt";
const string GRAPH_FILE_NAME = "youtube/com-youtube.ungraph.txt";


// Sorts a vector of (score, node) pairs in descending order
void sort_descending(vector<pair<double, int> >& Q)
{
	sort(Q.begin(), Q.end(), greater<>());
}


// Calculates the value of the objective function g for a set of nodes S
double g(vector<int>& S, map<int, vector<int> >& A, int n)
{
	set<int> affected_set;
	map<int, bool> active;

	// For all nodes s in S, indicate that s is active and that its neighbors
	// have an active neighbor
	for (int s : S) {

		active[s] = true;

		for (int t : A[s]) {
			affected_set.insert(t);
		}

	}

	int value = 0;

	// For all nodes with an active neighbor, compute the number of active 
	// neighbors and then test if the ratio between this number and the
	// degree of the node is greater than the ratio of the size of the set S 
	// to n - this is equivalent to testing whether the node overperceives
	// the behavior
	for (int u : affected_set) {

		double score_u = 0.0;

		for (int v : A[u]) {

			if (active[v]) {
				score_u++;
			}

		}

		if (score_u / A[u].size() > 1.0 * S.size() / n) {
			value++;
		}

	}

	return 1.0 * value / n;

}


// Calculates the value of the auxillary function g_hat for a set of nodes S
double g_hat(vector<int>& S, map<int, vector<int> >& A, int n)
{

	set<int> result_set;

	// For all nodes s in S, add all neighbors t of s to the result set - this
	// is equivalent to the number of nodes in the network that have at least
	// one neighbor in S
	for (int s : S) {

		for (int t : A[s]) {
			result_set.insert(t);
		}

	}

	return 1.0 * result_set.size() / n;

}


// Implements the node targeting procedure that selects the top k nodes by degree
void top_k_degree_procedure(vector<int>& V, map<int, vector<int> >& A, int k, string graph_name)
{

	auto start = chrono::high_resolution_clock::now();

	int n = V.size();

	vector<pair<double, int> > Q;

	// Get each node's degree and push the (degree, node) pair into a vector
	for (int i : V) {
		double delta_i = 1.0 * A[i].size();

		Q.push_back(make_pair(delta_i, i));
	}

	sort_descending(Q);

	vector<int> S;

	// For i = 1, ..., k, get the node at the top of the vector and add it to 
	// the set S
	for (int i=0; i<k; i++) {

		int v = Q[0].second;
		Q.erase(Q.begin());

		S.push_back(v);

		auto stop = chrono::high_resolution_clock::now();

		auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

		// If the iteration plus 1 is a multiple of K_INCREMENT, calculate g(S)
		// and print data
		if ((i+1) % K_INCREMENT == 0) {

			double g_S = g(S, A, n);

			cout << "degree, " << graph_name << ", " << to_string(S.size()) << ", " << to_string(g_S) << ", " << duration.count() / 1000.0 << endl;
		}

	}

}


// Implements the node targeting procedure that selects k nodes according to the
// lazy greedy algorithm of Minoux (1977)
void lazy_greedy_procedure(vector<int>& V, map<int, vector<int> >& A, int k, string graph_name)
{

	auto start = chrono::high_resolution_clock::now();

	int n = V.size();

	vector<pair<double, int> > Q;

	// For each node i in V, calculate f({i}) and add (f({i}), i) to a vector
	for (int i : V) {
		double delta_i = 1.0 * A[i].size() / n;

		Q.push_back(make_pair(delta_i, i));
	}

	sort_descending(Q);

	double base_score = 0.0;

	vector<int> S;

	// For i = 1, ..., k, get the node at the top of the vector, check if the marginal
	// gain of the node is larger than the marginal gain of the new node at the top
	// of the vector. If it is, add it to the set S. If not, calculate f(S + v) - f(S)
	// and add (f(S + v) - f(S), v) to the vector and re-sort
	for (int i=0; i<k; i++) {

		while (true) {

			int v = Q[0].second;
			Q.erase(Q.begin());

			vector<int> T = S;
			T.push_back(v);

			double delta_v = g_hat(T, A, n) - base_score;

			if (delta_v >= Q[0].first) {
				S.push_back(v);

				base_score += delta_v;
				break;
			} else {
				Q.push_back(make_pair(delta_v, v));

				sort_descending(Q);
			}

		}


		auto stop = chrono::high_resolution_clock::now();

		auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

		// If the iteration plus 1 is a multiple of K_INCREMENT, calculate g(S)
		// and print data
		if ((i+1) % K_INCREMENT == 0) {

			double g_S = g(S, A, n);

			cout << "greedy, " << graph_name << ", " << to_string(S.size()) << ", " << to_string(g_S) << ", " << duration.count() / 1000.0 << endl;
		}

	}

}


// Reads in the graph data from the .txt files and populates a vector of unique nodes
// in the graph, a vector of edges in the graph, and an adjacency list corresponding to
// the graph's structure
void create_graph(vector<int>& V, vector<pair<int, int> >& E, map<int, vector<int> >& A)
{

	set<int> VSET;

	ifstream infile(GRAPH_FILE_NAME.c_str());

	string line;
	while(getline(infile, line))
	{

		if (!(line.at(0) == POUND || line.at(0) == PERCENT)) {
			istringstream iss(line);
			int from;
			int to;

			iss >> from >> to;

			A[from].push_back(to);
			A[to].push_back(from);

			VSET.insert(to);
			VSET.insert(from);

			E.push_back(make_pair(from, to));
			E.push_back(make_pair(to, from));
			
		}

	}

	for (int v : VSET) {
		V.push_back(v);
	}

}


// Main function that runs the analysis
int main()
{
	
	// Read in graph data
	vector<int> V;
	vector<pair<int, int> > E;
	map<int, vector<int> > A;
	create_graph(V, E, A);

	size_t pos0 = GRAPH_FILE_NAME.find("/") + 1;
	size_t pos1 = GRAPH_FILE_NAME.find("."); 

	string graph_name = GRAPH_FILE_NAME.substr(pos0, pos1 - pos0);

	cout << endl << "GRAPH CREATED: " << graph_name << endl;

	cout << "NODES: " << to_string(V.size()) << endl;
	cout << "EDGES: " << to_string(E.size()) << endl << endl;

	cout << "RUNNING ALGORITHMS..." << endl;
	cout << "algo, network, k, g, time" << endl;

	// Run the lazy greedy procedure, top-k degree procedures on the graph data
	lazy_greedy_procedure(V, A, PARAM_K_MAX, graph_name);

	top_k_degree_procedure(V, A, PARAM_K_MAX, graph_name);
	
	return 0;
}