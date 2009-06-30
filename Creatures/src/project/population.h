#ifndef POPULATION_H
#define POPULATION_H

#include <iostream>
#include <string>

#include "project/creature.h"
#include "project/genotype.h"

namespace Game
{

//Interface for a fitness test
struct FitnessTest
{
	Creature* creature;

	virtual void start_test(Genotype* genes);
	virtual void stop_test();	
	virtual void update();
	virtual void draw();
	virtual float get_fitness();
};

//A population of creatures
struct Population
{
	vector< pair<float,Genotype*> >	species;
	vector< pair<float,Genotype*> >	best_species;
	
	FitnessTest*	tester;
	float 			max_round_time;
	
	//Constructors
	Population() {}
	Population(
		int num_creatures,
		int num_high_scores,
		float round_time,
		FitnessTest* test);
	
	//Updates the population
	void update();
	void draw();

	void save(ostream& os);
	static void load(istream& is);
	
private:
	float	round_time;
	int 	current_test;

	///Generates a new creature
	void next_round();
};

};

#endif

