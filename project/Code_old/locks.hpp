#pragma once
#include <algorithm> // for the std::max() function
#include <omp.h>
#include <atomic> // for the reference lock

// This is actually not how you're supposed to use source-files...but whatever
// it works so its fine for us :D
#include "toolbox.cpp"
#include "locks.cpp"
#include "lamport.cpp"
#include "taubenfeld.cpp"