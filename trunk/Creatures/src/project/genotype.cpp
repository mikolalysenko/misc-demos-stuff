#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <utility>
#include <cmath>

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

//Saves a gate edge
void GateEdge::save(ostream& os) const
{
	os << "WIRE " << source_node << ' ' << source_gate << ' '
				  << target_node << ' ' << target_gate << endl;
}

GateEdge GateEdge::load(istream& is)
{
	assert_token(is, "WIRE");
	
	GateEdge res;
	
	if(!(is 
		>> res.source_node >> res.source_gate
		>> res.target_node >> res.target_gate))
	{
		throw "Invalid wire";
	}
	
	return res;
}

//Gate nodes
void GateNode::save(ostream& os) const
{
	os << "GATE " << name << ' ' << params.size();
	for(int i=0; i<(int)params.size(); i++)
		os << ' ' << params[i];
	os << endl << wires.size() << endl;
	for(int i=0; i<(int)wires.size(); i++)
		wires[i].save(os);
}

GateNode GateNode::load(istream& is)
{
	assert_token(is, "GATE");
	
	GateNode res;
	int num_params;
	if(!(is >> res.name >> num_params))
	{	throw "Error reading gate description";
	}
	
	res.params.resize(num_params);
	for(int i=0; i<num_params; i++)
	{	if(!(is >> res.params[i]))
		{	throw "Error reading gate parameter";
		}
	}
	
	int num_wires;
	if(!(is >> num_wires))
		throw "Error reading wire count";
	res.wires.resize(num_wires);
	for(int i=0; i<num_wires; i++)
		res.wires[i] = GateEdge::load(is);

	return res;
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
	os << endl << gates.size() << endl;
	for(int i=0; i<(int)gates.size(); i++)
		gates[i].save(os);
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
	
	//Read in the gate descriptors
	int num_gates;
	if(!(is >> num_gates))
	{
		throw "Expected gate count";
	}
	res.gates.resize(num_gates);
	for(int i=0; i<num_gates; i++)
	{
		res.gates[i] = GateNode::load(is);
	}
	
	return res;
}

//Edge saving
void Edge::save(ostream& os) const
{
	os << "EDGE " 
		<< source  << ' ' << target  << ' '
		<< rot.w   << ' ' << rot.x   << ' ' << rot.y   << ' ' << rot.z << ' '
		<< scale  << ' '
		<< reflect << ' '

		<< s_point.x  << ' ' << s_point.y  << ' ' << s_point.z  << ' '
		<< t_point.x  << ' ' << t_point.y  << ' ' << t_point.z  << ' '

		<< s_axis.x  << ' ' << s_axis.y  << ' ' << s_axis.z  << ' '
		<< t_axis.x  << ' ' << t_axis.y  << ' ' << t_axis.z  << ' '

		<< s_norm.x  << ' ' << s_norm.y  << ' ' << s_norm.z  << ' '
		<< t_norm.x  << ' ' << t_norm.y  << ' ' << t_norm.z  << ' '

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
		>> res.scale
		>> res.reflect

		>> res.s_point.x  >> res.s_point.y  >> res.s_point.z
		>> res.t_point.x  >> res.t_point.y  >> res.t_point.z

		>> res.s_axis.x  >> res.s_axis.y  >> res.s_axis.z
		>> res.t_axis.x  >> res.t_axis.y  >> res.t_axis.z
		
		>> res.s_norm.x  >> res.s_norm.y  >> res.s_norm.z
		>> res.t_norm.x  >> res.t_norm.y  >> res.t_norm.z
		
		)) throw "Invalid edge";
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
	
	//Fix up edges
	for(int i=0; i<(int)res.edges.size(); i++)
	{
		for(int j=0; j<(int)res.edges[i].size(); j++)
			res.edges[i][j].normalize(res);
	}
	
	return res;
}


//Generates a body part
BodyPart* genCreatureRec(
	Genotype&			genes,
	Creature*			creature,
	int 				n,
	NxMat34				pose,
	float				scale,
	float				reflect)
{
	Node& node = genes.nodes[n];

	//Generate the part at this node
	BodyPart* res;
	switch(node.shape)
	{
		case BODY_BOX:
			res = new BodyPart(creature, node.color, pose, node.size * scale);
		break;
	
		case BODY_SPHERE:
			//TODO: Not yet implemented
			assert(false);
		break;
	
		case BODY_CAPSULE:
			//TODO: Not yet implemented
			assert(false);
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
		
		//Compute translation from attachment points
		NxVec3 s_point = edge.s_point * scale, 
			   t_point = edge.rot.rot(edge.t_point * scale * edge.scale);
		
		//Transform frame
		NxMat34 xform;
		xform.t = t_point - s_point;
		xform.M = NxMat33(edge.rot);
		NxMat34 npose = xform * pose;
		
		//Generate new part
		BodyPart* tmp = genCreatureRec(
			genes, 
			creature, 
			edge.target, 
			npose, 
			scale * edge.scale,
			reflect * edge.reflect);
		
		//If failed, then skip it
		if(tmp == NULL)
		{
			edge.marked = false;
			continue;
		}

		//Create joint and link parts
		NxRevoluteJointDesc joint_desc;
		joint_desc.setToDefault();
		
		joint_desc.actor[0] = 			res->actor;
		joint_desc.localAnchor[0] =		edge.s_point * scale;
		joint_desc.localAxis[0] =		edge.s_axis;
		joint_desc.localNormal[0] = 	edge.s_norm;
		
		joint_desc.actor[1] = 			tmp->actor;
		joint_desc.localAnchor[1] =		edge.t_point * scale * edge.scale;
		joint_desc.localAxis[1] =		edge.t_axis;
		joint_desc.localNormal[1] = 	edge.t_norm;
		
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
	BodyPart* b = genCreatureRec(*this, res, root, pose, 1., 1.);
	
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

NxVec3 Node::closest_pt(const NxVec3& p)
{
	NxVec3 res = p;

	switch(shape)
	{
		case BODY_BOX:
			for(int i=0; i<3; i++)
				res[i] = max(min(p[i], size[i]), -size[i]);
			for(int i=0; i<3; i++)
			{
				if(abs(res[i]) >= size[i] &&
					abs(res[i]) > size[(i+1)%3] &&
					abs(res[i]) > size[(i+2)%3])
				{
					if(res[i] < 0)
						res[i] = -size[i];
					else
						res[i] = size[i];
				}
			}
			
			return res;
		break;
		
		case BODY_SPHERE:
			//TODO: Implement sphere code
			assert(false);
		break;
		
		case BODY_CAPSULE:
			//TODO: Implement capsule code
			assert(false);
		break;
		
		default:
			assert(false);
		break;
	}
	return p;
}

//Normalizes an edge
void Edge::normalize(Genotype& gen)
{
	//Fix up the quaternion
	rot.normalize();

	//Fix points to surface of bodies
	s_point = gen.nodes[source].closest_pt(s_point);
	t_point = gen.nodes[source].closest_pt(t_point);

	//Renormalize edge frame
	s_axis /= s_axis.magnitude();
	t_axis /= t_axis.magnitude();
	
	s_norm -= s_axis * s_axis.dot(s_norm) / s_norm.magnitude();
	s_norm /= s_norm.magnitude();
	
	t_norm -= t_axis * t_axis.dot(t_norm) / t_norm.magnitude();
	t_norm /= t_norm.magnitude();
	
	//Fix up reflection
	if(reflect < 0)	reflect = -1;
	else			reflect =  1;
}


};
