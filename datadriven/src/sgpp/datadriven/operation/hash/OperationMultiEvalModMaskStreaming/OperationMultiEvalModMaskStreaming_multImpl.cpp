// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <cmath>

#include <sgpp/datadriven/operation/hash/OperationMultiEvalModMaskStreaming/OperationMultiEvalModMaskStreaming.hpp>
#include <sgpp/globaldef.hpp>

#if defined(__SSE3__) && !defined(__AVX__)
#include <pmmintrin.h>
#endif

#if defined(__SSE3__) && defined(__AVX__)
#include <immintrin.h>
#endif

#if defined(__MIC__)
#include <immintrin.h>
#endif

namespace SGPP {
namespace datadriven {

void OperationMultiEvalModMaskStreaming::multImpl(std::vector<double> &level,
		std::vector<double> &index, std::vector<double> &mask,
		std::vector<double> &offset,
		SGPP::base::DataMatrix* dataset,
		SGPP::base::DataVector& alpha,
		SGPP::base::DataVector& result, const size_t start_index_grid,
		const size_t end_index_grid, const size_t start_index_data,
		const size_t end_index_data) {

#if USE_DOUBLE_PRECISION == 1
	double* ptrLevel = level.data();
	double* ptrIndex = index.data();
	double* ptrMask = mask.data();
	double* ptrOffset = offset.data();
	double* ptrAlpha = alpha.getPointer();
	double* ptrData = dataset->getPointer();
	double* ptrResult = result.getPointer();
	size_t result_size = result.getSize();
	size_t dims = dataset->getNrows();

#if defined(__SSE3__) && !defined(__AVX__)

	for (size_t c = start_index_data; c < end_index_data;
			c += std::min<size_t>((size_t) getChunkDataPoints(), (end_index_data - c))) {
		size_t data_end = std::min<size_t>((size_t) getChunkDataPoints() + c, end_index_data);

#ifdef __ICC
#pragma ivdep
#pragma vector aligned
#endif

		for (size_t i = c; i < data_end; i++) {
			ptrResult[i] = 0.0;
		}

		for (size_t m = start_index_grid; m < end_index_grid;
				m += std::min<size_t>((size_t) getChunkGridPoints(), (end_index_grid - m))) {

			size_t grid_inc = std::min<size_t>((size_t)getChunkGridPoints(), (end_index_grid - m));

			for (size_t i = c; i < c + getChunkDataPoints(); i += 12) {
				for (size_t j = m; j < m + grid_inc; j++) {
					__m128d support_0 = _mm_loaddup_pd(&(ptrAlpha[j]));
					__m128d support_1 = _mm_loaddup_pd(&(ptrAlpha[j]));
					__m128d support_2 = _mm_loaddup_pd(&(ptrAlpha[j]));
					__m128d support_3 = _mm_loaddup_pd(&(ptrAlpha[j]));
					__m128d support_4 = _mm_loaddup_pd(&(ptrAlpha[j]));
					__m128d support_5 = _mm_loaddup_pd(&(ptrAlpha[j]));

					__m128d zero = _mm_set1_pd(0.0);

					for (size_t d = 0; d < dims; d++) {
						__m128d eval_0 = _mm_load_pd(&(ptrData[(d * result_size) + i]));
						__m128d eval_1 = _mm_load_pd(&(ptrData[(d * result_size) + i + 2]));
						__m128d eval_2 = _mm_load_pd(&(ptrData[(d * result_size) + i + 4]));
						__m128d eval_3 = _mm_load_pd(&(ptrData[(d * result_size) + i + 6]));
						__m128d eval_4 = _mm_load_pd(&(ptrData[(d * result_size) + i + 8]));
						__m128d eval_5 = _mm_load_pd(&(ptrData[(d * result_size) + i + 10]));

						__m128d level = _mm_loaddup_pd(&(ptrLevel[(j * dims) + d]));
						__m128d index = _mm_loaddup_pd(&(ptrIndex[(j * dims) + d]));
#ifdef __FMA4__
						eval_0 = _mm_msub_pd(eval_0, level, index);
						eval_1 = _mm_msub_pd(eval_1, level, index);
						eval_2 = _mm_msub_pd(eval_2, level, index);
						eval_3 = _mm_msub_pd(eval_3, level, index);
						eval_4 = _mm_msub_pd(eval_4, level, index);
						eval_5 = _mm_msub_pd(eval_5, level, index);
#else
						eval_0 = _mm_sub_pd(_mm_mul_pd(eval_0, level), index);
						eval_1 = _mm_sub_pd(_mm_mul_pd(eval_1, level), index);
						eval_2 = _mm_sub_pd(_mm_mul_pd(eval_2, level), index);
						eval_3 = _mm_sub_pd(_mm_mul_pd(eval_3, level), index);
						eval_4 = _mm_sub_pd(_mm_mul_pd(eval_4, level), index);
						eval_5 = _mm_sub_pd(_mm_mul_pd(eval_5, level), index);
#endif
						__m128d mask = _mm_loaddup_pd(&(ptrMask[(j * dims) + d]));
						__m128d offset = _mm_loaddup_pd(&(ptrOffset[(j * dims) + d]));

						eval_0 = _mm_or_pd(mask, eval_0);
						eval_1 = _mm_or_pd(mask, eval_1);
						eval_2 = _mm_or_pd(mask, eval_2);
						eval_3 = _mm_or_pd(mask, eval_3);
						eval_4 = _mm_or_pd(mask, eval_4);
						eval_5 = _mm_or_pd(mask, eval_5);

						eval_0 = _mm_add_pd(offset, eval_0);
						eval_1 = _mm_add_pd(offset, eval_1);
						eval_2 = _mm_add_pd(offset, eval_2);
						eval_3 = _mm_add_pd(offset, eval_3);
						eval_4 = _mm_add_pd(offset, eval_4);
						eval_5 = _mm_add_pd(offset, eval_5);

						eval_0 = _mm_max_pd(zero, eval_0);
						eval_1 = _mm_max_pd(zero, eval_1);
						eval_2 = _mm_max_pd(zero, eval_2);
						eval_3 = _mm_max_pd(zero, eval_3);
						eval_4 = _mm_max_pd(zero, eval_4);
						eval_5 = _mm_max_pd(zero, eval_5);

						support_0 = _mm_mul_pd(support_0, eval_0);
						support_1 = _mm_mul_pd(support_1, eval_1);
						support_2 = _mm_mul_pd(support_2, eval_2);
						support_3 = _mm_mul_pd(support_3, eval_3);
						support_4 = _mm_mul_pd(support_4, eval_4);
						support_5 = _mm_mul_pd(support_5, eval_5);
					}

					__m128d res_0 = _mm_load_pd(&(ptrResult[i]));
					__m128d res_1 = _mm_load_pd(&(ptrResult[i + 2]));
					__m128d res_2 = _mm_load_pd(&(ptrResult[i + 4]));
					__m128d res_3 = _mm_load_pd(&(ptrResult[i + 6]));
					__m128d res_4 = _mm_load_pd(&(ptrResult[i + 8]));
					__m128d res_5 = _mm_load_pd(&(ptrResult[i + 10]));

					res_0 = _mm_add_pd(res_0, support_0);
					res_1 = _mm_add_pd(res_1, support_1);
					res_2 = _mm_add_pd(res_2, support_2);
					res_3 = _mm_add_pd(res_3, support_3);
					res_4 = _mm_add_pd(res_4, support_4);
					res_5 = _mm_add_pd(res_5, support_5);

					_mm_store_pd(&(ptrResult[i]), res_0);
					_mm_store_pd(&(ptrResult[i + 2]), res_1);
					_mm_store_pd(&(ptrResult[i + 4]), res_2);
					_mm_store_pd(&(ptrResult[i + 6]), res_3);
					_mm_store_pd(&(ptrResult[i + 8]), res_4);
					_mm_store_pd(&(ptrResult[i + 10]), res_5);
				}
			}
		}
	}
#endif
#if defined(__SSE3__) && defined(__AVX__)

	for (size_t c = start_index_data; c < end_index_data;
			c += std::min<size_t>((size_t) getChunkDataPoints(),
					(end_index_data - c))) {
		size_t data_end = std::min<size_t>((size_t) getChunkDataPoints() + c,
				end_index_data);

#ifdef __ICC
#pragma ivdep
#pragma vector aligned
#endif

		for (size_t i = c; i < data_end; i++) {
			ptrResult[i] = 0.0;
		}

		for (size_t m = start_index_grid; m < end_index_grid;
				m += std::min<size_t>((size_t) getChunkGridPoints(),
						(end_index_grid - m))) {

			size_t grid_inc = std::min<size_t>((size_t) getChunkGridPoints(),
					(end_index_grid - m));

			for (size_t i = c; i < c + getChunkDataPoints(); i += 24) {
				for (size_t j = m; j < m + grid_inc; j++) {
					__m256d support_0 = _mm256_broadcast_sd(&(ptrAlpha[j]));
					__m256d support_1 = _mm256_broadcast_sd(&(ptrAlpha[j]));
					__m256d support_2 = _mm256_broadcast_sd(&(ptrAlpha[j]));
					__m256d support_3 = _mm256_broadcast_sd(&(ptrAlpha[j]));
					__m256d support_4 = _mm256_broadcast_sd(&(ptrAlpha[j]));
					__m256d support_5 = _mm256_broadcast_sd(&(ptrAlpha[j]));

					__m256d zero = _mm256_set1_pd(0.0);

					for (size_t d = 0; d < dims; d++) {
						__m256d eval_0 = _mm256_load_pd(
								&(ptrData[(d * result_size) + i]));
						__m256d eval_1 = _mm256_load_pd(
								&(ptrData[(d * result_size) + i + 4]));
						__m256d eval_2 = _mm256_load_pd(
								&(ptrData[(d * result_size) + i + 8]));
						__m256d eval_3 = _mm256_load_pd(
								&(ptrData[(d * result_size) + i + 12]));
						__m256d eval_4 = _mm256_load_pd(
								&(ptrData[(d * result_size) + i + 16]));
						__m256d eval_5 = _mm256_load_pd(
								&(ptrData[(d * result_size) + i + 20]));

						__m256d level = _mm256_broadcast_sd(
								&(ptrLevel[(j * dims) + d]));
						__m256d index = _mm256_broadcast_sd(
								&(ptrIndex[(j * dims) + d]));
#ifdef __FMA4__
						eval_0 = _mm256_msub_pd(eval_0, level, index);
						eval_1 = _mm256_msub_pd(eval_1, level, index);
						eval_2 = _mm256_msub_pd(eval_2, level, index);
						eval_3 = _mm256_msub_pd(eval_3, level, index);
						eval_4 = _mm256_msub_pd(eval_4, level, index);
						eval_5 = _mm256_msub_pd(eval_5, level, index);
#else
#ifdef __AVX2__
                                                eval_0 = _mm256_fmsub_pd(eval_0, level, index);
                                                eval_1 = _mm256_fmsub_pd(eval_1, level, index);
                                                eval_2 = _mm256_fmsub_pd(eval_2, level, index);
                                                eval_3 = _mm256_fmsub_pd(eval_3, level, index);
                                                eval_4 = _mm256_fmsub_pd(eval_4, level, index);
                                                eval_5 = _mm256_fmsub_pd(eval_5, level, index);
#else
						eval_0 = _mm256_sub_pd(_mm256_mul_pd(eval_0, level),
								index);
						eval_1 = _mm256_sub_pd(_mm256_mul_pd(eval_1, level),
								index);
						eval_2 = _mm256_sub_pd(_mm256_mul_pd(eval_2, level),
								index);
						eval_3 = _mm256_sub_pd(_mm256_mul_pd(eval_3, level),
								index);
						eval_4 = _mm256_sub_pd(_mm256_mul_pd(eval_4, level),
								index);
						eval_5 = _mm256_sub_pd(_mm256_mul_pd(eval_5, level),
								index);
#endif
#endif
						__m256d mask = _mm256_broadcast_sd(
								&(ptrMask[(j * dims) + d]));
						__m256d offset = _mm256_broadcast_sd(
								&(ptrOffset[(j * dims) + d]));

						eval_0 = _mm256_or_pd(mask, eval_0);
						eval_1 = _mm256_or_pd(mask, eval_1);
						eval_2 = _mm256_or_pd(mask, eval_2);
						eval_3 = _mm256_or_pd(mask, eval_3);
						eval_4 = _mm256_or_pd(mask, eval_4);
						eval_5 = _mm256_or_pd(mask, eval_5);

						eval_0 = _mm256_add_pd(offset, eval_0);
						eval_1 = _mm256_add_pd(offset, eval_1);
						eval_2 = _mm256_add_pd(offset, eval_2);
						eval_3 = _mm256_add_pd(offset, eval_3);
						eval_4 = _mm256_add_pd(offset, eval_4);
						eval_5 = _mm256_add_pd(offset, eval_5);

						eval_0 = _mm256_max_pd(zero, eval_0);
						eval_1 = _mm256_max_pd(zero, eval_1);
						eval_2 = _mm256_max_pd(zero, eval_2);
						eval_3 = _mm256_max_pd(zero, eval_3);
						eval_4 = _mm256_max_pd(zero, eval_4);
						eval_5 = _mm256_max_pd(zero, eval_5);

						support_0 = _mm256_mul_pd(support_0, eval_0);
						support_1 = _mm256_mul_pd(support_1, eval_1);
						support_2 = _mm256_mul_pd(support_2, eval_2);
						support_3 = _mm256_mul_pd(support_3, eval_3);
						support_4 = _mm256_mul_pd(support_4, eval_4);
						support_5 = _mm256_mul_pd(support_5, eval_5);
					}

					__m256d res_0 = _mm256_load_pd(&(ptrResult[i]));
					__m256d res_1 = _mm256_load_pd(&(ptrResult[i + 4]));
					__m256d res_2 = _mm256_load_pd(&(ptrResult[i + 8]));
					__m256d res_3 = _mm256_load_pd(&(ptrResult[i + 12]));
					__m256d res_4 = _mm256_load_pd(&(ptrResult[i + 16]));
					__m256d res_5 = _mm256_load_pd(&(ptrResult[i + 20]));

					res_0 = _mm256_add_pd(res_0, support_0);
					res_1 = _mm256_add_pd(res_1, support_1);
					res_2 = _mm256_add_pd(res_2, support_2);
					res_3 = _mm256_add_pd(res_3, support_3);
					res_4 = _mm256_add_pd(res_4, support_4);
					res_5 = _mm256_add_pd(res_5, support_5);

					_mm256_store_pd(&(ptrResult[i]), res_0);
					_mm256_store_pd(&(ptrResult[i + 4]), res_1);
					_mm256_store_pd(&(ptrResult[i + 8]), res_2);
					_mm256_store_pd(&(ptrResult[i + 12]), res_3);
					_mm256_store_pd(&(ptrResult[i + 16]), res_4);
					_mm256_store_pd(&(ptrResult[i + 20]), res_5);
				}
			}
		}
	}
#endif

/*
#ifdef __MIC__

#define STREAMING_MODLINEAR_STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH 24

	for (size_t i = start_index_data; i < end_index_data; i +=
			getChunkDataPoints()) {

		for (size_t j = start_index_grid; j < end_index_grid; j++) {
			__m512d support_0 = _mm512_extload_pd(&(ptrAlpha[j]),
					_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
			__m512d support_1 = _mm512_extload_pd(&(ptrAlpha[j]),
					_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
			__m512d support_2 = _mm512_extload_pd(&(ptrAlpha[j]),
					_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);

			for (size_t d = 0; d < dims - 1; d++) {
				__m512d eval_0 = _mm512_load_pd(
						&(ptrData[(d * result_size) + i + 0]));
				__m512d eval_1 = _mm512_load_pd(
						&(ptrData[(d * result_size) + i + 8]));
				__m512d eval_2 = _mm512_load_pd(
						&(ptrData[(d * result_size) + i + 16]));

				__m512d level = _mm512_extload_pd(&(ptrLevel[(j * dims) + d]),
						_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
				__m512d index = _mm512_extload_pd(&(ptrIndex[(j * dims) + d]),
						_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);

				eval_0 = _mm512_fmsub_pd(eval_0, level, index);
				eval_1 = _mm512_fmsub_pd(eval_1, level, index);
				eval_2 = _mm512_fmsub_pd(eval_2, level, index);

				__m512d mask = _mm512_extload_pd(&(ptrMask[(j * dims) + d]),
						_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
				__m512d offset = _mm512_extload_pd(&(ptrOffset[(j * dims) + d]),
						_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);

				eval_0 = _mm512_castsi512_pd(
						_mm512_or_epi64(_mm512_castpd_si512(mask),
								_mm512_castpd_si512(eval_0)));
				eval_1 = _mm512_castsi512_pd(
						_mm512_or_epi64(_mm512_castpd_si512(mask),
								_mm512_castpd_si512(eval_1)));
				eval_2 = _mm512_castsi512_pd(
						_mm512_or_epi64(_mm512_castpd_si512(mask),
								_mm512_castpd_si512(eval_2)));

				__m512d zero = _mm512_set_1to8_pd(0.0);

				eval_0 = _mm512_add_pd(offset, eval_0);
				eval_1 = _mm512_add_pd(offset, eval_1);
				eval_2 = _mm512_add_pd(offset, eval_2);

				eval_0 = _mm512_gmax_pd(zero, eval_0);
				eval_1 = _mm512_gmax_pd(zero, eval_1);
				eval_2 = _mm512_gmax_pd(zero, eval_2);

				support_0 = _mm512_mul_pd(support_0, eval_0);
				support_1 = _mm512_mul_pd(support_1, eval_1);
				support_2 = _mm512_mul_pd(support_2, eval_2);
			}

			size_t d = dims - 1;

			__m512d eval_0 = _mm512_load_pd(
					&(ptrData[(d * result_size) + i + 0]));
			__m512d eval_1 = _mm512_load_pd(
					&(ptrData[(d * result_size) + i + 8]));
			__m512d eval_2 = _mm512_load_pd(
					&(ptrData[(d * result_size) + i + 16]));

			__m512d level = _mm512_extload_pd(&(ptrLevel[(j * dims) + d]),
					_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
			__m512d index = _mm512_extload_pd(&(ptrIndex[(j * dims) + d]),
					_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);

			eval_0 = _mm512_fmsub_pd(eval_0, level, index);
			eval_1 = _mm512_fmsub_pd(eval_1, level, index);
			eval_2 = _mm512_fmsub_pd(eval_2, level, index);

			__m512d mask = _mm512_extload_pd(&(ptrMask[(j * dims) + d]),
					_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
			__m512d offset = _mm512_extload_pd(&(ptrOffset[(j * dims) + d]),
					_MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);

			eval_0 = _mm512_castsi512_pd(
					_mm512_or_epi64(_mm512_castpd_si512(mask),
							_mm512_castpd_si512(eval_0)));
			eval_1 = _mm512_castsi512_pd(
					_mm512_or_epi64(_mm512_castpd_si512(mask),
							_mm512_castpd_si512(eval_1)));
			eval_2 = _mm512_castsi512_pd(
					_mm512_or_epi64(_mm512_castpd_si512(mask),
							_mm512_castpd_si512(eval_2)));

			__m512d zero = _mm512_set_1to8_pd(0.0);

			eval_0 = _mm512_add_pd(offset, eval_0);
			eval_1 = _mm512_add_pd(offset, eval_1);
			eval_2 = _mm512_add_pd(offset, eval_2);

			eval_0 = _mm512_gmax_pd(zero, eval_0);
			eval_1 = _mm512_gmax_pd(zero, eval_1);
			eval_2 = _mm512_gmax_pd(zero, eval_2);

			__m512d res_0 = _mm512_load_pd(&(ptrResult[i + 0]));
			__m512d res_1 = _mm512_load_pd(&(ptrResult[i + 8]));
			__m512d res_2 = _mm512_load_pd(&(ptrResult[i + 16]));

			res_0 = _mm512_add_pd(res_0, _mm512_mul_pd(support_0, eval_0));
			res_1 = _mm512_add_pd(res_1, _mm512_mul_pd(support_1, eval_1));
			res_2 = _mm512_add_pd(res_2, _mm512_mul_pd(support_2, eval_2));

			_mm512_store_pd(&(ptrResult[i + 0]), res_0);
			_mm512_store_pd(&(ptrResult[i + 8]), res_1);
			_mm512_store_pd(&(ptrResult[i + 16]), res_2);
		}
	}
#endif
*/



#ifdef __MIC__

    for (size_t i = start_index_data; i < end_index_data; i += getChunkDataPoints()) {
      for (size_t j = start_index_grid; j < end_index_grid; j++) {
        __m512d support_0 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
        __m512d support_1 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
        __m512d support_2 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
        __m512d support_3 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
        __m512d support_4 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
        __m512d support_5 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
        __m512d support_6 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
        __m512d support_7 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
        __m512d support_8 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
        __m512d support_9 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
        __m512d support_10 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
        __m512d support_11 = _mm512_extload_pd(&(ptrAlpha[j]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
#endif

        for (size_t d = 0; d < dims; d++) {
          __m512d eval_0 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 0]));
          __m512d eval_1 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 8]));
          __m512d eval_2 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 16]));
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
          __m512d eval_3 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 24]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
          __m512d eval_4 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 32]));
          __m512d eval_5 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 40]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
          __m512d eval_6 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 48]));
          __m512d eval_7 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 56]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
          __m512d eval_8 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 64]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
          __m512d eval_9 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 72]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
          __m512d eval_10 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 80]));
          __m512d eval_11 = _mm512_load_pd(&(ptrData[(d * result_size) + i + 88]));
#endif

          __m512d level = _mm512_extload_pd(&(ptrLevel[(j * dims) + d]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
          __m512d index = _mm512_extload_pd(&(ptrIndex[(j * dims) + d]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);

          eval_0 = _mm512_fmsub_pd(eval_0, level, index);
          eval_1 = _mm512_fmsub_pd(eval_1, level, index);
          eval_2 = _mm512_fmsub_pd(eval_2, level, index);
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
          eval_3 = _mm512_fmsub_pd(eval_3, level, index);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
          eval_4 = _mm512_fmsub_pd(eval_4, level, index);
          eval_5 = _mm512_fmsub_pd(eval_5, level, index);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
          eval_6 = _mm512_fmsub_pd(eval_6, level, index);
          eval_7 = _mm512_fmsub_pd(eval_7, level, index);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
          eval_8 = _mm512_fmsub_pd(eval_8, level, index);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
          eval_9 = _mm512_fmsub_pd(eval_9, level, index);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
          eval_10 = _mm512_fmsub_pd(eval_10, level, index);
          eval_11 = _mm512_fmsub_pd(eval_11, level, index);
#endif

          __m512d mask = _mm512_extload_pd(&(ptrMask[(j * dims) + d]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
          __m512d offset = _mm512_extload_pd(&(ptrOffset[(j * dims) + d]), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);

          eval_0 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_0)));
          eval_1 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_1)));
          eval_2 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_2)));
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
          eval_3 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_3)));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
          eval_4 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_4)));
          eval_5 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_5)));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
          eval_6 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_6)));
          eval_7 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_7)));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
          eval_8 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_8)));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
          eval_9 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_9)));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
          eval_10 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_10)));
          eval_11 = _mm512_castsi512_pd(_mm512_or_epi64( _mm512_castpd_si512(mask), _mm512_castpd_si512(eval_11)));
#endif

          __m512d zero = _mm512_set_1to8_pd(0.0);

          eval_0 = _mm512_add_pd(offset, eval_0);
          eval_1 = _mm512_add_pd(offset, eval_1);
          eval_2 = _mm512_add_pd(offset, eval_2);
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
          eval_3 = _mm512_add_pd(offset, eval_3);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
          eval_4 = _mm512_add_pd(offset, eval_4);
          eval_5 = _mm512_add_pd(offset, eval_5);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
          eval_6 = _mm512_add_pd(offset, eval_6);
          eval_7 = _mm512_add_pd(offset, eval_7);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
          eval_8 = _mm512_add_pd(offset, eval_8);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
          eval_9 = _mm512_add_pd(offset, eval_9);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
          eval_10 = _mm512_add_pd(offset, eval_10);
          eval_11 = _mm512_add_pd(offset, eval_11);
#endif

          eval_0 = _mm512_gmax_pd(zero, eval_0);
          eval_1 = _mm512_gmax_pd(zero, eval_1);
          eval_2 = _mm512_gmax_pd(zero, eval_2);
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
          eval_3 = _mm512_gmax_pd(zero, eval_3);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
          eval_4 = _mm512_gmax_pd(zero, eval_4);
          eval_5 = _mm512_gmax_pd(zero, eval_5);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
          eval_6 = _mm512_gmax_pd(zero, eval_6);
          eval_7 = _mm512_gmax_pd(zero, eval_7);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
          eval_8 = _mm512_gmax_pd(zero, eval_8);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
          eval_9 = _mm512_gmax_pd(zero, eval_9);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
          eval_10 = _mm512_gmax_pd(zero, eval_10);
          eval_11 = _mm512_gmax_pd(zero, eval_11);
#endif

          support_0 = _mm512_mul_pd(support_0, eval_0);
          support_1 = _mm512_mul_pd(support_1, eval_1);
          support_2 = _mm512_mul_pd(support_2, eval_2);
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
          support_3 = _mm512_mul_pd(support_3, eval_3);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
          support_4 = _mm512_mul_pd(support_4, eval_4);
          support_5 = _mm512_mul_pd(support_5, eval_5);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
          support_6 = _mm512_mul_pd(support_6, eval_6);
          support_7 = _mm512_mul_pd(support_7, eval_7);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
          support_8 = _mm512_mul_pd(support_8, eval_8);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
          support_9 = _mm512_mul_pd(support_9, eval_9);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
          support_10 = _mm512_mul_pd(support_10, eval_10);
          support_11 = _mm512_mul_pd(support_11, eval_11);
#endif
        }

        __m512d res_0 = _mm512_load_pd(&(ptrResult[i + 0]));
        __m512d res_1 = _mm512_load_pd(&(ptrResult[i + 8]));
        __m512d res_2 = _mm512_load_pd(&(ptrResult[i + 16]));
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
        __m512d res_3 = _mm512_load_pd(&(ptrResult[i + 24]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
        __m512d res_4 = _mm512_load_pd(&(ptrResult[i + 32]));
        __m512d res_5 = _mm512_load_pd(&(ptrResult[i + 40]));
#endif

        res_0 = _mm512_add_pd(res_0, support_0);
        res_1 = _mm512_add_pd(res_1, support_1);
        res_2 = _mm512_add_pd(res_2, support_2);
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
        res_3 = _mm512_add_pd(res_3, support_3);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
        res_4 = _mm512_add_pd(res_4, support_4);
        res_5 = _mm512_add_pd(res_5, support_5);
#endif

        _mm512_store_pd(&(ptrResult[i + 0]), res_0);
        _mm512_store_pd(&(ptrResult[i + 8]), res_1);
        _mm512_store_pd(&(ptrResult[i + 16]), res_2);
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  24 )
        _mm512_store_pd(&(ptrResult[i + 24]), res_3);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  32 )
        _mm512_store_pd(&(ptrResult[i + 32]), res_4);
        _mm512_store_pd(&(ptrResult[i + 40]), res_5);
#endif

#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
        __m512d res_6 = _mm512_load_pd(&(ptrResult[i + 48]));
        __m512d res_7 = _mm512_load_pd(&(ptrResult[i + 56]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
        __m512d res_8 = _mm512_load_pd(&(ptrResult[i + 64]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
        __m512d res_9 = _mm512_load_pd(&(ptrResult[i + 72]));
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
        __m512d res_10 = _mm512_load_pd(&(ptrResult[i + 80]));
        __m512d res_11 = _mm512_load_pd(&(ptrResult[i + 88]));
#endif

#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
        res_6 = _mm512_add_pd(res_6, support_6);
        res_7 = _mm512_add_pd(res_7, support_7);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
        res_8 = _mm512_add_pd(res_8, support_8);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
        res_9 = _mm512_add_pd(res_9, support_9);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
        res_10 = _mm512_add_pd(res_10, support_10);
        res_11 = _mm512_add_pd(res_11, support_11);
#endif

#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  48 )
        _mm512_store_pd(&(ptrResult[i + 48]), res_6);
        _mm512_store_pd(&(ptrResult[i + 56]), res_7);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  64 )
        _mm512_store_pd(&(ptrResult[i + 64]), res_8);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  72 )
        _mm512_store_pd(&(ptrResult[i + 72]), res_9);
#endif
#if  ( STREAMING_MODLINEAR_MIC_UNROLLING_WIDTH >  80 )
        _mm512_store_pd(&(ptrResult[i + 80]), res_10);
        _mm512_store_pd(&(ptrResult[i + 88]), res_11);
#endif
      }
    }
#endif


#if !defined(__SSE3__) && !defined(__AVX__) && !defined(__MIC__)
#warning "warning: using fallback implementation for OperationMultiEvalModMaskStreaming mult kernel"

	for (size_t c = start_index_data; c < end_index_data;
			c += std::min<size_t>((size_t) getChunkDataPoints(),
					(end_index_data - c))) {
		size_t data_end = std::min<size_t>((size_t) getChunkDataPoints() + c,
				end_index_data);

#ifdef __ICC
#pragma ivdep
#pragma vector aligned
#endif

		for (size_t i = c; i < data_end; i++) {
			ptrResult[i] = 0.0;
		}

		for (size_t m = start_index_grid; m < end_index_grid;
				m += std::min<size_t>((size_t) getChunkGridPoints(),
						(end_index_grid - m))) {

			size_t grid_end = std::min<size_t>(
					(size_t) getChunkGridPoints() + m, end_index_grid);

			for (size_t i = c; i < data_end; i++) {
				for (size_t j = m; j < grid_end; j++) {
					double curSupport = ptrAlpha[j];

					for (size_t d = 0; d < dims; d++) {
						double eval = ((ptrLevel[(j * dims) + d])
								* (ptrData[(d * result_size) + i]))
						- (ptrIndex[(j * dims) + d]);
						uint64_t maskresult =
						*reinterpret_cast<uint64_t*>(&eval)
						| *reinterpret_cast<uint64_t*>(&(ptrMask[(j
												* dims) + d]));
						double masking = *reinterpret_cast<double*>(&maskresult);
						double last = masking + ptrOffset[(j * dims) + d];
						double localSupport = std::max<double>(last, 0.0);
						curSupport *= localSupport;
					}

					ptrResult[i] += curSupport;
				}
			}
		}
	}
#endif

#else /* USE_DOUBLE_PRECISION */
	throw std::logic_error("Not implemented when compiling with single "
			"precision support.");
#endif /* USE_DOUBLE_PRECISION */
}

}
}
