#include "project/circuit.h"
#include "project/genotype.h"
#include "project/mutation.h"

namespace Game
{

const double MUTATION_RATE		= 0.02;

const double NODE_CREATION_RATE	= 0.02;
const double GATE_CREATION_RATE	= 0.02;
const double EDGE_CREATION_RATE = 0.02;
const double WIRE_CREATION_RATE = 0.02;

const double NODE_KILL_RATE 	= 0.02;
const double GATE_KILL_RATE 	= 0.02;
const double EDGE_KILL_RATE 	= 0.02;
const double WIRE_KILL_RATE 	= 0.02;

const double ROOT_RELOC_RATE	= 0.001;

float nrand(int x = 5)
{
	float s = 0;
	for(int i=0; i<x; i++)
		s += drand48() - .5;
	return s;
}

NxVec3 rand_vec(int x = 5)
{
	return NxVec3(nrand(x), nrand(x), nrand(x));
}

void perturb_node(Node& n)
{
	n.color += rand_vec();
	n.size += rand_vec();
	n.radius += nrand();
	n.length += nrand();
}

void perturb_edge(Edge& e)
{
	e.rot = e.rot * NxQuat(nrand(), nrand(), nrand(), nrand());
	e.rot.normalize();
	
	e.scale *= abs(nrand());

	if(rand() % 2)
		e.reflect *= -1;
	
	e.s_axis += rand_vec();
	e.s_norm += rand_vec();
	e.s_point += rand_vec() * 5;

	e.t_axis += rand_vec();
	e.t_norm += rand_vec();
	e.t_point += rand_vec() * 5;
}


void perturb_gate(GateNode& g)
{
	if(drand48() < 0.1)
	{
		//Switch gate type
		g.name = randomGateName();
		GateFactory* f = getFactory(g.name);
		g.params = f->generateParams();
	}
	else for(int i=0; i<g.params.size(); i++)
	{
		g.params[i] += nrand();
	}
}

void perturb_wire(GateEdge& w)
{
}

void create_node(Genotype& genes)
{
}

void create_edge(Genotype& genes, int x)
{
}

void create_gate(Node& n)
{
}

void create_wire(GateNode& g)
{
}


void remove_node(Genotype& g)
{
}

void remove_edge(Genotype& g, int i)
{
}

void remove_gate(Node& n)
{
}

void remove_wire(GateNode& g)
{
}

void mutate(Genotype& genes)
{
	//Generate new nodes
	while(drand48() < NODE_CREATION_RATE)
	{
		create_node(genes);
	}

	//Modify nodes
	for(int i=0; i<genes.nodes.size(); i++)
	{
		Node& n = genes.node[i];
		
		//Perturb nodes
		if(drand48() < MUTATION_RATE)
		{
			perturb_node(n);
		}

		//Generate random gate
		while(drand48() < GATE_CREATION_RATE)
		{
			create_gate(n);
		}
		
		//Perturb gates
		for(int j=0; j<n.gates.size(); i++)
		{
			GateNode& g = n.gates[j];		
		
			if(drand48() < MUTATION_RATE)
			{
				perturb_gate(g);
			}
			
			while(drand48() < WIRE_CREATION_RATE)
			{
				create_wire(g);
			}

			for(int k=0; k<g.wires.size(); k++)
			{
				perturb_wire(g.wires[k]);
			}
			
			while(drand48() < WIRE_KILL_RATE)
			{
				remove_wire(g);
			}
		}
		
		//Randomly kill off gates
		while(drand48() < GATE_KILL_RATE)
		{
			remove_gate(n):
		}
		

		//Generate new edges
		while(drand48() < EDGE_CREATION_RATE)
		{
			create_edge(genes, i);
		}
	
		//Perturb edges
		for(int j=0; j<genes.edges[i].size(); j++)
		{
			Edge& e = genes.edges[i][j];
			
			if(drand48() < MUTATION_RATE)
			{
				perturb_edge(e);
			}
		}
		
		while(drand48() < EDGE_KILL_RATE)
		{
			remove_edge(genes, i);
		}
	}
	
	while(drand48() < NODE_KILL_RATE && nodes.size() > 1)
	{
		remove_node(genes);
	}
	
	if(drand48() < ROOT_RELOC_RATE)
	{
		genes.root = rand() % (int)genes.nodes.size();
	}
	
	//Normalize genes
	genes.normalize();
}

Genotype crossover(Genotype& a, Genotype& b)
{
	Genotype res;
	
	
	//TODO: Implement this
	assert(false);
	
	return res;
}

Genotype graft(Genotype& a, Genotype& b)
{
	Genotype res;
	
	//TODO: Implement this
	assert(false);
	
	return res;
}

};
