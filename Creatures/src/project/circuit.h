#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <map>
#include <vector>
#include <string>

namespace Game
{

//A wire connecting two gates
struct Wire
{
	
};
	
//A control gate
struct Gate
{
	virtual ~Gate();
	
	//Updates the logic gate
	virtual void update();
	
	//Outputs/inputs to the gates
	std::vector<Wire*>		inputs;
	std::vector<Wire*>		outputs;
};

//The gate factory creates gates
struct GateFactory
{
	virtual ~GateFactory();

	//Returns the number of parameters required to construct a gate
	virtual int numParams();
	
	//Generates a random vector of parameters
	virtual std::vector<float> generateParams();
	
	//Normalizes the parameter vector
	virtual void normalize(std::vector<float> params);
	
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params);
	
};

//A circuit is a network of gates linked by wires
struct Circuit
{
	//Clean up
	~Circuit();

	//The gates in the circuit
	std::vector<Gate*> gates;
	
	//The wires in the circuit
	std::vector<Wire*> wires;

	//Updates the circuit
	void update();
};


extern void registerGateFactory(const std::string& name, GateFactory* factory);
extern GateFactory* getFactory(const std::string& name);




};

#endif

