/******************************************************************************
* Copyright (C) 2010 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#include "sgpp.hpp"
#include "basis/linear/noboundary/operation/datadriven/OperationBIterativeSPHybridSSEOCLLinear.hpp"
#include "exception/operation_exception.hpp"
#include "common/AlignedMemory.hpp"

#ifdef USEOMP
#include "omp.h"
#endif

// This value is adjusted for a 2 socket Intel Westmere System (X5650) (SMT on) with 2 NVidia Fermis (GTX470)
#define PERCENT_CPUS 12

#ifdef USEICCINTRINSICS
// include SSE3 intrinsics
#include <pmmintrin.h>

union floatAbsMaskHybrid
{
   const float f;
   const int i;

   floatAbsMaskHybrid() : i(0x7FFFFFFF) {}
};

_MM_ALIGN16 const floatAbsMaskHybrid absMaskHybrid;
static const __m128 abs2MaskHybrid = _mm_load1_ps( &absMaskHybrid.f );
#endif

namespace sg
{

OperationBIterativeSPHybridSSEOCLLinear::OperationBIterativeSPHybridSSEOCLLinear(GridStorage* storage) : storage(storage)
{
	Level = new DataMatrixSP(storage->size(), storage->dim());
	Index = new DataMatrixSP(storage->size(), storage->dim());

	storage->getLevelIndexArraysForEval(*Level, *Index);

	myTimer = new SGppStopwatch();
	myOCLKernels = new OCLKernels();
}

OperationBIterativeSPHybridSSEOCLLinear::~OperationBIterativeSPHybridSSEOCLLinear()
{
	delete Level;
	delete Index;
	delete myTimer;
	delete myOCLKernels;
}

void OperationBIterativeSPHybridSSEOCLLinear::rebuildLevelAndIndex()
{
	delete Level;
	delete Index;

	Level = new DataMatrixSP(storage->size(), storage->dim());
	Index = new DataMatrixSP(storage->size(), storage->dim());

	storage->getLevelIndexArraysForEval(*Level, *Index);

	myOCLKernels->resetKernels();
}

double OperationBIterativeSPHybridSSEOCLLinear::multVectorized(DataVectorSP& alpha, DataMatrixSP& data, DataVectorSP& result)
{
	size_t source_size = alpha.getSize();
    size_t dims = storage->dim();
    size_t storageSize = storage->size();

    result.setAll(0.0f);

    float* ptrSource = alpha.getPointer();
    float* ptrData = data.getPointer();
    float* ptrLevel = this->Level->getPointer();
    float* ptrIndex = this->Index->getPointer();
    float* ptrGlobalResult = result.getPointer();

    if (data.getNrows() % 128 != 0 || source_size != data.getNrows())
    {
    	throw operation_exception("For iterative mult an even number of instances is required and result vector length must fit to data!");
    }

    double time = myOCLKernels->multSPOCL(ptrSource, ptrData, ptrLevel, ptrIndex, ptrGlobalResult, source_size, storageSize, dims);

    // do the rest...
	size_t numWGs = storageSize/OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_SP;
    size_t global = numWGs*OCL_MULT_N_DATAPREFETCH_BLOCKSIZE_SP;

    if (global == 0)
    {
    	global = storageSize;
    }

#ifdef USEOMP
	#pragma omp parallel for
#endif
	for (size_t j = global; j < storageSize; j++)
	{
		ptrGlobalResult[j] = 0.0f;

		for (size_t i = 0; i < source_size; i++)
		{
			float curSupport = ptrSource[i];

			for (size_t d = 0; d < dims; d++)
			{
				float eval = ((ptrLevel[(j*dims)+d]) * (ptrData[(i*dims)+d]));
				float index_calc = eval - (ptrIndex[(j*dims)+d]);
				float abs = fabs(index_calc);
				float last = 1.0f - abs;
				float localSupport = std::max<float>(last, 0.0f);
				curSupport *= localSupport;
			}

			ptrGlobalResult[j] += curSupport;
		}
	}

	return time;
}

double OperationBIterativeSPHybridSSEOCLLinear::multTransposeVectorized(DataVectorSP& alpha, DataMatrixSP& data, DataVectorSP& result)
{
	size_t result_size = result.getSize();
    size_t dims = storage->dim();
    size_t storageSize = storage->size();

    result.setAll(0.0f);

    float* ptrAlpha = alpha.getPointer();
    float* ptrData = data.getPointer();
    float* ptrResult = result.getPointer();
    float* ptrLevel = this->Level->getPointer();
    float* ptrIndex = this->Index->getPointer();

    if (data.getNrows() % 128 != 0 || result_size != data.getNrows())
    {
    	throw operation_exception("For iterative mult transpose an even number of instances is required and result vector length must fit to data!");
    }

#ifdef USEOMPTHREE
    // split result into GPU and CPU partition
    size_t cpu_partition = (result_size * PERCENT_CPUS)/100;
    size_t cpu_pad = cpu_partition % 128;
    cpu_partition -= cpu_pad;
    size_t gpu_partition = result_size - cpu_partition;

	#pragma omp parallel
    {
		#pragma omp single nowait
    	{
			#pragma omp task
    		{
    			myOCLKernels->multTransSPOCL(ptrAlpha, ptrData, ptrLevel, ptrIndex, ptrResult, result_size, storageSize, dims, gpu_partition);
    		}

			#pragma omp task
    		{
#ifdef USEICCINTRINSICS
				for (size_t i = gpu_partition; i < result_size; i+=16)
				{
					#pragma omp task firstprivate(i)
					{
						__m128 res_0 = _mm_load_ps(&(ptrResult[i]));
						__m128 res_1 = _mm_load_ps(&(ptrResult[i+4]));
						__m128 res_2 = _mm_load_ps(&(ptrResult[i+8]));
						__m128 res_3 = _mm_load_ps(&(ptrResult[i+12]));

						// Do on-demand transpose
						float* ptrTransData = new float[dims*16];
						for (size_t n = 0; n < 16; n++)
						{
							for(size_t d = 0; d < dims; d++)
							{
								ptrTransData[(d*16)+n] = ptrData[((i+n)*dims)+d];
							}
						}

						for (size_t j = 0; j < storageSize; j++)
						{
							__m128 support_0 = _mm_load1_ps(&(ptrAlpha[j]));
							__m128 support_1 = _mm_load1_ps(&(ptrAlpha[j]));
							__m128 support_2 = _mm_load1_ps(&(ptrAlpha[j]));
							__m128 support_3 = _mm_load1_ps(&(ptrAlpha[j]));

							__m128 one = _mm_set1_ps(1.0f);
							__m128 zero = _mm_set1_ps(0.0f);

							for (size_t d = 0; d < dims; d++)
							{
								__m128 eval_0 = _mm_load_ps(&(ptrTransData[(d*16)]));
								__m128 eval_1 = _mm_load_ps(&(ptrTransData[(d*16)+4]));
								__m128 eval_2 = _mm_load_ps(&(ptrTransData[(d*16)+8]));
								__m128 eval_3 = _mm_load_ps(&(ptrTransData[(d*16)+12]));;

								__m128 level = _mm_load1_ps(&(ptrLevel[(j*dims)+d]));
								__m128 index = _mm_load1_ps(&(ptrIndex[(j*dims)+d]));

								eval_0 = _mm_mul_ps(eval_0, level);
								eval_1 = _mm_mul_ps(eval_1, level);
								eval_2 = _mm_mul_ps(eval_2, level);
								eval_3 = _mm_mul_ps(eval_3, level);

								eval_0 = _mm_sub_ps(eval_0, index);
								eval_1 = _mm_sub_ps(eval_1, index);
								eval_2 = _mm_sub_ps(eval_2, index);
								eval_3 = _mm_sub_ps(eval_3, index);

								eval_0 = _mm_and_ps(abs2MaskHybrid, eval_0);
								eval_1 = _mm_and_ps(abs2MaskHybrid, eval_1);
								eval_2 = _mm_and_ps(abs2MaskHybrid, eval_2);
								eval_3 = _mm_and_ps(abs2MaskHybrid, eval_3);

								eval_0 = _mm_sub_ps(one, eval_0);
								eval_1 = _mm_sub_ps(one, eval_1);
								eval_2 = _mm_sub_ps(one, eval_2);
								eval_3 = _mm_sub_ps(one, eval_3);

								eval_0 = _mm_max_ps(zero, eval_0);
								eval_1 = _mm_max_ps(zero, eval_1);
								eval_2 = _mm_max_ps(zero, eval_2);
								eval_3 = _mm_max_ps(zero, eval_3);

								support_0 = _mm_mul_ps(support_0, eval_0);
								support_1 = _mm_mul_ps(support_1, eval_1);
								support_2 = _mm_mul_ps(support_2, eval_2);
								support_3 = _mm_mul_ps(support_3, eval_3);
							}

							res_0 = _mm_add_ps(res_0, support_0);
							res_1 = _mm_add_ps(res_1, support_1);
							res_2 = _mm_add_ps(res_2, support_2);
							res_3 = _mm_add_ps(res_3, support_3);
						}

						delete[] ptrTransData;

						_mm_store_ps(&(ptrResult[i]), res_0);
						_mm_store_ps(&(ptrResult[i+4]), res_1);
						_mm_store_ps(&(ptrResult[i+8]), res_2);
						_mm_store_ps(&(ptrResult[i+12]), res_3);
					}
				}
#else
				for (size_t i = gpu_partition; i < result_size; i++)
				{
					#pragma omp task firstprivate(i)
					{
						for (size_t j = 0; j < storageSize; j++)
						{
							float curSupport = ptrAlpha[j];

							for (size_t d = 0; d < dims; d++)
							{
								float eval = ((ptrLevel[(j*dims)+d]) * (ptrData[(i*dims)+d]));
								float index_calc = eval - (ptrIndex[(j*dims)+d]);
								float abs = fabs(index_calc);
								float last = 1.0f - abs;
								float localSupport = std::max<double>(last, 0.0f);
								curSupport *= localSupport;
							}

							ptrResult[i] += curSupport;
						}
					}
				}
#endif
    		}
    	}
    }
    double time = 0.0;
#else
    double time = myOCLKernels->multTransSPOCL(ptrAlpha, ptrData, ptrLevel, ptrIndex, ptrResult, result_size, storageSize, dims, result_size);
#endif
   	return time;
}

}
