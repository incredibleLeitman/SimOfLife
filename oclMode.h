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

const std::string KERNEL_FILE = "kernel.cl";

cl::Buffer boardBuffer;
cl::Buffer cacheBuffer;
cl::Kernel kernel;
cl::CommandQueue queue;

void oclReadFromFile(const char* filePath)
{
	std::cout << "read file: " << filePath << "..." << std::endl;
	std::ifstream in(filePath);
	if (in.is_open())
	{
		std::string line;
		std::getline(in, line); // get first line for explizit w / h

		// TODO: better split
		size_t pos = line.find(',');
		if (pos != std::string::npos)
		{
			w = std::stoi(line.substr(0, pos));
			h = std::stoi(line.substr(pos + 1));
		}
		else // use default values
		{
			w = 1000;
			h = 10000;
		}

		total_elem_count = w * h;
		col_right = w - 1;
		col_bot = total_elem_count - w;
		row_bot = h - 1;
		std::cout << "total: " << total_elem_count << ", w: " << w << ", h: " << h << std::endl;
		std::cout << "col_right: " << col_right << ", col_bot: " << col_bot << ", row_bot: " << row_bot << std::endl;

		cells = new unsigned char[total_elem_count];
		memset(cells, 0, total_elem_count);

		unsigned int idx = 0;
		char c;
		for (unsigned int y = 0; y < h; y++)
		{
			getline(in, line);
			for (unsigned int x = 0; x < w; x++)
			{
				c = line[x];
				//std::cout << "read c: " << c << " for index: " << idx << std::endl;
				if (c == 'x')  // only need to set alive cells, otherwise stay 0
				{
					*(cells + idx) = STATE_ALIVE;
				}

				if (++idx >= total_elem_count) break;
			}
		}
	}
	else std::cout << "Error opening " << filePath << std::endl;

	if (!in.eof() && in.fail())
		std::cout << "error reading " << filePath << std::endl;

	in.close();
}

void oclWriteToFile(const char* filePath, bool drawNeighbours = false)
{
	std::cout << "write file: " << filePath << "..." << std::endl;
	std::ofstream out(filePath);
	if (out.is_open())
	{
		out << w << "," << h << std::endl;
		for (unsigned int i = 0; i < total_elem_count; ++i)
		{
			if (drawNeighbours) out << (neighbours[i] + 0);
			else out << ((cells[i] & STATE_ALIVE) ? "x" : ".");

			if (i % w == w - 1) out << std::endl;
		}
	}
	else std::cout << "Error opening " << filePath << std::endl;

	out.close();
}

void initOCL(unsigned int platformId, unsigned int deviceId)
{
	cl_int err = CL_SUCCESS;
	cl::Program program;
	std::vector<cl::Device> devices;

	try
	{
		// get available platforms ( NVIDIA, Intel, AMD,...)
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0 || platforms.size() < platformId) throw "specified OpenCL platform not available!";

#ifdef _DEBUG
		// test output to gather information about installed hardware:
		// SyntaX-Desktop:
		//	platform: NVIDIA CUDA
		//		device: GeForce GTX 1080
		//	platform: Intel(R) OpenCL HD Graphics
		//		device : Intel(R) HD Graphics 530
		for (auto& platform : platforms)
		{
			std::cout << "platform: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";
			//cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(), 0 };
			//cl::Context context(CL_DEVICE_TYPE_ALL, properties);
			//devices = context.getInfo<CL_CONTEXT_DEVICES>();
			platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
			for (auto& device : devices)
			{
				std::cout << "device: " << device.getInfo<CL_DEVICE_NAME>() << "\n";
			}
		}
#endif

		// create a context and get available devices
		cl::Platform platform = platforms[platformId];
		std::cout << "using platform: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";

		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
		if (devices.size() == 0 || devices.size() < deviceId) throw "specified OpenCL device not available!";

		cl::Device device = devices[deviceId];
		std::cout << "using device: " << device.getInfo<CL_DEVICE_NAME>() << "\n";

		cl::Context context({ device });
	 	cl::Program::Sources sources;

		// load and build the kernel
		std::ifstream sourceFile(KERNEL_FILE);
		if (!sourceFile) std::cerr << "kernel source file " << KERNEL_FILE << " not found!" << std::endl;

		std::string sourceCode(
			std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		program = cl::Program(context, source);
		//program.build(devices);
		if (program.build({ device }) != CL_SUCCESS) std::cerr << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;

		// init buffer and Kernel
		boardBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(unsigned char) * total_elem_count);
		cacheBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(unsigned char) * total_elem_count);
		
		kernel = cl::Kernel(program, "gol_generation");

		queue = cl::CommandQueue(context, device);
		queue.enqueueWriteBuffer(boardBuffer, CL_TRUE, 0, sizeof(unsigned char) * total_elem_count, cells);
		queue.enqueueWriteBuffer(cacheBuffer, CL_TRUE, 0, sizeof(unsigned char) * total_elem_count, oldCells);
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

	// init grid from file
	Timing::getInstance()->startSetup();
	ompReadFromFile(fileI);

	// make an array for saving previous state
	oldCells = new unsigned char[total_elem_count];

	initOCL(platformId, deviceId);

	unsigned int gen = 0;
	Timing::getInstance()->stopSetup();

	Timing::getInstance()->startComputation();
	for (gen = 0; gen < generations; gen++)
	{
		kernel.setArg(0, boardBuffer);
		kernel.setArg(1, cacheBuffer);
		kernel.setArg(2, w);
		kernel.setArg(3, h);

		queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(total_elem_count), cl::NullRange);
		queue.finish();

		std::swap(boardBuffer, cacheBuffer);
	}
	// read back current board state
	queue.enqueueReadBuffer(boardBuffer, CL_TRUE, 0, sizeof(unsigned char) * total_elem_count, cells);
	Timing::getInstance()->stopComputation();

	// write out result
	Timing::getInstance()->startFinalization();
	oclWriteToFile(fileO);
	//writeToFile(fileO, true); // writes count of neighbours instead of just x / .
	Timing::getInstance()->stopFinalization();
}
