#pragma once

#ifndef SUBSTRATE_H_
#include "substrate.h"
#endif

#define hookSymbol(symbol, hook) ((void) MSHookFunction(dlsym(RTLD_DEFAULT, symbol), (void*) &hook, (void**) &hook##_real))
// #define hookReal(real, hook) ((void) MSHookFunction((void*) &real, (void*) &hook, (void**) &hook##_real)) // TBD