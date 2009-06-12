#include <vector>
#include <iostream>
#include <string>
#include <cassert>

#include "common/sys_includes.h"
#include "common/physics.h"

#include "project/creature.h"
#include "project/genotype.h"

using namespace std;
using namespace Common;

namespace Game
{

//Checks that the given token is in the stream
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
		res.shape = BODY_BOX;
		if(!(is >> res.size.x >> res.size.y >> res.size.z))
			throw "Invalid box shape";
	}
	else if(s == "CAPSULE")
	{
		res.shape = BODY_CAPSULE;
		if(!(is >> res.length >> res.radius))
			throw "Invalid capsule shape";
	}
	else if(s == "SPHERE")
	{
		res.shape = BODY_SPHERE;
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
		<< scale
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
		>> res.scale
		>> res.rot.w   >> res.rot.x   >> res.rot.y   >> res.rot.z
		>> res.trans.x >> res.trans.y >> res.trans.z
		>> res.axis.x  >> res.axis.y  >> res.axis.z))
			throw "Invalid edge";
	return res;
}


//Save the phenotype
void Genotype::save(ostream& os) const
{
	os	<< "GENOTYPE" << endl
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
Genotype Genotype::load(istream& is)
{
	assert_token(is, "GENOTYPE");
	
	int root, n_nodes;
	if(!(is >> root >> n_nodes))
		throw "Error reading root/number of nodes";
		
	Genotype res;
	res.root = root;
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


//Generates a body part
BodyPart* genCreatureRec(
	Genotype&			genes,
	Creature*			creature,
	int 				n,
	NxMat34				pose,
	float				scale)
{
	//TODO: Implement this

	Node& node = genes.nodes[n];

	//Generate the part at this node
	BodyPart* res;
	switch(node.shape)
	{
		case BODY_BOX:
			res = new BodyPart(creature, node.color, pose, node.size * scale);
		break;
	
		case BODY_SPHERE:
		break;
	
		case BODY_CAPSULE:
		break;
	}

	//If fails, then return NULL
	if(res->actor == NULL)
	{
		delete res;
		return NULL;
	}

	//Add body part to body vector
	creature->body.push_back(res);
	
	//For each edge:
	for(int i=0; i<(int)genes.edges[n].size(); i++)
	{
		Edge& edge = genes.edges[n][i];
		
		//Check for mark
		if(edge.marked)
			continue;
		edge.marked = true;
		
		//Transform frame
		NxMat34 xform;
		xform.t = edge.trans;
		xform.M = NxMat33(edge.rot);
		NxMat34 npose = xform * pose;
		
		//Generate new part
		BodyPart* tmp = genCreatureRec(
			genes, 
			creature, 
			edge.target, 
			npose, 
			scale * edge.scale);
		
		//If failed, then skip it
		if(tmp == NULL)
		{
			edge.marked = false;
			continue;
		}

		//Create joint and link parts
		NxRevoluteJointDesc joint_desc;
		joint_desc.setToDefault();
		joint_desc.actor[0] = res->actor;
		joint_desc.actor[1] = tmp->actor;
		joint_desc.setGlobalAxis(pose.M * edge.axis);
		joint_desc.setGlobalAnchor( pose * (0.5 * edge.trans));
		
	    NxRevoluteJoint * joint = static_cast<NxRevoluteJoint*>(scene->createJoint(joint_desc));
	    
	    res->attachPart(tmp, joint);

		//Unmark used edge
	 	edge.marked = false;
	}
	
	return res;
}
	
	

//Constructs a creature from the genotype
Creature* Genotype::createCreature(NxMat34 pose)
{
	Creature* res = new Creature();

	//Generate a body schema
	BodyPart* b = genCreatureRec(*this, res, root, pose, 1.);
	
	//Check for failure
	if(b == NULL)
	{
		delete res;
		return NULL;
	}
	
	//Attach root and done
	res->root = b;
	return res;
}




};
