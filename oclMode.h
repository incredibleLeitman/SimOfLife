#pragma once

#include "common.h"

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define __CL_ENABLE_EXCEPTIONS // to use cl::Error
// adapted from opencltest
// requirements: 
// - runs only for x86 builds
// - CUDA Toolkit 9.0 (sets the environment variable CUDA_PATH)
// - cl.hpp, the C++ bindings for OpenCL v 1.2
#include <CL/cl.hpp>

void initOCL(unsigned int plaformId, unsigned int deviceId)
{
	const std::string KERNEL_FILE = "kernel.cl";
	cl_int err = CL_SUCCESS;
	cl::Program program;
	std::vector<cl::Device> devices;

	try
	{
		// get available platforms ( NVIDIA, Intel, AMD,...)
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0 || platforms.size() < plaformId) throw "specified OpenCL platform not available!\n";

		bool test = true;
		// test output to gather information about installed hardware:
		// SyntaX-Desktop:
		//	platform: NVIDIA CUDA
		//		device: GeForce GTX 1080
		//	platform: Intel(R) OpenCL HD Graphics
		//		device : Intel(R) HD Graphics 530
		if (test)
		{
			for (auto& platform : platforms)
			{
				std::cout << "platform: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";
				cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(), 0 };
				cl::Context context(CL_DEVICE_TYPE_ALL, properties);
				devices = context.getInfo<CL_CONTEXT_DEVICES>();
				for (auto& device : devices)
				{
					std::cout << "device: " << device.getInfo<CL_DEVICE_NAME>() << "\n";
				}
			}
		}

		// create a context and get available devices

		cl::Platform platform = platforms[plaformId];
		cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(), 0 };
		cl::Context context(CL_DEVICE_TYPE_GPU, properties);

		devices = context.getInfo<CL_CONTEXT_DEVICES>();
		if (devices.size() == 0 || devices.size() < deviceId) throw "specified OpenCL device not available!\n";

		// load and build the kernel
		std::ifstream sourceFile(KERNEL_FILE);
		if (!sourceFile) std::cerr << "kernel source file " << KERNEL_FILE << " not found!" << std::endl;

		std::string sourceCode(
			std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		program = cl::Program(context, source);
		program.build(devices);

		//create kernels
		cl::Kernel kernel(program, "hello", &err);
		cl::Event event;

		// launch hello kernel
		std::cout << "calling 'hello' kernel" << std::endl;
		cl::CommandQueue queue(context, devices[deviceId], 0, &err);
		queue.enqueueNDRangeKernel(
			kernel,
			cl::NullRange,
			cl::NDRange(4, 4),
			cl::NullRange,
			NULL,
			&event);
		event.wait();

		// create input and output data
		std::vector<int> a({ 1,2,3,4,5,6,7,8 });
		std::vector<int> b({ 1,2,3,4,5,6,7,8 });
		std::vector<int> c;
		c.resize(a.size());
		// input buffers
		cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY, a.size() * sizeof(int));
		cl::Buffer bufferB = cl::Buffer(context, CL_MEM_READ_ONLY, b.size() * sizeof(int));
		// output buffers
		cl::Buffer bufferC = cl::Buffer(context, CL_MEM_WRITE_ONLY, c.size() * sizeof(int));

		// fill buffers
		queue.enqueueWriteBuffer(
			bufferA, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			a.size() * sizeof(int), // size of write 
			&a[0]); // pointer to input
		queue.enqueueWriteBuffer(bufferB, CL_TRUE, 0, b.size() * sizeof(int), &b[0]);

		cl::Kernel addKernel(program, "vector_add", &err);
		addKernel.setArg(0, bufferA);
		addKernel.setArg(1, bufferB);
		addKernel.setArg(2, bufferC);

		// launch add kernel
		// Run the kernel on specific ND range
		cl::NDRange global(a.size());
		cl::NDRange local(1); //make sure local range is divisible by global range
		cl::NDRange offset(0);
		std::cout << "call 'vector_add' kernel" << std::endl;
		queue.enqueueNDRangeKernel(addKernel, offset, global, local);

		// read back result
		queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, c.size() * sizeof(int), &c[0]);

		for (int i : c)
		{
			std::cout << i << std::endl;
		}
	}
	catch (cl::Error err)
	{
		std::string s;
		program.getBuildInfo(devices[deviceId], CL_PROGRAM_BUILD_LOG, &s);
		std::cout << s << std::endl;
		program.getBuildInfo(devices[deviceId], CL_PROGRAM_BUILD_OPTIONS, &s);
		std::cout << s << std::endl;
		std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
	}
}

void runOCL(const char* fileI, const char* fileO, unsigned int generations, unsigned int platformId, unsigned int deviceId)
{
#ifdef _DEBUG
	std::cout << "DEBUG" << std::endl;
#endif
	std::cout << "running mode: ocl" << std::endl;

	Timing::getInstance()->startSetup();
	// init grid from file
	// TODO: fill datastructure

	initOCL(platformId, deviceId);
	Timing::getInstance()->stopSetup();

	Timing::getInstance()->startComputation();
	// TODO: do stuff
	Timing::getInstance()->stopComputation();
}
