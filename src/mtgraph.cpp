#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <cmath>
#include <limits>
#include <map>
#include <sys/time.h>
#include <thread>
#include <queue>
#include "queue.hpp"
#include "mtgraph.hpp"
int main(int argc, char *argv[])
{
//	double epsilon, damping;
//	int maxIterations;
//	bool verbose = false;
//	if (argc < 5) {
//		std::cout << "Usage: " << argv[0] << " <edge list filename> <epsilon> <damping factor> <max iterations> [verbose]" << std::endl;
//		return 1;
//	}
//	std::cout << "I am graph processing system!\n";
//	Graph g2;
//	readGraph(g2, argv[1]);
//	epsilon = atof(argv[2]);
//	damping = atof(argv[3]);
//	maxIterations = atoi(argv[4]);
//	if (argc > 5) {
//		std::cout << "verbose mode enabled" << std::endl;
//		verbose = true;
//	}
//	else {
//		std::cout << "verbose mode disabled" << std::endl;
//	}
//	std::cout << "epsilon: " << epsilon << std::endl;
//	std::cout << "damping: " << damping << std::endl;
//	std::cout << "max iterations: " << maxIterations << std::endl;
//	if (verbose) {
//		g2.print();
//		g2.printRank();
//	}
//	struct timeval start, end;
//	double elapsed;
//	gettimeofday(&start, NULL);
//	g2.pagerank(damping, epsilon, maxIterations);
////    g2.pagerank(0.5, .01, 12);
//	gettimeofday(&end, NULL);
//	elapsed = ((end.tv_sec-start.tv_sec)*1000000 + end.tv_usec-start.tv_usec) / (double)1000000;
//	if (verbose) {
//		g2.printRank();
//	}
//	std::cout << "Number of nodes: " << g2.size() << std::endl;
//	std::cout << "Number of edges: " << g2.edgeCount() << std::endl;
//	std::cout << elapsed << std::endl;
}


// TODO implement this function for pagerank
void Graph::update (int nodeId) {
    // TODO perform pagerank operations

    // TODO add new node to the queue if necessary
}

// main task of each thread
// pulls from task queue and runs tasks
// WARNING - not  thread safe queue at this time
void Graph::threadMain () {
    while (!this->done) {
        // TODO make queue thread safe
        // this will not work right now - the empty check may return false
        // but another thread may pop something in between and then we will have undefined results
        if (!taskQueue->empty()) {
	  int nodeId = taskQueue->front();
	  taskQueue->dequeue();

            // TODO Add locking of neighbor nodes
            // I did not add this because I do not completely understand how the graphlab model locking works
            this->update(nodeId);
        }
    }
}

// launches worker threads
void Graph::start (int numThreads) {
  std::vector<std::thread*> threads;
  for (int i = 0; i < numThreads; i++) {
    threads.push_back(new std::thread(&Graph::threadMain, this));
  }
  for (int i = 0; i < numThreads; i++) {
    threads[i]->join();
  }

  for(int i=0;i<numThreads;i++) {
    delete threads[i];
  }
}

void Graph::addVertex (int weight) {
	//GraphNode *node = new GraphNode(weight);
	vertices[this->size()] = new GraphNode(weight);
}

void Graph::addEdge (int from, int to) {
	vertices[from]->neighbors.push_back(to);
	vertices[to]->inEdges.push_back(from);
}

//TODO iterate over keys to support non sequential keys
void Graph::print () {
	std::vector<int>::const_iterator it;
	for (int i = 0; i < vertices.size(); i++) {
		std::cout << i << "-> ";
		for (it = vertices[i]->neighbors.begin(); it != vertices[i]->neighbors.end(); it++) {
			std::cout << (*it) << ' ';
		}
		std::cout << std::endl;
	}
}

//TODO iterate over keys to support non sequential keys
void Graph::printRank () {
	std::list<int>::const_iterator it;
	for (int i = 0; i < vertices.size(); i++) {
		std::cout << i << "-> " << vertices[i]->data->weight << std::endl;
	}
}

int Graph::size () {
	return vertices.size();
}

int Graph::edgeCount () {
	int total = 0;
    for (int i = 0; i < vertices.size(); i++) {
        total += vertices[i]->neighbors.size();
    }
//	std::unordered_map<int, GraphNode*>::const_iterator it;
//	for (it = vertices.begin(); it != vertices.end(); it++) {
//		total += (*it)->neighbors.size();
//	}
	return total;
}

/**
 * NON-THREADED VERSION
 */
void Graph::pagerank (double alpha, double epsilon, int maxIterations) {
	int n = size();
	double linkResult, delta, total_delta = std::numeric_limits<double>::max(), old;
    int iteration = 0;
	std::list<int>::const_iterator inEdgeIter;
	std::vector<GraphNode*>::const_iterator nodePtrIter;
    int inEdgeId;
	GraphNode *v;
	int nodeTouchCount = 0, edgeTouchCount = 0;
    while (iteration < maxIterations && total_delta >= epsilon) {
        total_delta = 0;
        for (int i = 0; i < this->size(); i++) {
            nodeTouchCount++;
            v = vertices[i];
            linkResult = 0;
            for (int i = 0; i < v->inEdges.size(); i++) {
                // For now, we use 1 for edge weight
                inEdgeId = v->inEdges[i];
                linkResult += (1.0 / vertices[inEdgeId]->neighbors.size()) * vertices[inEdgeId]->data->weight;
                edgeTouchCount++;
            }
            old = v->data->weight;
            v->data->weight = n*((alpha / n) + (1 - alpha) * linkResult/n);
            delta = fabs(v->data->weight - old);
            total_delta += delta;
        }
        std::cout << "Delta: " << total_delta << std::endl;
        iteration++;
    }
    std::cout << "Iterations completed: " << iteration << std::endl;
	std::cout << "Vertices touched: " << nodeTouchCount << std::endl;
	std::cout << "Edges touched: " << edgeTouchCount << std::endl;
}

void printList (std::list<int> ll) {
	std::list<int>::const_iterator it;
	for (it = ll.begin(); it != ll.end(); it++) {
		std::cout << (*it) << ' ';
	}
}

void printVec (std::vector<GraphNode> ll) {
	std::vector<GraphNode>::const_iterator it;
	for (it = ll.begin(); it != ll.end(); it++) {
		std::cout << it->data->weight << ' ';
	}
}

void readGraphEdges (Graph g, std::string filename) {
	std::ifstream infile(filename);
	int from, to;
	while (infile >> from >> to) {
		g.addEdge(from-1, to-1);
	}
}

// expects to/from edge pairs
// id starts at 1
void readGraph (Graph& g, std::string filename) {
	std::ifstream infile(filename);
	int id, to, from, maxId = 0;
	int minId = std::numeric_limits<int>::max();
	while (infile >> id) {
		maxId = std::max(id-1, maxId);
		minId = std::min(id-1, minId);
	}
	for (int i = minId; i <= maxId; i++) {
		// all same starting weight
		g.addVertex(1);
	}
	// return to beginning of file
	infile.clear();
	infile.seekg(0, std::ios::beg);
	while (infile >> from >> std::skipws >> to) {
		g.addEdge(from-1, to-1);
	}
}
