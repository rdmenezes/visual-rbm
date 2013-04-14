#pragma once

#include <vector>
#include <sstream>
#include <stdint.h>
#include <intrin.h>

#define COMPUTE_ASSERT(X) if(!(X)) {printf("Assert Failed: %s line %i on \"%s\"\n", __FILE__, __LINE__, #X);  __debugbreak();}

#include "Enums.h"
#include "AST.h"
#include "Source.h"
#include "Compiler.h"
#include "Program.h"

// Backends
#include "Backends/OpenGL.h"

