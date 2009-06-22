#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>
#include <utility>
#include <cmath>

#include "project/circuit.h"

using namespace std;


namespace Game
{

//An add gate
struct AddGate : public Gate
{
	virtual ~AddGate() { }
	
	virtual void update()
	{
		float s = 0.;
		for(int i=0; i<(int)inputs.size(); i++)
		{
			s += inputs[i]->read();
		}
		for(int j=0; j<(int)outputs.size(); j++)
		{
			outputs[j]->write(s);
		}
	}
};

struct AddGateFactory : public GateFactory
{
	virtual ~AddGateFactory() {}

	//Returns the number of parameters required to construct a gate
	virtual int numParams()
	{
		return 0;
	}
	
	//Generates a random vector of parameters
	virtual std::vector<float> generateParams()
	{
		return vector<float>(0);
	}
	
	//Normalizes the parameter vector
	virtual void normalize(std::vector<float>& params)
	{
		params.resize(0);
	}
	
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params)
	{
		return new AddGate();
	}
};

//A multiplier gate
struct MultiplyGate : public Gate
{
	virtual ~MultiplyGate() {}
	
	virtual void update()
	{
		float s = 1.;
		for(int i=0; i<(int)inputs.size(); i++)
		{
			s *= inputs[i]->read();
		}
		for(int j=0; j<(int)outputs.size(); j++)
		{
			outputs[j]->write(s);
		}
	}
};
struct MultiplyGateFactory : public AddGateFactory
{
	virtual ~MultiplyGateFactory() {}
	
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params)
	{
		return new MultiplyGate();
	}
};

//Multiplexor
struct MultiplexGate : public Gate
{
	int count;
	
	MultiplexGate() : count(0) {}

	virtual ~MultiplexGate() {}
	
	virtual void update()
	{
		float s = 0.;
		
		if(inputs.size() > 0)
		{
			count = (count + 1) % inputs.size();
			s = inputs[count]->read();
		}
		for(int j=0; j<(int)outputs.size(); j++)
		{
			outputs[j]->write(s);
		}
	}
};
struct MultiplexGateFactory : public AddGateFactory
{
	virtual ~MultiplexGateFactory() {}
	
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params)
	{
		return new MultiplexGate();
	}
};

//Timer gate
struct TimerGate : public Gate
{
	float time, incr, reset;
	
	TimerGate(float time_, float incr_, float reset_) :
		time(time_), incr(incr_), reset(reset_) {}
	
	virtual ~TimerGate() {}
	
	virtual void update()
	{
		time += incr;
		while(time >= reset)
			time -= reset;
		
		for(int i=0; i<(int)outputs.size(); i++)
		{
			outputs[i]->write(time);
		}
	}
};
struct TimerFactory : public GateFactory
{
	//Returns the number of parameters required to construct a gate
	virtual int numParams()
	{
		return 3;
	}
	
	//Generates a random vector of parameters
	virtual std::vector<float> generateParams()
	{
		vector<float> res(3);
		
		res[2] = drand48() * 1e8;
		res[1] = res[2] / drand48() * 1e-8;
		res[0] = res[2] * drand48();
		
		return res;
	}
	
	//Normalizes the parameter vector
	virtual void normalize(std::vector<float>& params)
	{
		params.resize(3);
		
		params[2] = max(params[2], 1.e-6f);
		params[1] = max(min(params[1], params[2]), 0.f);
		params[0] = max(min(params[0], params[2]), 0.f);
	}
	
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params)
	{
		assert(params.size() == 3);
		return new TimerGate(params[0], params[1], params[2]);
	}

};

//A constant signal
struct ConstantGate : public Gate
{
	float val;
	
	ConstantGate(float val_) : val(val_) {}
	virtual ~ConstantGate() {}
	
	virtual void update()
	{
		for(int i=0; i<(int)outputs.size(); i++)
			outputs[i]->write(val);
	}
};
struct ConstantGateFactory : public GateFactory
{
	//Returns the number of parameters required to construct a gate
	virtual int numParams()
	{
		return 1;
	}
	
	//Generates a random vector of parameters
	virtual std::vector<float> generateParams()
	{
		vector<float> res(1);
		
		res[0] = (drand48() - .5) * 1e8;
		
		return res;
	}
	
	//Normalizes the parameter vector
	virtual void normalize(std::vector<float>& params)
	{
		params.resize(1);
	}
	
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params)
	{
		assert(params.size() == 1);
		return new ConstantGate(params[0]);
	}

};


//GateFactory registration
static map<string, GateFactory*> factories;
static vector<string>			 gate_names;
void registerGateFactory(const std::string& name, GateFactory* factory)
{
	factories[name] = factory;
	gate_names.push_back(name);
}

GateFactory* getFactory(const std::string& name)
{
	assert(factories.find(name) != factories.end());
	return factories[name];
}

string randomGateName()
{
	return gate_names[rand() % gate_names.size()];
}


//Initializes each of the gates
struct GateInitializer
{
GateInitializer()
{
	registerGateFactory("add", new AddGateFactory());
	registerGateFactory("mul", new MultiplyGateFactory());
	registerGateFactory("multiplex", new MultiplexGateFactory());
	registerGateFactory("timer", new MultiplexGateFactory());
	registerGateFactory("const", new ConstantGateFactory());
}
};
GateInitializer init_gates;

};
