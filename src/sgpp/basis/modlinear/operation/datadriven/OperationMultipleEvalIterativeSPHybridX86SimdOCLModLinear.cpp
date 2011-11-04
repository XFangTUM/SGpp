/******************************************************************************
* Copyright (C) 2010 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#include "basis/modlinear/operation/datadriven/OperationMultipleEvalIterativeSPHybridX86SimdOCLModLinear.hpp"
#include "exception/operation_exception.hpp"
#include "tools/common/AlignedMemory.hpp"

#if defined(__SSE3__) || defined(__AVX__)
#include <x86intrin.h>
#endif

#ifdef __USEAVX128__
#undef __AVX__
#endif

namespace sg
{
namespace parallel
{

OperationMultipleEvalIterativeSPHybridX86SimdOCLModLinear::OperationMultipleEvalIterativeSPHybridX86SimdOCLModLinear(sg::base::GridStorage* storage, sg::base::DataMatrixSP* dataset) : sg::base::OperationMultipleEvalVectorizedSP(dataset)
{
	this->storage = storage;

	this->level_ = new sg::base::DataMatrixSP(storage->size(), storage->dim());
	this->index_ = new sg::base::DataMatrixSP(storage->size(), storage->dim());

	storage->getLevelIndexArraysForEval(*(this->level_), *(this->index_));

	myTimer = new sg::base::SGppStopwatch();
	myOCLKernels = new OCLKernels();

	_tuningMult = new sg::base::TwoPartitionAutoTuning(dataset->getNrows(), 128, 10, 0.9, 5);
	_tuningMultTrans = new sg::base::TwoPartitionAutoTuning(storage->size(), 128, 10, 0.9, 5);
}

OperationMultipleEvalIterativeSPHybridX86SimdOCLModLinear::~OperationMultipleEvalIterativeSPHybridX86SimdOCLModLinear()
{
	delete myTimer;
	delete myOCLKernels;
	delete _tuningMult;
	delete _tuningMultTrans;
}

void OperationMultipleEvalIterativeSPHybridX86SimdOCLModLinear::rebuildLevelAndIndex()
{
	delete this->level_;
	delete this->index_;

	this->level_ = new sg::base::DataMatrixSP(storage->size(), storage->dim());
	this->index_ = new sg::base::DataMatrixSP(storage->size(), storage->dim());

	storage->getLevelIndexArraysForEval(*(this->level_), *(this->index_));

	myOCLKernels->resetKernels();

	_tuningMultTrans->setProblemSize(storage->size());
	_tuningMult->softResetAutoTuning();
}

double OperationMultipleEvalIterativeSPHybridX86SimdOCLModLinear::multTransposeVectorized(sg::base::DataVectorSP& source, sg::base::DataVectorSP& result)
{
	size_t source_size = source.getSize();
    size_t dims = storage->dim();
    size_t storageSize = storage->size();

    double gpu_time = 0.0;
    double cpu_time = 0.0;

    result.setAll(0.0f);

    float* ptrSource = source.getPointer();
    float* ptrData = this->dataset_->getPointer();
    float* ptrLevel = this->level_->getPointer();
    float* ptrIndex = this->index_->getPointer();
    float* ptrGlobalResult = result.getPointer();

    if (this->dataset_->getNrows() % 128 != 0 || source_size != this->dataset_->getNrows())
    {
    	throw sg::base::operation_exception("For iterative mult an even number of instances is required and result vector length must fit to data!");
    }

    // split result into GPU and CPU partition
    size_t gpu_partition = storageSize - _tuningMultTrans->getPartition1Size();
    // Do on-demand transpose
	float* ptrTransData = new float[dims*source_size];

	#pragma omp parallel shared(gpu_time, cpu_time)
    {
		#pragma omp single nowait
    	{
			#pragma omp task shared(gpu_time, cpu_time)
    		{
    			if (gpu_partition > 0)
    			{
    				gpu_time = myOCLKernels->multTransModSPOCL(ptrSource, ptrData, ptrLevel, ptrIndex, ptrGlobalResult, source_size, storageSize, dims, gpu_partition);
    			}
    		}

			#pragma omp task shared(gpu_time, cpu_time)
    		{
    			myTimer->start();
#if defined(__SSE3__) && !defined(__AVX__)
    			for (size_t n = 0; n < source_size; n++)
    			{
    				for(size_t d = 0; d < dims; d++)
    				{
    					ptrTransData[(d*source_size)+n] = ptrData[(n*dims)+d];
    				}
    			}

    			for (size_t j = gpu_partition; j < storageSize; j++)
				{
					#pragma omp task firstprivate(j)
					{
						__m128 res = _mm_set1_ps(0.0f);
						int imask = 0x7FFFFFFF;
						float* fmask = (float*)&imask;

						for (size_t i = 0; i < source_size; i+=16)
						{
							__m128 support_0 = _mm_load_ps(&(ptrSource[i+0]));
							__m128 support_1 = _mm_load_ps(&(ptrSource[i+4]));
							__m128 support_2 = _mm_load_ps(&(ptrSource[i+8]));
							__m128 support_3 = _mm_load_ps(&(ptrSource[i+12]));

							__m128 one = _mm_set1_ps(1.0f);
							__m128 two = _mm_set1_ps(2.0f);
							__m128 zero = _mm_set1_ps(0.0f);
							__m128 mask = _mm_set1_ps(*fmask);

							for (size_t d = 0; d < dims; d++)
							{
								// special case for level 1
								if (ptrLevel[(j*dims)+d] == 2.0f)
								{
									// Nothing (multiply by one)
								}
								// most left basis function on every level
								else if (ptrIndex[(j*dims)+d] == 1.0f)
								{
									__m128 eval_0 = _mm_load_ps(&(ptrTransData[(d*source_size)+i]));
									__m128 eval_1 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+4]));
									__m128 eval_2 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+8]));
									__m128 eval_3 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+12]));;

									__m128 level = _mm_load1_ps(&(ptrLevel[(j*dims)+d]));

									eval_0 = _mm_mul_ps(eval_0, level);
									eval_1 = _mm_mul_ps(eval_1, level);
									eval_2 = _mm_mul_ps(eval_2, level);
									eval_3 = _mm_mul_ps(eval_3, level);

									eval_0 = _mm_sub_ps(two, eval_0);
									eval_1 = _mm_sub_ps(two, eval_1);
									eval_2 = _mm_sub_ps(two, eval_2);
									eval_3 = _mm_sub_ps(two, eval_3);

									eval_0 = _mm_max_ps(zero, eval_0);
									eval_1 = _mm_max_ps(zero, eval_1);
									eval_2 = _mm_max_ps(zero, eval_2);
									eval_3 = _mm_max_ps(zero, eval_3);

									support_0 = _mm_mul_ps(support_0, eval_0);
									support_1 = _mm_mul_ps(support_1, eval_1);
									support_2 = _mm_mul_ps(support_2, eval_2);
									support_3 = _mm_mul_ps(support_3, eval_3);
								}
								// most right basis function on every level
								else if (ptrIndex[(j*dims)+d] == (ptrLevel[(j*dims)+d] - 1.0f))
								{
									__m128 eval_0 = _mm_load_ps(&(ptrTransData[(d*source_size)+i]));
									__m128 eval_1 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+4]));
									__m128 eval_2 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+8]));
									__m128 eval_3 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+12]));;

									__m128 level = _mm_load1_ps(&(ptrLevel[(j*dims)+d]));
									__m128 index = _mm_load1_ps(&(ptrIndex[(j*dims)+d]));
#ifdef __FMA4__
									eval_0 = _mm_msub_ps(eval_0, level, index);
									eval_1 = _mm_msub_ps(eval_1, level, index);
									eval_2 = _mm_msub_ps(eval_2, level, index);
									eval_3 = _mm_msub_ps(eval_3, level, index);
#else
									eval_0 = _mm_sub_ps(_mm_mul_ps(eval_0, level), index);
									eval_1 = _mm_sub_ps(_mm_mul_ps(eval_1, level), index);
									eval_2 = _mm_sub_ps(_mm_mul_ps(eval_2, level), index);
									eval_3 = _mm_sub_ps(_mm_mul_ps(eval_3, level), index);
#endif
									eval_0 = _mm_add_ps(one, eval_0);
									eval_1 = _mm_add_ps(one, eval_1);
									eval_2 = _mm_add_ps(one, eval_2);
									eval_3 = _mm_add_ps(one, eval_3);

									eval_0 = _mm_max_ps(zero, eval_0);
									eval_1 = _mm_max_ps(zero, eval_1);
									eval_2 = _mm_max_ps(zero, eval_2);
									eval_3 = _mm_max_ps(zero, eval_3);

									support_0 = _mm_mul_ps(support_0, eval_0);
									support_1 = _mm_mul_ps(support_1, eval_1);
									support_2 = _mm_mul_ps(support_2, eval_2);
									support_3 = _mm_mul_ps(support_3, eval_3);
								}
								// all other basis functions
								else
								{
									__m128 eval_0 = _mm_load_ps(&(ptrTransData[(d*source_size)+i]));
									__m128 eval_1 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+4]));
									__m128 eval_2 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+8]));
									__m128 eval_3 = _mm_load_ps(&(ptrTransData[(d*source_size)+i+12]));;

									__m128 level = _mm_load1_ps(&(ptrLevel[(j*dims)+d]));
									__m128 index = _mm_load1_ps(&(ptrIndex[(j*dims)+d]));
#ifdef __FMA4__
									eval_0 = _mm_msub_ps(eval_0, level, index);
									eval_1 = _mm_msub_ps(eval_1, level, index);
									eval_2 = _mm_msub_ps(eval_2, level, index);
									eval_3 = _mm_msub_ps(eval_3, level, index);
#else
									eval_0 = _mm_sub_ps(_mm_mul_ps(eval_0, level), index);
									eval_1 = _mm_sub_ps(_mm_mul_ps(eval_1, level), index);
									eval_2 = _mm_sub_ps(_mm_mul_ps(eval_2, level), index);
									eval_3 = _mm_sub_ps(_mm_mul_ps(eval_3, level), index);
#endif
									eval_0 = _mm_and_ps(mask, eval_0);
									eval_1 = _mm_and_ps(mask, eval_1);
									eval_2 = _mm_and_ps(mask, eval_2);
									eval_3 = _mm_and_ps(mask, eval_3);

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
							}

							support_0 = _mm_add_ps(support_0, support_1);
							support_2 = _mm_add_ps(support_2, support_3);
							support_0 = _mm_add_ps(support_0, support_2);

							res = _mm_add_ps(res, support_0);
						}

						res = _mm_hadd_ps(res, res);
						res = _mm_hadd_ps(res, res);

						_mm_store_ss(&(ptrGlobalResult[j]), res);
					}
				}
#endif
#if defined(__SSE3__) && defined(__AVX__)
       			for (size_t n = 0; n < source_size; n++)
        			{
        				for(size_t d = 0; d < dims; d++)
        				{
        					ptrTransData[(d*source_size)+n] = ptrData[(n*dims)+d];
        				}
        			}

        			for (size_t j = gpu_partition; j < storageSize; j++)
    				{
    					#pragma omp task firstprivate(j)
    					{
    						__m256 res = _mm_set1_ps(0.0f);
    						int imask = 0x7FFFFFFF;
    						float* fmask = (float*)&imask;

    						for (size_t i = 0; i < source_size; i+=32)
    						{
    							__m256 support_0 = _mm256_load_ps(&(ptrSource[i+0]));
    							__m256 support_1 = _mm256_load_ps(&(ptrSource[i+8]));
    							__m256 support_2 = _mm256_load_ps(&(ptrSource[i+16]));
    							__m256 support_3 = _mm256_load_ps(&(ptrSource[i+24]));

    							__m256 one = _mm256_set1_ps(1.0f);
    							__m256 two = _mm256_set1_ps(2.0f);
    							__m256 zero = _mm256_set1_ps(0.0f);
								__m256 mask = _mm256_set1_ps(*fmask);

    							for (size_t d = 0; d < dims; d++)
    							{
    								// special case for level 1
    								if (ptrLevel[(j*dims)+d] == 2.0f)
    								{
    									// Nothing (multiply by one)
    								}
    								// most left basis function on every level
    								else if (ptrIndex[(j*dims)+d] == 1.0f)
    								{
    									__m256 eval_0 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i]));
    									__m256 eval_1 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+8]));
    									__m256 eval_2 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+16]));
    									__m256 eval_3 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+24]));

    									__m256 level = _mm256_broadcast_ss(&(ptrLevel[(j*dims)+d]));

    									eval_0 = _mm256_mul_ps(eval_0, level);
    									eval_1 = _mm256_mul_ps(eval_1, level);
    									eval_2 = _mm256_mul_ps(eval_2, level);
    									eval_3 = _mm256_mul_ps(eval_3, level);

    									eval_0 = _mm256_sub_ps(two, eval_0);
    									eval_1 = _mm256_sub_ps(two, eval_1);
    									eval_2 = _mm256_sub_ps(two, eval_2);
    									eval_3 = _mm256_sub_ps(two, eval_3);

    									eval_0 = _mm256_max_ps(zero, eval_0);
    									eval_1 = _mm256_max_ps(zero, eval_1);
    									eval_2 = _mm256_max_ps(zero, eval_2);
    									eval_3 = _mm256_max_ps(zero, eval_3);

    									support_0 = _mm256_mul_ps(support_0, eval_0);
    									support_1 = _mm256_mul_ps(support_1, eval_1);
    									support_2 = _mm256_mul_ps(support_2, eval_2);
    									support_3 = _mm256_mul_ps(support_3, eval_3);
    								}
    								// most right basis function on every level
    								else if (ptrIndex[(j*dims)+d] == (ptrLevel[(j*dims)+d] - 1.0f))
    								{
    									__m256 eval_0 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i]));
    									__m256 eval_1 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+8]));
    									__m256 eval_2 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+16]));
    									__m256 eval_3 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+24]));

    									__m256 level = _mm256_broadcast_ss(&(ptrLevel[(j*dims)+d]));
    									__m256 index = _mm256_broadcast_ss(&(ptrIndex[(j*dims)+d]));
#ifdef __FMA4__
										eval_0 = _mm256_msub_ps(eval_0, level, index);
										eval_1 = _mm256_msub_ps(eval_1, level, index);
										eval_2 = _mm256_msub_ps(eval_2, level, index);
										eval_3 = _mm256_msub_ps(eval_3, level, index);
#else
										eval_0 = _mm256_sub_ps(_mm256_mul_ps(eval_0, level), index);
										eval_1 = _mm256_sub_ps(_mm256_mul_ps(eval_1, level), index);
										eval_2 = _mm256_sub_ps(_mm256_mul_ps(eval_2, level), index);
										eval_3 = _mm256_sub_ps(_mm256_mul_ps(eval_3, level), index);
#endif
    									eval_0 = _mm256_add_ps(one, eval_0);
    									eval_1 = _mm256_add_ps(one, eval_1);
    									eval_2 = _mm256_add_ps(one, eval_2);
    									eval_3 = _mm256_add_ps(one, eval_3);

    									eval_0 = _mm256_max_ps(zero, eval_0);
    									eval_1 = _mm256_max_ps(zero, eval_1);
    									eval_2 = _mm256_max_ps(zero, eval_2);
    									eval_3 = _mm256_max_ps(zero, eval_3);

    									support_0 = _mm256_mul_ps(support_0, eval_0);
    									support_1 = _mm256_mul_ps(support_1, eval_1);
    									support_2 = _mm256_mul_ps(support_2, eval_2);
    									support_3 = _mm256_mul_ps(support_3, eval_3);
    								}
    								// all other basis functions
    								else
    								{
    									__m256 eval_0 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i]));
    									__m256 eval_1 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+8]));
    									__m256 eval_2 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+16]));
    									__m256 eval_3 = _mm256_load_ps(&(ptrTransData[(d*source_size)+i+24]));

    									__m128 level = _mm_load1_ps(&(ptrLevel[(j*dims)+d]));
    									__m128 index = _mm_load1_ps(&(ptrIndex[(j*dims)+d]));
#ifdef __FMA4__
										eval_0 = _mm256_msub_ps(eval_0, level, index);
										eval_1 = _mm256_msub_ps(eval_1, level, index);
										eval_2 = _mm256_msub_ps(eval_2, level, index);
										eval_3 = _mm256_msub_ps(eval_3, level, index);
#else
										eval_0 = _mm256_sub_ps(_mm256_mul_ps(eval_0, level), index);
										eval_1 = _mm256_sub_ps(_mm256_mul_ps(eval_1, level), index);
										eval_2 = _mm256_sub_ps(_mm256_mul_ps(eval_2, level), index);
										eval_3 = _mm256_sub_ps(_mm256_mul_ps(eval_3, level), index);
#endif
    									eval_0 = _mm256_and_ps(mask, eval_0);
    									eval_1 = _mm256_and_ps(mask, eval_1);
    									eval_2 = _mm256_and_ps(mask, eval_2);
    									eval_3 = _mm256_and_ps(mask, eval_3);

    									eval_0 = _mm256_sub_ps(one, eval_0);
    									eval_1 = _mm256_sub_ps(one, eval_1);
    									eval_2 = _mm256_sub_ps(one, eval_2);
    									eval_3 = _mm256_sub_ps(one, eval_3);

    									eval_0 = _mm256_max_ps(zero, eval_0);
    									eval_1 = _mm256_max_ps(zero, eval_1);
    									eval_2 = _mm256_max_ps(zero, eval_2);
    									eval_3 = _mm256_max_ps(zero, eval_3);

    									support_0 = _mm256_mul_ps(support_0, eval_0);
    									support_1 = _mm256_mul_ps(support_1, eval_1);
    									support_2 = _mm256_mul_ps(support_2, eval_2);
    									support_3 = _mm256_mul_ps(support_3, eval_3);
    								}
    							}

    							support_0 = _mm256_add_ps(support_0, support_1);
    							support_2 = _mm256_add_ps(support_2, support_3);
    							support_0 = _mm256_add_ps(support_0, support_2);

    							res = _mm256_add_ps(res, support_0);
    						}

    						const __m256i ldStMaskSPAVX = _mm256_set_epi32(0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF);

    						res = _mm256_hadd_ps(res, res);
    						__m256 tmp = _mm256_permute2f128_ps(res, res, 0x81);
    						res = _mm256_add_ps(res, tmp);
    						res = _mm256_hadd_ps(res, res);

    						_mm256_maskstore_ps(&(ptrResult[j]), ldStMaskSPAVX, res_0);
    					}
    				}
#endif
#if !defined(__SSE3__) && !defined(__AVX__)
				for (size_t j = gpu_partition; j < storageSize; j++)
				{
					#pragma omp task firstprivate(j)
					{
						ptrGlobalResult[j] = 0.0f;

						for (size_t i = 0; i < source_size; i++)
						{
							float curSupport = ptrSource[i];

							for (size_t d = 0; d < dims; d++)
							{
								if (ptrLevel[(j*dims)+d] == 2.0f)
								{
									// nothing to do (mult with 1)
								}
								else if (ptrIndex[(j*dims)+d] == 1.0f)
								{
									float eval = ((ptrLevel[(j*dims)+d]) * (ptrData[(i*dims)+d]));
									eval = 2.0f - eval;
									float localSupport = std::max<float>(eval, 0.0f);
									curSupport *= localSupport;
								}
								else if (ptrIndex[(j*dims)+d] == (ptrLevel[(j*dims)+d] - 1.0f))
								{
									float eval = ((ptrLevel[(j*dims)+d]) * (ptrData[(i*dims)+d]));
									float index_calc = eval - (ptrIndex[(j*dims)+d]);
									float last = 1.0f + index_calc;
									float localSupport = std::max<float>(last, 0.0f);
									curSupport *= localSupport;
								}
								else
								{
									float eval = ((ptrLevel[(j*dims)+d]) * (ptrData[(i*dims)+d]));
									float index_calc = eval - (ptrIndex[(j*dims)+d]);
									float abs = fabs(index_calc);
									float last = 1.0f - abs;
									float localSupport = std::max<float>(last, 0.0f);
									curSupport *= localSupport;
								}
							}

							ptrGlobalResult[j] += curSupport;
						}
					}
				}
#endif
				cpu_time = myTimer->stop();
    		}

			#pragma omp taskwait
    	}
    }

    _tuningMultTrans->setExecutionTimes(cpu_time, gpu_time);

    double time = 0.0;
    //cleanup
    delete[] ptrTransData;

	return time;
}

double OperationMultipleEvalIterativeSPHybridX86SimdOCLModLinear::multVectorized(sg::base::DataVectorSP& alpha, sg::base::DataVectorSP& result)
{
	size_t result_size = result.getSize();
    size_t dims = storage->dim();
    size_t storageSize = storage->size();

    double gpu_time = 0.0;
    double cpu_time = 0.0;

    result.setAll(0.0f);

    float* ptrAlpha = alpha.getPointer();
    float* ptrData = this->dataset_->getPointer();
    float* ptrResult = result.getPointer();
    float* ptrLevel = this->level_->getPointer();
    float* ptrIndex = this->index_->getPointer();

    if (this->dataset_->getNrows() % 128 != 0 || result_size != this->dataset_->getNrows())
    {
    	throw sg::base::operation_exception("For iterative mult transpose an even number of instances is required and result vector length must fit to data!");
    }

    // split result into GPU and CPU partition
    size_t gpu_partition = result_size - _tuningMult->getPartition1Size();

	#pragma omp parallel shared(gpu_time, cpu_time)
    {
		#pragma omp single nowait
    	{
			#pragma omp task shared(gpu_time, cpu_time)
    		{
    			if (gpu_partition > 0)
    			{
    				gpu_time = myOCLKernels->multModSPOCL(ptrAlpha, ptrData, ptrLevel, ptrIndex, ptrResult, result_size, storageSize, dims, gpu_partition);
    			}
    		}

			#pragma omp task shared(gpu_time, cpu_time)
    		{
    			myTimer->start();
#if defined(__SSE3__) && !defined(__AVX__)
				for (size_t i = gpu_partition; i < result_size; i+=16)
				{
					#pragma omp task firstprivate(i)
					{
						int imask = 0x7FFFFFFF;
						float* fmask = (float*)&imask;

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
							__m128 two = _mm_set1_ps(2.0f);
							__m128 zero = _mm_set1_ps(0.0f);
							__m128 mask = _mm_set1_ps(*fmask);

							for (size_t d = 0; d < dims; d++)
							{
								// special case for level 1
								if (ptrLevel[(j*dims)+d] == 2.0f)
								{
									// Nothing (multiply by one)
								}
								// most left basis function on every level
								else if (ptrIndex[(j*dims)+d] == 1.0f)
								{
									__m128 eval_0 = _mm_load_ps(&(ptrTransData[(d*16)]));
									__m128 eval_1 = _mm_load_ps(&(ptrTransData[(d*16)+4]));
									__m128 eval_2 = _mm_load_ps(&(ptrTransData[(d*16)+8]));
									__m128 eval_3 = _mm_load_ps(&(ptrTransData[(d*16)+12]));

									__m128 level = _mm_load1_ps(&(ptrLevel[(j*dims)+d]));

									eval_0 = _mm_mul_ps(eval_0, level);
									eval_1 = _mm_mul_ps(eval_1, level);
									eval_2 = _mm_mul_ps(eval_2, level);
									eval_3 = _mm_mul_ps(eval_3, level);

									eval_0 = _mm_sub_ps(two, eval_0);
									eval_1 = _mm_sub_ps(two, eval_1);
									eval_2 = _mm_sub_ps(two, eval_2);
									eval_3 = _mm_sub_ps(two, eval_3);

									eval_0 = _mm_max_ps(zero, eval_0);
									eval_1 = _mm_max_ps(zero, eval_1);
									eval_2 = _mm_max_ps(zero, eval_2);
									eval_3 = _mm_max_ps(zero, eval_3);

									support_0 = _mm_mul_ps(support_0, eval_0);
									support_1 = _mm_mul_ps(support_1, eval_1);
									support_2 = _mm_mul_ps(support_2, eval_2);
									support_3 = _mm_mul_ps(support_3, eval_3);
								}
								// most right basis function on every level
								else if (ptrIndex[(j*dims)+d] == (ptrLevel[(j*dims)+d] - 1.0f))
								{
									__m128 eval_0 = _mm_load_ps(&(ptrTransData[(d*16)]));
									__m128 eval_1 = _mm_load_ps(&(ptrTransData[(d*16)+4]));
									__m128 eval_2 = _mm_load_ps(&(ptrTransData[(d*16)+8]));
									__m128 eval_3 = _mm_load_ps(&(ptrTransData[(d*16)+12]));

									__m128 level = _mm_load1_ps(&(ptrLevel[(j*dims)+d]));
									__m128 index = _mm_load1_ps(&(ptrIndex[(j*dims)+d]));
#ifdef __FMA4__
									eval_0 = _mm_msub_ps(eval_0, level, index);
									eval_1 = _mm_msub_ps(eval_1, level, index);
									eval_2 = _mm_msub_ps(eval_2, level, index);
									eval_3 = _mm_msub_ps(eval_3, level, index);
#else
									eval_0 = _mm_sub_ps(_mm_mul_ps(eval_0, level), index);
									eval_1 = _mm_sub_ps(_mm_mul_ps(eval_1, level), index);
									eval_2 = _mm_sub_ps(_mm_mul_ps(eval_2, level), index);
									eval_3 = _mm_sub_ps(_mm_mul_ps(eval_3, level), index);
#endif
									eval_0 = _mm_add_ps(one, eval_0);
									eval_1 = _mm_add_ps(one, eval_1);
									eval_2 = _mm_add_ps(one, eval_2);
									eval_3 = _mm_add_ps(one, eval_3);

									eval_0 = _mm_max_ps(zero, eval_0);
									eval_1 = _mm_max_ps(zero, eval_1);
									eval_2 = _mm_max_ps(zero, eval_2);
									eval_3 = _mm_max_ps(zero, eval_3);

									support_0 = _mm_mul_ps(support_0, eval_0);
									support_1 = _mm_mul_ps(support_1, eval_1);
									support_2 = _mm_mul_ps(support_2, eval_2);
									support_3 = _mm_mul_ps(support_3, eval_3);
								}
								// all other basis functions
								else
								{
									__m128 eval_0 = _mm_load_ps(&(ptrTransData[(d*16)]));
									__m128 eval_1 = _mm_load_ps(&(ptrTransData[(d*16)+4]));
									__m128 eval_2 = _mm_load_ps(&(ptrTransData[(d*16)+8]));
									__m128 eval_3 = _mm_load_ps(&(ptrTransData[(d*16)+12]));

									__m128 level = _mm_load1_ps(&(ptrLevel[(j*dims)+d]));
									__m128 index = _mm_load1_ps(&(ptrIndex[(j*dims)+d]));
#ifdef __FMA4__
									eval_0 = _mm_msub_ps(eval_0, level, index);
									eval_1 = _mm_msub_ps(eval_1, level, index);
									eval_2 = _mm_msub_ps(eval_2, level, index);
									eval_3 = _mm_msub_ps(eval_3, level, index);
#else
									eval_0 = _mm_sub_ps(_mm_mul_ps(eval_0, level), index);
									eval_1 = _mm_sub_ps(_mm_mul_ps(eval_1, level), index);
									eval_2 = _mm_sub_ps(_mm_mul_ps(eval_2, level), index);
									eval_3 = _mm_sub_ps(_mm_mul_ps(eval_3, level), index);
#endif
									eval_0 = _mm_and_ps(mask, eval_0);
									eval_1 = _mm_and_ps(mask, eval_1);
									eval_2 = _mm_and_ps(mask, eval_2);
									eval_3 = _mm_and_ps(mask, eval_3);

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
#endif
#if defined(__SSE3__) && defined(__AVX__)
				for (size_t i = gpu_partition; i < result_size; i+=32)
				{
					#pragma omp task firstprivate(i)
					{
						int imask = 0x7FFFFFFF;
						float* fmask = (float*)&imask;

						__m256 res_0 = _mm256_load_ps(&(ptrResult[i]));
						__m256 res_1 = _mm256_load_ps(&(ptrResult[i+8]));
						__m256 res_2 = _mm256_load_ps(&(ptrResult[i+16]));
						__m256 res_3 = _mm256_load_ps(&(ptrResult[i+32]));

						// Do on-demand transpose
						float* ptrTransData = new float[dims*32];
						for (size_t n = 0; n < 32; n++)
						{
							for(size_t d = 0; d < dims; d++)
							{
								ptrTransData[(d*32)+n] = ptrData[((i+n)*dims)+d];
							}
						}

						for (size_t j = 0; j < storageSize; j++)
						{
							__m256 support_0 = _mm256_broadcast_ss(&(ptrAlpha[j]));
							__m256 support_1 = _mm256_broadcast_ss(&(ptrAlpha[j]));
							__m256 support_2 = _mm256_broadcast_ss(&(ptrAlpha[j]));
							__m256 support_3 = _mm256_broadcast_ss(&(ptrAlpha[j]));

							__m256 one = _mm_set1_ps(1.0f);
							__m256 two = _mm_set1_ps(2.0f);
							__m256 zero = _mm_set1_ps(0.0f);
							__m256 mask = _mm_set1_ps(*fmask);

							for (size_t d = 0; d < dims; d++)
							{
								// special case for level 1
								if (ptrLevel[(j*dims)+d] == 2.0f)
								{
									// Nothing (multiply by one)
								}
								// most left basis function on every level
								else if (ptrIndex[(j*dims)+d] == 1.0f)
								{
									__m256 eval_0 = _mm256_load_ps(&(ptrTransData[(d*32)]));
									__m256 eval_1 = _mm256_load_ps(&(ptrTransData[(d*32)+8]));
									__m256 eval_2 = _mm256_load_ps(&(ptrTransData[(d*32)+16]));
									__m256 eval_3 = _mm256_load_ps(&(ptrTransData[(d*32)+24]));

									__m256 level = _mm256_broadcast_ss(&(ptrLevel[(j*dims)+d]));

									eval_0 = _mm256_mul_ps(eval_0, level);
									eval_1 = _mm256_mul_ps(eval_1, level);
									eval_2 = _mm256_mul_ps(eval_2, level);
									eval_3 = _mm256_mul_ps(eval_3, level);

									eval_0 = _mm256_sub_ps(two, eval_0);
									eval_1 = _mm256_sub_ps(two, eval_1);
									eval_2 = _mm256_sub_ps(two, eval_2);
									eval_3 = _mm256_sub_ps(two, eval_3);

									eval_0 = _mm256_max_ps(zero, eval_0);
									eval_1 = _mm256_max_ps(zero, eval_1);
									eval_2 = _mm256_max_ps(zero, eval_2);
									eval_3 = _mm256_max_ps(zero, eval_3);

									support_0 = _mm256_mul_ps(support_0, eval_0);
									support_1 = _mm256_mul_ps(support_1, eval_1);
									support_2 = _mm256_mul_ps(support_2, eval_2);
									support_3 = _mm256_mul_ps(support_3, eval_3);
								}
								// most right basis function on every level
								else if (ptrIndex[(j*dims)+d] == (ptrLevel[(j*dims)+d] - 1.0f))
								{
									__m256 eval_0 = _mm256_load_ps(&(ptrTransData[(d*32)]));
									__m256 eval_1 = _mm256_load_ps(&(ptrTransData[(d*32)+8]));
									__m256 eval_2 = _mm256_load_ps(&(ptrTransData[(d*32)+16]));
									__m256 eval_3 = _mm256_load_ps(&(ptrTransData[(d*32)+24]));

									__m256 level = _mm256_broadcast_ss(&(ptrLevel[(j*dims)+d]));
									__m256 index = _mm256_broadcast_ss(&(ptrIndex[(j*dims)+d]));
#ifdef __FMA4__
									eval_0 = _mm256_msub_ps(eval_0, level, index);
									eval_1 = _mm256_msub_ps(eval_1, level, index);
									eval_2 = _mm256_msub_ps(eval_2, level, index);
									eval_3 = _mm256_msub_ps(eval_3, level, index);
#else
									eval_0 = _mm256_sub_ps(_mm256_mul_ps(eval_0, level), index);
									eval_1 = _mm256_sub_ps(_mm256_mul_ps(eval_1, level), index);
									eval_2 = _mm256_sub_ps(_mm256_mul_ps(eval_2, level), index);
									eval_3 = _mm256_sub_ps(_mm256_mul_ps(eval_3, level), index);
#endif
									eval_0 = _mm256_add_ps(one, eval_0);
									eval_1 = _mm256_add_ps(one, eval_1);
									eval_2 = _mm256_add_ps(one, eval_2);
									eval_3 = _mm256_add_ps(one, eval_3);

									eval_0 = _mm256_max_ps(zero, eval_0);
									eval_1 = _mm256_max_ps(zero, eval_1);
									eval_2 = _mm256_max_ps(zero, eval_2);
									eval_3 = _mm256_max_ps(zero, eval_3);

									support_0 = _mm256_mul_ps(support_0, eval_0);
									support_1 = _mm256_mul_ps(support_1, eval_1);
									support_2 = _mm256_mul_ps(support_2, eval_2);
									support_3 = _mm256_mul_ps(support_3, eval_3);
								}
								// all other basis functions
								else
								{
									__m256 eval_0 = _mm256_load_ps(&(ptrTransData[(d*32)]));
									__m256 eval_1 = _mm256_load_ps(&(ptrTransData[(d*32)+8]));
									__m256 eval_2 = _mm256_load_ps(&(ptrTransData[(d*32)+16]));
									__m256 eval_3 = _mm256_load_ps(&(ptrTransData[(d*32)+24]));

									__m256 level = _mm256_broadcast_ss(&(ptrLevel[(j*dims)+d]));
									__m256 index = _mm256_broadcast_ss(&(ptrIndex[(j*dims)+d]));
#ifdef __FMA4__
									eval_0 = _mm256_msub_ps(eval_0, level, index);
									eval_1 = _mm256_msub_ps(eval_1, level, index);
									eval_2 = _mm256_msub_ps(eval_2, level, index);
									eval_3 = _mm256_msub_ps(eval_3, level, index);
#else
									eval_0 = _mm256_sub_ps(_mm256_mul_ps(eval_0, level), index);
									eval_1 = _mm256_sub_ps(_mm256_mul_ps(eval_1, level), index);
									eval_2 = _mm256_sub_ps(_mm256_mul_ps(eval_2, level), index);
									eval_3 = _mm256_sub_ps(_mm256_mul_ps(eval_3, level), index);
#endif
									eval_0 = _mm256_and_ps(mask, eval_0);
									eval_1 = _mm256_and_ps(mask, eval_1);
									eval_2 = _mm256_and_ps(mask, eval_2);
									eval_3 = _mm256_and_ps(mask, eval_3);

									eval_0 = _mm256_sub_ps(one, eval_0);
									eval_1 = _mm256_sub_ps(one, eval_1);
									eval_2 = _mm256_sub_ps(one, eval_2);
									eval_3 = _mm256_sub_ps(one, eval_3);

									eval_0 = _mm256_max_ps(zero, eval_0);
									eval_1 = _mm256_max_ps(zero, eval_1);
									eval_2 = _mm256_max_ps(zero, eval_2);
									eval_3 = _mm256_max_ps(zero, eval_3);

									support_0 = _mm256_mul_ps(support_0, eval_0);
									support_1 = _mm256_mul_ps(support_1, eval_1);
									support_2 = _mm256_mul_ps(support_2, eval_2);
									support_3 = _mm256_mul_ps(support_3, eval_3);
								}
							}

							res_0 = _mm256_add_ps(res_0, support_0);
							res_1 = _mm256_add_ps(res_1, support_1);
							res_2 = _mm256_add_ps(res_2, support_2);
							res_3 = _mm256_add_ps(res_3, support_3);
						}

						delete[] ptrTransData;

						_mm256_store_ps(&(ptrResult[i]), res_0);
						_mm256_store_ps(&(ptrResult[i+8]), res_1);
						_mm256_store_ps(&(ptrResult[i+16]), res_2);
						_mm256_store_ps(&(ptrResult[i+24]), res_3);
					}
				}
#endif
#if !defined(__SSE3__) && !defined(__AVX__)
				for (size_t i = gpu_partition; i < result_size; i++)
				{
					#pragma omp task firstprivate(i)
					{
						for (size_t j = 0; j < storageSize; j++)
						{
							float curSupport = ptrAlpha[j];

							for (size_t d = 0; d < dims; d++)
							{
								if (ptrLevel[(j*dims)+d] == 2.0f)
								{
									// nothing to do (mult with 1)
								}
								else if (ptrIndex[(j*dims)+d] == 1.0f)
								{
									float eval = ((ptrLevel[(j*dims)+d]) * (ptrData[(i*dims)+d]));
									eval = 2.0f - eval;
									float localSupport = std::max<float>(eval, 0.0f);
									curSupport *= localSupport;
								}
								else if (ptrIndex[(j*dims)+d] == (ptrLevel[(j*dims)+d] - 1.0f))
								{
									float eval = ((ptrLevel[(j*dims)+d]) * (ptrData[(i*dims)+d]));
									float index_calc = eval - (ptrIndex[(j*dims)+d]);
									float last = 1.0f + index_calc;
									float localSupport = std::max<float>(last, 0.0f);
									curSupport *= localSupport;
								}
								else
								{
									float eval = ((ptrLevel[(j*dims)+d]) * (ptrData[(i*dims)+d]));
									float index_calc = eval - (ptrIndex[(j*dims)+d]);
									float abs = fabs(index_calc);
									float last = 1.0f - abs;
									float localSupport = std::max<float>(last, 0.0f);
									curSupport *= localSupport;
								}
							}


							ptrResult[i] += curSupport;
						}
					}
				}
#endif
				cpu_time = myTimer->stop();
    		}

			#pragma omp taskwait
    	}
    }

    _tuningMult->setExecutionTimes(cpu_time, gpu_time);

    double time = 0.0;

   	return time;
}

}

}