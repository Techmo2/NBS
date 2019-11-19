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

int line = 0;

const char *getErrorStr(cl_int error)
{
switch(error){
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
}

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
	bBodies = new cl::Buffer(*context, CL_MEM_READ_WRITE,
			capacity * sizeof(body_t));
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
	cl_int err;
	line = __LINE__;
	err = queue->enqueueWriteBuffer(*bBodies, CL_TRUE, 0, capacity * sizeof(body_t),
			&bodies[0]);
	if(err != CL_SUCCESS)
			    std::cerr << getErrorStr(err) << std::endl;
}

int Engine::getDataFromDevice() {
	queue->enqueueReadBuffer(*bBodies, CL_TRUE, 0, capacity * sizeof(body_t),
			&bodies[0]);
}

int Engine::simulate(int steps) {
	fKernel->setArg(0, (*bBodies)());
	fKernel->setArg(1, G);
	int s = bodies.size();
	fKernel->setArg(2, s);

	pKernel->setArg(0, (*bBodies)());
	pKernel->setArg(1, dt);
	pKernel->setArg(2, s);

	cl::NDRange global(capacity);
	cl::NDRange local(1);

	for (int step = 0; step < steps; step++) {
		queue->enqueueNDRangeKernel(*fKernel, cl::NullRange, global, local);
		queue->enqueueNDRangeKernel(*pKernel, cl::NullRange, global, local);
	}
}

Engine::~Engine() {

}

