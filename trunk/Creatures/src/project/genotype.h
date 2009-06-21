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

//Reference types
typedef int					node_ref;
typedef pair<int,int>		edge_ref;
typedef pair<int,int>		gate_ref;
typedef pair<gate_ref,int>	wire_ref;


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
	//Gate container stuff
	int parent_gate, idx;
	
	//Pointer stuff
	NodeType node_type;
	GateType gate_type;
	int node, gate, direction;
		
	//How this works --
	//	the node descriptor tells which node to look for the gate in, and the gate
	//	number tells which gate in that node to connect to.  Sensors, effectors and logic are handled separately.
		
	void save(ostream& os) const;
	static GateEdge load(istream& is);
	
	//Returns the reference to this wire
	wire_ref	ref() const;
};

//A circuit node
struct GateNode
{
	//Index/parent stuff
	int container, idx;

	//An internal node
	vector<float>	params;
	string			name;
	
	//Serialization
	void save(ostream& os) const;
	static GateNode load(istream& is);
	
	//Connections to other gates
	vector<GateEdge>	wires;

	//Returns a handle
	gate_ref	ref() const;
};

//A link between two phenotypes
struct Edge
{
	//Index of edge
	int idx;

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

	//Returns a handle
	edge_ref	ref() const;
};

//Generates some body part
struct Node
{
	//Index of node
	int idx;

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
	
	//Control circuits associated to this particular body part.
	vector<GateNode> gates;

	//Returns a handle
	node_ref	ref() const;
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
	Genotype(const Genotype& t) : nodes(t.nodes), edges(t.edges) {}
	Genotype operator=(const Genotype& t)
	{
		nodes = t.nodes;
		edges = t.edges;
		return *this;
	}
	
	//Graph modification/traversal stuff
	node_ref	add_node();
	edge_ref	add_edge(node_ref a, node_ref b);
	gate_ref	add_gate(node_ref n);
	wire_ref	add_wire(gate_ref a, gate_ref b);
	
	void		remove_node(node_ref n);
	void		remove_edge(edge_ref e);
	void		remove_gate(gate_ref g);
	void		remove_wire(wire_ref w);

	Node&		get_node(node_ref);
	Edge&		get_edge(edge_ref);
	Gate&		get_gate(gate_ref);
	Wire&		get_wire(wire_ref);
	
	node_ref	random_node();
	edge_ref	random_edge();
	gate_ref	random_gate();
	wire_ref	random_wire();
	
	//Save/load phenotypes from file
	void save(ostream& os) const;
	static Genotype load(istream& is);
	
	//Generates a creature from this graph
	Creature* createCreature(NxMat34 pose);
	Creature* createCreature() { NxMat34 tmp; tmp.id(); return createCreature(tmp); }
};

};


#endif

