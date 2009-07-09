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
	//Used for connecting gates
	Wire() : val(0.) {}

	//Channel operators
	float read()
	{
		return val;
	}
	
	void write(float f)
	{
		val = f;
	}

private:
	float val;
};
	
//A control gate
struct Gate
{
	virtual ~Gate() {}
	
	//Updates the logic gate
	virtual void update() = 0;
	
	float read(int x)
	{
		if(x < 0 || x >= (int)inputs.size())
			return 0.;
		return inputs[x]->read();
	}
	
	void write(int x, float v)
	{
		if(x >= 0 && x < (int)outputs.size())
			outputs[x]->write(v);
	}
	
	//Outputs/inputs to the gates
	std::vector<Wire*>		inputs;
	std::vector<Wire*>		outputs;
};

//The gate factory creates gates
struct GateFactory
{
	virtual ~GateFactory() {}

	//Returns the number of parameters required to construct a gate
	virtual int numParams() = 0;
	
	//Generates a random vector of parameters
	virtual std::vector<float> generateParams() = 0;
	
	virtual void perturbParams(std::vector<float>& params);
	
	//Normalizes the parameter vector
	virtual void normalize(std::vector<float>& params) = 0;
	
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params) = 0;
	
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
extern std::string randomGateName();




};

#endif

