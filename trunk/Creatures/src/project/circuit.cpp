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

void GateFactory::perturbParams(std::vector<float>& params)
{
	for(int i=0; i<(int)params.size(); i++)
	{
		params[i] += 20. * (drand48() + drand48() + drand48() + drand48()  - 2.); 
	}
}



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
		
		res[0] = (drand48() + drand48() + drand48() + drand48() - 2.) * 1e3;
		
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

//A constant signal
struct SineGate : public Gate
{
	float freq, ampl, phase;
	
	SineGate(float freq_, float amp_, float phase_) : freq(freq_), ampl(amp_), phase(phase_) {}
	virtual ~SineGate() {}
	
	virtual void update()
	{
		for(int i=0; i<(int)inputs.size(); i++)
			write(i, ampl * sin(read(i)*freq + phase));
	}
};
struct SineGateFactory : public GateFactory
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
		
		res[0] = (drand48() + drand48() + drand48() + drand48() - 2.) * 10.;
		res[1] = (drand48() + drand48() + drand48() + drand48() - 2.) * 1e3;
		res[2] = drand48() * M_PI;
		
		return res;
	}
	
	//Normalizes the parameter vector
	virtual void normalize(std::vector<float>& params)
	{
		params.resize(3);
	}
	
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params)
	{
		assert(params.size() == 3);
		return new SineGate(params[0], params[1], params[2]);
	}

};


struct ExpGate : public Gate
{
	ExpGate() {}
	virtual ~ExpGate() {}
	
	virtual void update()
	{
		for(int i=0; i<(int)inputs.size(); i++)
			write(i, exp(read(i)));
	}
};
struct ExpGateFactory : public AddGateFactory
{
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params)
	{
		return new ExpGate();
	}
};


struct LogGate : public Gate
{
	LogGate() {}
	virtual ~LogGate() {}
	
	virtual void update()
	{
		for(int i=0; i<(int)inputs.size(); i++)
		{
			float x = read(i);
			if(x <= 1e-6)
			{
				write(i, 0.);
			}
			else
			{
				write(i, log(read(i)));
			}
		}
	}
};
struct LogGateFactory : public AddGateFactory
{
	//Creates a gate
	virtual Gate* createGate(std::vector<float> params)
	{
		return new LogGate();
	}
};


struct RecipGate : public Gate
{
	RecipGate() {}
	virtual ~RecipGate() {}
	virtual void update()
	{
		for(int i=0; i<(int)inputs.size(); i++)
		{
			float x = read(i);
			if(abs(x) <= 1e-6)
			{
				write(i, 0.);
			}
			else
			{
				write(i, 1./read(i));
			}
		}
	}
};
struct RecipGateFactory : public AddGateFactory
{
	virtual Gate* createGate(std::vector<float> params)
	{
		return new RecipGate();
	}
};



struct NegGate : public Gate
{
	NegGate() {}
	virtual ~NegGate() {}
	virtual void update()
	{
		for(int i=0; i<(int)inputs.size(); i++)
			write(i, -read(i));
	}
};
struct NegGateFactory : public AddGateFactory
{
	virtual Gate* createGate(std::vector<float> params)
	{
		return new NegGate();
	}
};


struct TanGate : public Gate
{
	TanGate() {}
	virtual ~TanGate() {}
	virtual void update()
	{
		for(int i=0; i<(int)inputs.size(); i++)
			write(i, tan(read(i)));
	}
};
struct TanGateFactory : public AddGateFactory
{
	virtual Gate* createGate(std::vector<float> params)
	{
		return new TanGate();
	}
};

struct ATanGate : public Gate
{
	ATanGate() {}
	virtual ~ATanGate() {}
	virtual void update()
	{
		for(int i=0; i<(int)inputs.size(); i++)
			write(i, atan(read(i)));
	}
};
struct ATanGateFactory : public AddGateFactory
{
	virtual Gate* createGate(std::vector<float> params)
	{
		return new ATanGate();
	}
};


struct MaxGate : public Gate
{
	MaxGate() {}
	virtual ~MaxGate() {}
	virtual void update()
	{
		float m = -1e20;
		for(int i=0; i<(int)inputs.size(); i++)
			m = max(m, read(i));
		write(0, m);
	}
};
struct MaxGateFactory : public AddGateFactory
{
	virtual Gate* createGate(std::vector<float> params)
	{
		return new MaxGate();
	}
};

struct MinGate : public Gate
{
	MinGate() {}
	virtual ~MinGate() {}
	virtual void update()
	{
		float m = 1e20;
		for(int i=0; i<(int)inputs.size(); i++)
			m = min(m, read(i));
		write(0, m);
	}
};
struct MinGateFactory : public AddGateFactory
{
	virtual Gate* createGate(std::vector<float> params)
	{
		return new MinGate();
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
	registerGateFactory("timer", new TimerGateFactory());
	registerGateFactory("const", new ConstantGateFactory());
	registerGateFactory("sine", new SineGateFactory());
	registerGateFactory("exp", new ExpGateFactory());
	registerGateFactory("log", new LogGateFactory());
	registerGateFactory("recip", new RecipGateFactory());
	registerGateFactory("negate", new NegGateFactory());
	registerGateFactory("tan", new TanGateFactory());
	registerGateFactory("atan", new ATanGateFactory());
	registerGateFactory("max", new MaxGateFactory());
	registerGateFactory("min", new MinGateFactory());
}
};
GateInitializer init_gates;

};
