#ifndef PHENOTYPE_H
#define PHENOTYPE_H


#include "common/sys_includes.h"
#include "project/creature.h"
#include "project/circuit.h"

#include <vector>
#include <utility>

namespace Game
{

using namespace std;

enum NodeType
{
	NODE_CURRENT,
	NODE_CHILD,
};

enum GateType
{
	GATE_CONTROL,
	GATE_SENSOR,
	GATE_EFFECTOR,
};

//A GateEdge represents a wire in the creature's control system
struct GateEdge
{
	//Pointer stuff
	NodeType node_type;
	GateType gate_type;
	int node, gate, direction;
		
	//How this works --
	//	the node descriptor tells which node to look for the gate in, and the gate
	//	number tells which gate in that node to connect to.  Sensors, effectors and logic are handled separately.
		
	void save(ostream& os) const;
	static GateEdge load(istream& is);
};

//A circuit node
struct GateNode
{
	//An internal node
	vector<float>	params;
	string			name;
	
	//Serialization
	void save(ostream& os) const;
	static GateNode load(istream& is);
	
	//Connections to other gates
	vector<GateEdge>	wires;
};

//A link between two phenotypes
struct Edge
{
	//If true, edge is active
	bool marked;

	//Number of times this edge may be traversed
	int source, target;
	
	//Frame of reference for new body part
	NxQuat rot;
	float scale;
	int reflect;
	
	//Joint information (must be a hinge)
	NxVec3	s_axis,  t_axis,
			s_norm,  t_norm,
			s_point, t_point;

	//Constructor
	Edge() : marked(false) {}	

	//Serialization
	void save(ostream& os) const;
	static Edge load(istream& is);
	
	//Normalizes the edge
	void normalize(struct Genotype& gen);
};

//Generates some body part
struct Node
{
	//Color of this body part
	NxVec3	color;
	
	//Body part information
	BodyPartType shape;
	
	//Body part specific information
	NxVec3 size;
	float radius, length;
	
	//Serialization
	void save(ostream& os) const;
	static Node load(istream& is);
	
	//Gets the closest point to the surface of this body
	NxVec3 closest_pt(const NxVec3& x);
	
	//Normalizes the genes
	void normalize();
	
	//Control circuits associated to this particular body part.
	vector<GateNode> gates;
};


//A creature phenotype
struct Genotype
{
	//Phenotype graph
	int						root;
	vector<Node>			nodes;
	vector< vector<Edge> >	edges;
	
	//Constructors
	Genotype() {}
	Genotype(const Genotype& t) : root(t.root), nodes(t.nodes), edges(t.edges) {}
	Genotype operator=(const Genotype& t)
	{
		nodes = t.nodes;
		edges = t.edges;
		return *this;
	}
	
	//Random node accessors
	Node&		random_node();
	Edge&		random_edge();
	GateNode&	random_gate();
	GateEdge&	random_gate();
	
	//Save/load phenotypes from file
	void save(ostream& os) const;
	static Genotype load(istream& is);
	
	//Generates a creature from this graph
	Creature* createCreature(NxMat34 pose);
	Creature* createCreature() { NxMat34 tmp; tmp.id(); return createCreature(tmp); }
	
	void normalize();
};

};


#endif
