#ifndef solver_h
#define solver_h

#include "util/OutputHelper.h"
#include "util/TokenArray.h"

std::string stringmaker(util::TokenArray &data, bool html);
void solver(util::OutputHelper &oh, util::TokenArray &input);

#endif /* solver_h */
