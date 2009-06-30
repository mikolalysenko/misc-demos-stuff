#ifndef MUTATIONS_H
#define MUTATIONS_H

#include "project/genotype.h"

namespace Game
{

Genotype randomCreature();
void mutate(Genotype& genes);
Genotype crossover(Genotype& a, Genotype& b);
Genotype graft(Genotype& a, Genotype& b);


};

#endif

