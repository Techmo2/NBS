/*
 * Enigne.h
 *
 *  Created on: Nov 18, 2019
 *      Author: bradly
 */

#ifndef ENIGNE_H_
#define ENIGNE_H_

//#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <vector>
#include "body.h"

class Engine {
private:
	std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;
    cl::Context* context;
    cl::CommandQueue* queue;
    cl::Program* fProgram;
    cl::Program* pProgram;
    cl::Kernel* fKernel;
    cl::Kernel* pKernel;

    cl::Buffer* bBodies;

    std::vector<body_t> bodies;
    float G;
    float dt;
    int capacity;

public:
	Engine(float _G, float _dt, int _capacity);

	void addBody(body_t b);
	body_t getBody(int idx);
	body_t* getBodyPtr(int idx);

	int getSize();
	int getCapacity();

	int sendDataToDevice();
	int getDataFromDevice();

	int simulate(int steps);

	virtual ~Engine();
};

#endif /* ENIGNE_H_ */
