#pragma once

typedef bool (*TestFunction)(int argc, char** argv);


#define EXTERN(X) extern bool X(int,char**)
#define TEST(X) {X, #X}


// externs here

EXTERN(VerifySigmoid);
EXTERN(VerifyLn1PlusEx);
EXTERN(TrainRBM);
EXTERN(TrainAutoEncoder);
// function list

struct
{
	TestFunction Func;
	const char* Name;
} TestList[] =
{
	TEST(VerifySigmoid),
	TEST(VerifyLn1PlusEx),
	TEST(TrainRBM),
	TEST(TrainAutoEncoder),
};