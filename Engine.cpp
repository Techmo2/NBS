/*
 * Enigne.cpp
 *
 *  Created on: Nov 18, 2019
 *      Author: bradly
 */

#include "Engine.h"
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

Engine::Engine(float _G, float _dt, int _capacity) {
	this->G = _G;
	this->dt = _dt;
	this->capacity = _capacity;

	bodies.reserve(capacity);

	// fill bodies with empty values
	for (int i = 0; i < capacity; i++) {
		body_t b;

		b.px = 0;
		b.py = 0;
		b.pz = 0;

		b.fx = 0;
		b.fy = 0;
		b.fz = 0;

		b.vx = 0;
		b.vy = 0;
		b.vz = 0;

		b.mass = 0;
	}

	cl::Platform::get(&platforms);

	// select default platform and create context
	cl_context_properties cps[3] = {
	CL_CONTEXT_PLATFORM, (cl_context_properties) (platforms[0])(), 0 };

	context = new cl::Context(CL_DEVICE_TYPE_GPU, cps);

	devices = context->getInfo<CL_CONTEXT_DEVICES>();

	// create command queue
	queue = new cl::CommandQueue(*context, devices[0]);

	// read cl kernel sources && build programs
	std::ifstream fSourceFile("nbs_forces.cl");
	std::string fSourceCode(std::istreambuf_iterator<char>(fSourceFile),
			(std::istreambuf_iterator<char>()));

	cl::Program::Sources fSource(1,
			std::make_pair(fSourceCode.c_str(), fSourceCode.length() + 1));

	try {
		fProgram = new cl::Program(*context, fSource);
		fProgram->build(devices);
	} catch (cl::Error &e) {
		if (e.err() == CL_BUILD_PROGRAM_FAILURE) {
			for (cl::Device dev : devices) {
				// Check the build status
				cl_build_status status = fProgram->getBuildInfo
						< CL_PROGRAM_BUILD_STATUS > (dev);
				if (status != CL_BUILD_ERROR)
					continue;

				// Get the build log
				std::string name = dev.getInfo<CL_DEVICE_NAME>();
				std::string buildlog = fProgram->getBuildInfo
						< CL_PROGRAM_BUILD_LOG > (dev);
				std::cerr << "Build log for " << name << ":" << std::endl
						<< buildlog << std::endl;
			}

			throw e;
		} else {
			throw e;
		}
	}

	std::ifstream pSourceFile("nbs_positions.cl");
	std::string pSourceCode(std::istreambuf_iterator<char>(pSourceFile),
			(std::istreambuf_iterator<char>()));

	cl::Program::Sources pSource(1,
			std::make_pair(pSourceCode.c_str(), pSourceCode.length() + 1));

	try {
		pProgram = new cl::Program(*context, pSource);
		pProgram->build(devices);
	} catch (cl::Error &e) {
		if (e.err() ==  CL_BUILD_PROGRAM_FAILURE) {
			for (cl::Device dev : devices) {
				// Check the build status
				cl_build_status status = pProgram->getBuildInfo
						< CL_PROGRAM_BUILD_STATUS > (dev);
				if (status != CL_BUILD_ERROR)
					continue;

				// Get the build log
				std::string name = dev.getInfo<CL_DEVICE_NAME>();
				std::string buildlog = pProgram->getBuildInfo
						< CL_PROGRAM_BUILD_LOG > (dev);
				std::cerr << "Build log for " << name << ":" << std::endl
						<< buildlog << std::endl;
			}

			throw e;
		} else {
			throw e;
		}
	}

	// build compute kernels
	fKernel = new cl::Kernel(*fProgram, "execute");
	pKernel = new cl::Kernel(*pProgram, "execute");

	// create memory buffers
	bGravity = new cl::Buffer(*context, CL_MEM_READ_ONLY, sizeof(float));
	bBodies = new cl::Buffer(*context, CL_MEM_READ_WRITE,
			capacity * sizeof(body_t));
	bTimestep = new cl::Buffer(*context, CL_MEM_READ_ONLY, sizeof(float));
	bSize = new cl::Buffer(*context, CL_MEM_READ_ONLY, sizeof(int));

	// Copy the timestep and g to buffers
	queue->enqueueWriteBuffer(*bGravity, CL_TRUE, 0, sizeof(float), &G);
	queue->enqueueWriteBuffer(*bTimestep, CL_TRUE, 0, sizeof(float), &dt);
}

void Engine::addBody(body_t b) {
	if (bodies.size() < capacity)
		bodies.push_back(b);
}

body_t Engine::getBody(int idx) {
	return bodies.at(idx);
}

body_t* Engine::getBodyPtr(int idx) {
	if (idx < bodies.size())
		return &(bodies.at(idx));
	return nullptr;
}

int Engine::getSize() {
	return bodies.size();
}

int Engine::getCapacity() {
	return capacity;
}

int Engine::sendDataToDevice() {
	queue->enqueueWriteBuffer(*bBodies, CL_TRUE, 0, capacity * sizeof(body_t),
			&bodies[0]);
	int s = bodies.size();
	queue->enqueueWriteBuffer(*bSize, CL_TRUE, 0, sizeof(int), &s);
}

int Engine::getDataFromDevice() {
	queue->enqueueReadBuffer(*bBodies, CL_TRUE, 0, capacity * sizeof(body_t),
			&bodies[0]);
}

int Engine::simulate(int steps) {
	fKernel->setArg(0, *bBodies);
	fKernel->setArg(1, *bGravity);
	fKernel->setArg(2, *bSize);

	pKernel->setArg(0, *bBodies);
	pKernel->setArg(1, *bTimestep);
	pKernel->setArg(2, *bSize);

	cl::NDRange global(capacity);
	cl::NDRange local(1);

	for (int step = 0; step < steps; step++) {
		queue->enqueueNDRangeKernel(*fKernel, cl::NullRange, global, local);
		queue->enqueueNDRangeKernel(*pKernel, cl::NullRange, global, local);
	}
}

Engine::~Engine() {

}

