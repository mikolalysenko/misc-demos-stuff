#ifndef MUTATIONS_H
#define MUTATIONS_H

#include "project/genotype.h"

namespace Game
{

void mutate(Genotype& genes);
Genotype crossover(Genotype& a, Genotype& b);
Genotype graft(Genotype& a, Genotype& b);


};

#endif

