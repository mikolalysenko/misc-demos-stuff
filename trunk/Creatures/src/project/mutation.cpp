#include <iostream>

#include "common/sys_includes.h"

#include "project/circuit.h"
#include "project/genotype.h"
#include "project/mutation.h"

namespace Game
{

const double MUTATION_RATE		= 0.01;

const double NODE_CREATION_RATE	= 0.002;
const double GATE_CREATION_RATE	= 0.002;
const double EDGE_CREATION_RATE = 0.004;
const double WIRE_CREATION_RATE = 0.002;

const double NODE_KILL_RATE 	= 0.008;
const double GATE_KILL_RATE 	= 0.008;
const double EDGE_KILL_RATE 	= 0.008;
const double WIRE_KILL_RATE 	= 0.008;

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
	NxVec3 rv = rand_vec(x);
	rv.normalize();
	
	NxQuat rq;
	rq.fromAngleAxis(nrand(x)*180., rv);
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

void perturb_edge(Genotype& g, int s, int x)
{
	Edge& e = g.edges[s][x];

	if(drand48() < MUTATION_RATE)
	{
		e.rot *= rand_quat();
		e.rot.normalize();
	}
	
	if(drand48() < MUTATION_RATE)
		e.scale *= (2.5 + nrand()) / 2.5;

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
		
	if(drand48() < MUTATION_RATE)
		e.target = rand() % g.nodes.size();
	
	if(drand48() < MUTATION_RATE)
		e.strength += nrand() * 20.;
	
	if(drand48() < MUTATION_RATE)
		e.stiffness += nrand() * 20.;
		
	if(drand48() < MUTATION_RATE)
	{
		e.source = rand() % g.nodes.size();
		g.edges[e.source].push_back(e);
		g.remove_edge(s, x);
	}
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
	if(drand48() < MUTATION_RATE)
		w.direction *= -1;
	if(drand48() < MUTATION_RATE)
		w.node = rand();
	if(drand48() < MUTATION_RATE)
		w.gate = rand();
	if(drand48() < MUTATION_RATE)
		w.gate_type = (GateType)(rand() % 3);
	if(drand48() < MUTATION_RATE)
		w.node_type = (NodeType)(rand() % 2);
}

void create_node(Genotype& genes)
{
	Node tmp;
	tmp.color = NxVec3(pow(drand48()*2.,4.5)/16.,pow(drand48()*2.,4.5)/16.,pow(drand48()*2,4.5)/16.);
	
	tmp.shape = (BodyPartType)(rand() % 2);
	
	//Body part specific information
	tmp.size = rand_vec(5) * 5. + NxVec3(5,5,5);
	tmp.radius = nrand(2) * 10.;
	tmp.length = nrand(2) * 10.;
	
	genes.nodes.push_back(tmp);
	
	vector<Edge> tmp2;
	tmp2.resize(0);
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
	while(tmp.target == x && drand48() < .8)
		tmp.target = rand() % genes.nodes.size();
	
	//Frame of reference for new body part
	tmp.rot = rand_quat(1);
	tmp.scale = nrand() + 1.;
	tmp.reflect = (rand() & 2) ? 1 : -1;
	
	tmp.stiffness = fabsf(nrand(6) * 40.) + 10.;
	tmp.strength = fabsf(nrand() * 200.);
	
	
	//Joint information (must be a hinge)
	NxVec3 s_size = genes.nodes[tmp.source].size;
	
	tmp.s_axis = NxVec3(0,0,0);
	tmp.s_axis[rand()%3]=(rand()%2?1.f:-1.f);
	tmp.s_axis = rand_vec();
	tmp.s_norm = rand_vec();
	tmp.s_point = NxVec3(nrand(3)*s_size.x/1.5,
				nrand(3)*s_size.y/1.5,
				nrand(3)*s_size.z/1.5);


	NxVec3 t_size = genes.nodes[tmp.target].size;
	tmp.t_axis = NxVec3(0,0,0);
	tmp.t_axis[rand()%3]=(rand()%2?1.f:-1.f);
	tmp.t_norm = rand_vec();
	tmp.t_point = NxVec3(nrand(3)*t_size.x/1.5,
				nrand(3)*t_size.y/1.5,
				nrand(3)*t_size.z/1.5);
				
	
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
	int N = g.nodes.size();
	if(g.nodes.size() <= 1)
		return;
	g.remove_node(rand() % N);
}

void remove_edge(Genotype& g, int i)
{
	int N = g.edges[i].size();
	if(N == 0)
		return;
	g.remove_edge(i, rand() % N);
}

void remove_gate(Genotype& g, int n)
{
	int N = g.nodes[n].gates.size();
	if(N == 0)
		return;
	g.remove_gate(n, rand() % N);
}

void remove_wire(Genotype& g, int n, int gt)
{
	int N = g.nodes[n].gates[gt].wires.size();
	if(N == 0)
		return;
	g.remove_wire(n, gt, rand() % N);
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
				remove_wire(genes, i, j);
			}
		}
		
		//Randomly kill off gates
		while(drand48() < GATE_KILL_RATE)
		{
			remove_gate(genes, i);
		}
		

		//Generate new edges
		while(drand48() < EDGE_CREATION_RATE)
		{
			create_edge(genes, i);
		}

		//Perturb edges
		for(int j=0; j<(int)genes.edges[i].size(); j++)
		{
			perturb_edge(genes, i, j);
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
Genotype randomCreature(int N, int E, int G, int W)
{
	Genotype res;
	
	for(int i=rand()%(N-1); i<N; i++)
	{
		create_node(res);
		
		for(int j=rand()%G; j<G; j++)
		{
			Node& n = res.nodes[res.nodes.size()-1];
			create_gate(n);
			GateNode& g = n.gates[n.gates.size()-1];
			for(int k=rand()%W; k<W; k++)
				create_wire(g);
		}
		
		for(int j=rand()%E; j<E-1; j++)
			create_edge(res, res.nodes.size()-1);	
	}
	
	res.root = rand() % res.nodes.size();
	
	res.normalize();
	
	return res;
}

};
