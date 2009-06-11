#include <vector>
#include <iostream>
#include <string>
#include <cassert>

#include "common/sys_includes.h"
#include "common/physics.h"

#include "project/creature.h"
#include "project/phenotype.h"

using namespace std;
using namespace Common;

namespace Game
{

void assert_token(istream& is, const string& token)
{
	string tmp;
	if(!(is >> tmp) || tmp != token)
		throw "Expected token: " + token;
}

//Node saving
void Node::save(ostream& os) const
{
	os << "NODE " << color.x << ' ' << color.y << ' ' << color.z << ' ';
	
	switch(shape)
	{
		case BODY_BOX:
			os << "BOX " << size.x << ' ' << size.y << ' ' << size.z;
		break;
		
		case BODY_CAPSULE:
			os << "CAPSULE " << length << ' ' << radius;
		break;
		
		case BODY_SPHERE:
			os << "SPHERE " << radius;
		break;
		
		default:
			assert(false);
	}
	
	os << endl;
}

//Node loading
Node Node::load(istream& is)
{
	assert_token(is, "NODE");

	Node res;
	if(!(is >> res.color.x >> res.color.y >> res.color.z))
		throw "Invalid color data for node";
	
	string s;
	if(!(is >> s))
		throw "Invalid string for node shape";
	
	if(s == "BOX")
	{
		if(!(is >> res.size.x >> res.size.y >> res.size.z))
			throw "Invalid box shape";
	}
	else if(s == "CAPSULE")
	{
		if(!(is >> res.length >> res.radius))
			throw "Invalid capsule shape";
	}
	else if(s == "SPHERE")
	{
		if(!(is >> res.radius))
			throw "Invalid sphere shape";	
	}
	else
	{
		throw "Unknown node shape: " + s;
	}
	
	return res;
}

//Edge saving
void Edge::save(ostream& os) const
{
	os << "EDGE " 
		<< source  << ' ' << target  << ' '
		<< rot.w   << ' ' << rot.x   << ' ' << rot.y   << ' ' << rot.z << ' '
		<< trans.x << ' ' << trans.y << ' ' << trans.z << ' '
		<< axis.x  << ' ' << axis.y  << ' ' << axis.z
		<< endl;
}

//Loading an edge
Edge Edge::load(istream& is)
{
	assert_token(is, "EDGE");
	Edge res;
	if(!(is
		>> res.source  >> res.target
		>> res.rot.w   >> res.rot.x   >> res.rot.y   >> res.rot.z
		>> res.trans.x >> res.trans.y >> res.trans.z
		>> res.axis.x  >> res.axis.y  >> res.axis.z))
			throw "Invalid edge";
	return res;
}


//Save the phenotype
void Phenotype::save(ostream& os) const
{
	os	<< "PHENOTYPE" << endl
		<< nodes.size() << endl;
	
	for(int i=0; i<(int)nodes.size(); i++)
	{
		nodes[i].save(os);
		os << edges[i].size() << endl;
		for(int j=0; j<(int)edges[i].size(); j++)
			edges[i][j].save(os);
	}
}

//Restore the phenotype
Phenotype Phenotype::load(istream& is)
{
	assert_token(is, "PHENOTYPE");
	
	int n_nodes;
	if(!(is >> n_nodes))
		throw "Error reading number of nodes/edges";
		
	Phenotype res;
	res.nodes.resize(n_nodes);
	res.edges.resize(n_nodes);
	
	for(int i=0; i<n_nodes; i++)
	{
		res.nodes[i] = Node::load(is);
	
		int n_edges;
		is >> n_edges;
		
		res.edges[i].resize(n_edges);
		for(int j=0; j<n_edges; j++)
		{
			res.edges[i][j] = Edge::load(is);
			assert(res.edges[i][j].source == i);
			assert(res.edges[i][j].target < n_nodes && res.edges[i][j].target >= 0);
		}
	}
	
	return res;
}




};
