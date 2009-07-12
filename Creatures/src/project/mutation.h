#ifndef MUTATIONS_H
#define MUTATIONS_H

#include "project/genotype.h"

namespace Game
{

Genotype randomCreature(int N = 10, int E=4, int G=10, int W=50);
void mutate(Genotype& genes);
Genotype crossover(Genotype& a, Genotype& b);
Genotype graft(Genotype& a, Genotype& b);


};

#endif

