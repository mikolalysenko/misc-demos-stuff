#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <utility>
#include <cmath>
#include <locale>

#include "common/sys_includes.h"
#include "common/physics.h"

#include "project/creature.h"
#include "project/genotype.h"

using namespace std;
using namespace Common;

namespace Game
{


void print_vec(const NxVec3& v)
{
	cout << v.x << ',' <<v.y << ',' << v.z << endl;
}
void print_mat(const NxMat33& m)
{
	for(int i=0; i<3; i++)
	{
	for(int j=0; j<3; j++)
	{
		cout << m(i,j) << ',';
	}
		cout << endl;
	}
}




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
	os << "WIRE " << node_type << ' ' << node << ' '
				  << gate_type << ' ' << gate << ' ' 
				  << direction << endl;
}

GateEdge GateEdge::load(istream& is)
{
	assert_token(is, "WIRE");
	
	GateEdge res;
	int nt, gt;
	
	if(!(is 
		>> nt >> res.node
		>> gt >> res.gate
		>> res.direction))
	{
		throw "Invalid wire";
	}
	
	res.node_type = (NodeType)nt;
	res.gate_type = (GateType)gt;
	
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

		<< strength << ' ' << stiffness << ' '
		
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

		>> res.strength >> res.stiffness
		
		)) throw "Invalid edge";
	return res;
}


//Save the phenotype
void Genotype::save(ostream& os) const
{
	os	<< "GENOTYPE" << endl
		<< root << ' ' << nodes.size() << endl;
	
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
	
	//Do clean up
	res.normalize();
	
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
				if(abs(res[i]) > abs(res[(i+1)%3]) &&
					abs(res[i]) > abs(res[(i+2)%3]))
				{
					if(res[i] < 0)
						res[i] = -size[i];
					else
						res[i] = size[i];
					break;
				}
			}
			
			return res;
		break;
		
		case BODY_SPHERE:
			if(res.magnitude() <= 1e-6)
				res = NxVec3(1,0,0);
			res *= radius / res.magnitude();
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

void GateEdge::normalize(
	Genotype& genes,
	int n,
	int g)
{
	//Normalize enum types
	if(node_type != NODE_CURRENT &&
		node_type != NODE_CHILD)
	{
		node_type = NODE_CURRENT;
	}
	
	node = node % genes.nodes.size();
	int t = node_type == NODE_CURRENT ? n : node;

	//Validate gate type
	if(!(gate_type == GATE_SENSOR ||
		gate_type == GATE_EFFECTOR ||
		gate_type == GATE_CONTROL) ||
		genes.edges[t].size() == 0)
	{
		gate_type = GATE_CONTROL;
	}
	
	//Handle case where there might not be any control gates
	if(gate_type == GATE_CONTROL &&
		genes.nodes[t].gates.size() == 0)
	{
		node_type = NODE_CURRENT;
		t = node = node % genes.nodes.size();
	}

	
	switch(gate_type)
	{
		case GATE_SENSOR:
			//TODO: Mod gate selector by number of sensors
			//gate %= genes.edges[t].size();
			gate = 0;
		break;
		
		case GATE_EFFECTOR:
			//TODO: Mod gate selector by number of effectors
			//gate %= genes.edges[t].size();
			gate = 0;
		break;
		
		case GATE_CONTROL:
			if(genes.nodes[t].gates.size() > 0)
				gate %= genes.nodes[t].gates.size();
			else
				gate = 0;
		break;
		
		default: assert(false);
	}
}


void GateNode::normalize()
{
	//Force name to lower case
	for(int i=0; i<(int)name.size(); i++)
		name[i] = tolower(name[i]);

	//Get factory
	GateFactory* f = getFactory(name);
	
	//No factory, make up some random name
	if(f == NULL)
	{
		name = randomGateName();
		f = getFactory(name);
	}

	f->normalize(params);
}

void Node::normalize()
{
	//Clamp color range to [0,1]
	for(int i=0; i<3; i++)
		color[i] = max(min(color[i], 1.f), 0.f);
		
	//Kill shape
	shape = BODY_BOX;


	for(int i=0; i<3; i++)
		size[i] = max(size[i], 1.f);

	radius = max(radius, 1.f);
	length = max(length, 1.f);
}

NxVec3 max_dim(NxVec3 sz, NxVec3 v)
{
	NxVec3 res = NxVec3(0,0,0);
	for(int i=0; i<3; i++)
	{
		if(abs(abs(v[i]) - sz[i]) <= 1e-4)
		{
			res[i] = v[i] > 0 ? 1 : -1;
		}
	}
	if(res.magnitude() < 1e-4)
		return NxVec3(0, 0, 1);
	
	res.normalize();
	return res;
}


//Normalizes an edge
void Edge::normalize(Genotype& genes)
{
	marked = false;

	//Fix up the quaternion
	rot.normalize();
	
	if(stiffness < 10.)
		stiffness = 10.;
	if(strength < 0.)
		strength = 0.;
	if(strength > 1e6)
		strength = 1e6;

	//Fix points to surface of bodies
	s_point = genes.nodes[source].closest_pt(s_point);
	t_point = genes.nodes[target].closest_pt(t_point);

	//Renormalize edge frame
	s_axis /= s_axis.magnitude();
	t_axis /= t_axis.magnitude();


	//Adjust s_axis so that it is at least 30 deg away from surface
	NxVec3 s_face = max_dim(genes.nodes[source].size, s_point), 
			t_face = max_dim(genes.nodes[target].size, t_point);
	s_axis = s_face;
	t_axis = t_face;
	
	s_norm.normalize();
	s_norm -= s_axis * s_axis.dot(s_norm);
	s_norm.normalize();
	
	t_norm.normalize();
	t_norm -= t_axis * t_axis.dot(t_norm);
	t_norm.normalize();
	
	//Adjust rotation based on the s-axis, t-axis parameters

	
	
	//Fix up reflection
	if(reflect < 0)	reflect = -1;
	else			reflect =  1;
	
	//Fix up possible bounds issues
	target %= genes.nodes.size();
}


//Normalizes all parameters to be within acceptable bounds.  This is necessary
//because stuff might get fucked up due to mutation or dumb user input.
void Genotype::normalize()
{
	for(int i=0; i<(int)nodes.size(); i++)
	{
		nodes[i].normalize();
		for(int j=0; j<(int)nodes[i].gates.size(); j++)
		{
			nodes[i].gates[j].normalize();
			
			for(int k=0; k<(int)nodes[i].gates[j].wires.size(); k++)
			{
				nodes[i].gates[j].wires[k].normalize(*this, i, j);
			}
		}
	}
	
	//Clean up edges
	for(int i=0; i<(int)edges.size(); i++)
	for(int j=0; j<(int)edges[i].size(); j++)
	{
		edges[i][j].source = i;
		edges[i][j].normalize(*this);
	}
}

//Graph modification stuff
void Genotype::remove_node(int n)
{
	//Remove object
	int T = nodes.size() - 1;
	edges[n] = edges[T];
	nodes[n] = nodes[T];
	edges.resize(T);
	nodes.resize(T);
	
	if(root == T)
		root = n;
	
	//Update edges	
	for(int i=0; i<(int)edges.size(); i++)
	for(int j=0; j<(int)edges[i].size(); j++)
	{
		if(edges[i][j].target == n)
		{
			remove_edge(i, j);
			j--;
		}
		else if(edges[i][j].target == T)
		{
			edges[i][j].target = n;
		}
	}
}

void Genotype::remove_edge(int s, int x)
{
	int T = edges[s].size() - 1;
	
	edges[s][x] = edges[s][T];
	edges[s].resize(T);
	
	//Need to update gates somehow
	for(int i=0; i<(int)nodes[s].gates.size(); i++)
	{
		GateNode& g = nodes[s].gates[i];
		for(int j=0; j<(int)g.wires.size(); j++)
		{
			GateEdge w = g.wires[j];
			if(w.node_type != NODE_CHILD || edges[s].size() == 0)
				continue;
				
				
			if(w.node % (int)edges[s].size() == x)
			{
				remove_wire(s, i, j);
				j--;
			}
			else if(w.node % (int)edges[s].size() == T)
			{
				w.node = x;
			}
		}
	}
}

void Genotype::remove_gate(int n, int g)
{
	Node& N = nodes[n];

	int T = N.gates.size()-1;
	N.gates[g] = N.gates[T];
	N.gates.resize(T);
	
	for(int s=0; s<(int)nodes.size(); s++)
	{
		Node& nn = nodes[s];
		for(int i=0; i<(int)nn.gates.size(); i++)
		{
			GateNode& gg = nn.gates[i];
			for(int j=0; j<(int)gg.wires.size(); j++)
			{
				GateEdge& w = gg.wires[j];
				if(	w.node_type == NODE_CURRENT ||
					w.gate_type != GATE_CONTROL ||
					edges[s].size() == 0 ||
					edges[s][w.node % edges[s].size()].target != n )
					continue;
				
				if(w.gate == g)
				{
					remove_wire(s, i, j);
					j--;
				}
				else if(w.gate == T)
				{
					w.gate = T;
				}
			}
		}
	}
}

void Genotype::remove_wire(int n, int g, int w)
{
	GateNode& gg = nodes[n].gates[g];
	gg.wires[w] = gg.wires[gg.wires.size()-1];
	gg.wires.resize(gg.wires.size()-1);
}

//Generates a body part
BodyPart* genCreatureRec(
	Genotype&			genes,
	Creature*			creature,
	int					n,
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
			res = new BodyPart(creature, node.color, pose, node.radius * scale);
		break;
	
		case BODY_CAPSULE:
			//TODO: Not yet implemented
			assert(false);
		break;
		
		default: assert(false);
	}

	//If fails, then return NULL
	if(res->actor == NULL)
	{
		delete res;
		return NULL;
	}

	//Add body part to body vector
	creature->body.push_back(res);
	
	//Generate gates
	for(int i=0; i<(int)node.gates.size(); i++)
	{
		GateFactory* gf = getFactory(node.gates[i].name);	
		res->controls.push_back(gf->createGate(node.gates[i].params));
	}
	
	//For each edge:
	for(int i=0; i<(int)genes.edges[n].size(); i++)
	{
		Edge& edge = genes.edges[n][i];
		
		//Check for mark
		if(edge.marked)
			continue;
		edge.marked = true;
		
	
		NxMat33 R = edge.rot;
						
		//Compute translation from attachment points
		NxVec3 s_point = edge.s_point * (scale + 0.01), 
			   t_point = edge.t_point * (scale * edge.scale + 0.01);

		
		//Transform frame
		NxMat33 rot = pose.M * R, rotInv;
		rot.getInverse(rotInv);
		
		NxMat34 npose;
		npose.M = rot;
		npose.t = pose * s_point - rot * t_point;;
		
		//Generate new part
		int c_partlen = creature->body.size();
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
			cout << "Failed to create body part" << endl;
		
			edge.marked = false;
			
			for(int i=c_partlen; i<(int)creature->body.size(); i++)
				delete creature->body[i];			
			creature->body.resize(c_partlen);
			continue;
		}

		//Create joint and link parts
		NxD6JointDesc joint_desc;
		joint_desc.setToDefault();
		
		joint_desc.maxForce =			1e8;
		joint_desc.maxTorque =			1e8;
		
		joint_desc.xMotion = joint_desc.yMotion = joint_desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
		
		joint_desc.swing1Motion = joint_desc.swing2Motion = joint_desc.twistMotion = NX_D6JOINT_MOTION_FREE;
		
		NxJointDriveDesc drive_limits;
		
		drive_limits.damping = edge.stiffness/20.;
		drive_limits.spring = edge.stiffness;
		drive_limits.forceLimit = edge.strength;
		drive_limits.driveType = NX_D6JOINT_DRIVE_VELOCITY;
		joint_desc.slerpDrive = drive_limits;
		
		joint_desc.actor[0] = 			res->actor;
		joint_desc.localAnchor[0] =		s_point;
		joint_desc.localAxis[0] =		edge.s_axis;
		joint_desc.localNormal[0] = 	edge.s_norm;
		
		joint_desc.actor[1] = 			tmp->actor;
		joint_desc.localAnchor[1] =		t_point;
		joint_desc.localAxis[1] =		edge.t_axis;
		joint_desc.localNormal[1] = 	edge.t_norm;
		
	    NxJoint * joint = scene->createJoint(joint_desc);
	    
	    /*
	    
	    cout << "s_size = ";
	    print_vec(scale * node.size);
	    cout << "s = ";
		print_vec(s_point);
		cout << "A = ";
		print_mat(A);
		
		cout << "t_size = ";
		print_vec(scale * edge.scale * genes.nodes[edge.target].size);
		cout << "t = ";
		print_vec(t_point);
		cout << "B = ";
		print_mat(B);
		
		cout << "R = ";
		print_mat(R);
		
		print_mat(R * B);
		*/
		
	    
	    if(joint == NULL)
	    {
	    	cout << "Failed to create joint!" << endl;
	    	delete tmp;
	    	edge.marked = false;
	    	
			for(int i=c_partlen+1; i<(int)creature->body.size(); i++)
				delete creature->body[i];			
			creature->body.resize(c_partlen);
			
	    	continue;
	    }
	   	else
	   	{
	    	res->attachPart(tmp, joint, 10.f);	//TODO: Adjust strength calculation here
		}
		
		//Unmark used edge
	 	edge.marked = false;
	}
	
	//Rig up wires
	for(int i=0; i<(int)node.gates.size(); i++)
	{
		for(int j=0; j<(int)node.gates[i].wires.size(); j++)
		{
			GateEdge ge = node.gates[i].wires[j];
			
			//Hard part: need to find target gate
			BodyPart* container;
			switch(ge.node_type)
			{
				case NODE_CURRENT:
					container = res;
				break;
				case NODE_CHILD:
				
					if(res->limbs.size() > 0)
						container = res->limbs[ge.node % res->limbs.size()];
					else
						container = res;
				break;
				
				default: assert(false);
			}
			
			Gate * a = res->controls[i];
			Gate* b = a;
			switch(ge.gate_type)
			{
				case GATE_SENSOR:
				if(container->sensors.size() > 0)
				{	b = container->sensors[ge.gate % container->sensors.size()];
					break;
				}
				case GATE_EFFECTOR:
				if(container->effectors.size() > 0)
				{	b = container->effectors[ge.gate % container->effectors.size()];
					break;
				}

				case GATE_CONTROL:
				if(container->controls.size() > 0)
					b = container->controls[ge.gate % container->controls.size()];
				break;
				
				default: assert(false);
			}
			
			//Get default gate
			if(ge.direction > 0)
				swap(a, b);

			//Add wire
			Wire * wire = new Wire();
			res->wires.push_back(wire);
			
			//Connect gates
			a->outputs.push_back(wire);
			b->inputs.push_back(wire);
		}
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
		cout << "Creature build fail!" << endl;
		delete res;
		return NULL;
	}
	
	//Attach root and done
	res->root = b;
	return res;
}

};
