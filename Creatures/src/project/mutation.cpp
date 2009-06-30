#include <iostream>

#include "common/sys_includes.h"

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

NxQuat rand_quat(int x = 5)
{
	NxQuat rq;
	rq.setXYZW(nrand(x), nrand(x), nrand(x), nrand(x));
	rq.normalize();
	return rq;
}

void perturb_node(Node& n)
{
	if(drand48() < MUTATION_RATE)
		n.color = rand_vec() + 0.3 * n.color;
	if(drand48() < MUTATION_RATE)
		n.size += rand_vec();
	if(drand48() < MUTATION_RATE)
		n.radius += nrand();
	if(drand48() < MUTATION_RATE)
		n.length += nrand();
}

void perturb_edge(Edge& e)
{
	if(drand48() < MUTATION_RATE)
	{
		e.rot *= rand_quat();
		e.rot.normalize();
	}
	
	if(drand48() < MUTATION_RATE)
	e.scale *= fabsf(nrand());

	if(drand48() < MUTATION_RATE)
		e.reflect *= -1;
	
	if(drand48() < MUTATION_RATE)
		e.s_axis += rand_vec();
	if(drand48() < MUTATION_RATE)
		e.s_norm += rand_vec();
	if(drand48() < MUTATION_RATE)
		e.s_point += rand_vec() * 5;

	if(drand48() < MUTATION_RATE)
		e.t_axis += rand_vec();
	if(drand48() < MUTATION_RATE)
		e.t_norm += rand_vec();
	if(drand48() < MUTATION_RATE)
		e.t_point += rand_vec() * 5;
	
	//TODO: Maybe switch topology
}


void perturb_gate(GateNode& g)
{
	if(drand48() < MUTATION_RATE * 0.1)
	{
		//Switch gate type
		g.name = randomGateName();
		GateFactory* f = getFactory(g.name);
		g.params = f->generateParams();
	}
	else for(int i=0; i<(int)g.params.size(); i++)
	{
		if(drand48() < MUTATION_RATE)
			g.params[i] += nrand();
	}
}

void perturb_wire(GateEdge& w)
{
	//TODO: Maybe switch target here?
}

void create_node(Genotype& genes)
{
	Node tmp;
	tmp.color = NxVec3(pow(drand48()*2.,4.)/16.,pow(drand48()*2.,4.)/16.,pow(drand48()*2,4.)/16.);
	
	tmp.shape = (BodyPartType)(rand() % 2); //TODO: When capsules are implemented make this a 3
	
	//Body part specific information
	tmp.size = rand_vec(2) * 10.;
	tmp.radius = nrand(2) * 10.;
	tmp.length = nrand(2) * 10.;
	
	genes.nodes.push_back(tmp);
	
	vector<Edge> tmp2;
	genes.edges.push_back(tmp2);
}

void create_edge(Genotype& genes, int x)
{
	//Roll dice to select target
	Edge tmp;
	
	tmp.marked = false;

	//Number of times this edge may be traversed
	tmp.source = x;
	tmp.target = rand() % genes.nodes.size();
	
	//Frame of reference for new body part
	tmp.rot = rand_quat(1);
	tmp.scale = nrand();
	tmp.reflect = (rand() & 2) ? 1 : -1;
	
	//Joint information (must be a hinge)
	tmp.s_axis = rand_vec();
	tmp.s_norm = rand_vec();
	tmp.s_point = rand_vec(3)*4.;
	tmp.t_axis = rand_vec();
	tmp.t_norm = rand_vec();
	tmp.t_point = rand_vec(3)*4.;
	
	genes.edges[x].push_back(tmp);
}

void create_gate(Node& n)
{
	GateNode tmp;
	tmp.name = randomGateName();
	GateFactory* f = getFactory(tmp.name);
	tmp.params = f->generateParams();
	
	n.gates.push_back(tmp);
}

void create_wire(GateNode& g)
{
	GateEdge tmp;
	
	tmp.node_type = (NodeType)(rand() % 2);
	tmp.gate_type = (GateType)(rand() % 3);
	tmp.node = rand();
	tmp.gate = rand();
	tmp.direction = (rand()&2) ? 1 : -1;

	g.wires.push_back(tmp);
}


void remove_node(Genotype& g)
{
	//TODO: Implement me!
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
	for(int i=0; i<(int)genes.nodes.size(); i++)
	{
		Node& n = genes.nodes[i];
		
		//Perturb nodes
		perturb_node(n);

		//Generate random gate
		while(drand48() < GATE_CREATION_RATE)
		{
			create_gate(n);
		}
		
		//Perturb gates
		for(int j=0; j<(int)n.gates.size(); j++)
		{
			GateNode& g = n.gates[j];		
			perturb_gate(g);
			
			while(drand48() < WIRE_CREATION_RATE)
			{
				create_wire(g);
			}

			for(int k=0; k<(int)g.wires.size(); k++)
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
			remove_gate(n);
		}
		

		//Generate new edges
		while(drand48() < EDGE_CREATION_RATE)
		{
			create_edge(genes, i);
		}
	
		//Perturb edges
		for(int j=0; j<(int)genes.edges[i].size(); j++)
		{
			Edge& e = genes.edges[i][j];
			perturb_edge(e);
		}
		
		while(drand48() < EDGE_KILL_RATE)
		{
			remove_edge(genes, i);
		}
	}
	
	while(drand48() < NODE_KILL_RATE && genes.nodes.size() > 1)
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

//Generates a random creature
Genotype randomCreature()
{
	Genotype res;
	
	res.root = 0;
	for(int i=rand()%4; i<5; i++)
	{
		create_node(res);
		
		for(int j=rand()%3; j<3; j++)
		{
			Node& n = res.nodes[res.nodes.size()-1];
			create_gate(n);
			GateNode& g = n.gates[n.gates.size()-1];
			for(int k=rand()%2; k<2; k++)
				create_wire(g);
		}
		
		for(int j=rand()%3; j<3; j++)
			create_edge(res, res.nodes.size()-1);	
	}
	
	res.normalize();
	
	res.save(cout);
	
	return res;
}

};
