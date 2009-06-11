#ifndef PHENOTYPE_H
#define PHENOTYPE_H


#include "common/sys_includes.h"
#include "project/creature.h"

#include <vector>
#include <utility>

namespace Game
{

using namespace std;


enum BodyPartType
{
	BODY_BOX,
	BODY_CAPSULE,
	BODY_SPHERE,
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
};


//A link between two phenotypes
struct Edge
{
	//Number of times this edge may be traversed
	int source, target;
	
	//Frame of reference for new body part
	NxQuat rot;
	NxVec3 trans;
	
	//Joint information (must be a hinge)
	NxVec3	axis;

	//Serialization
	void save(ostream& os) const;
	static Edge load(istream& is);
};



//A creature phenotype
struct Genotype
{
	
	//Phenotype graph
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

	//Save/load phenotypes from file
	void save(ostream& os) const;
	static Genotype load(istream& is);
	
	//Generates a creature from this graph
	Creature* createCreature() const;
	
	//Creates a mutated version of this phenotype
	Genotype mutate() const;
};


};


#endif

