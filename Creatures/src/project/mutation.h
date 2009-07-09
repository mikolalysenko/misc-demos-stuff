#ifndef MUTATIONS_H
#define MUTATIONS_H

#include "project/genotype.h"

namespace Game
{

Genotype randomCreature(int N = 3, int E=4, int G=8, int W=12);
void mutate(Genotype& genes);
Genotype crossover(Genotype& a, Genotype& b);
Genotype graft(Genotype& a, Genotype& b);


};

#endif

