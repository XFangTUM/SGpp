// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/datadriven/operation/hash/OperationMultiEvalStreaming/OperationMultiEvalStreaming.hpp>

#include <sgpp/globaldef.hpp>

#if defined(__SSE3__) && !defined(__AVX__) && USE_DOUBLE_PRECISION==1
#include <pmmintrin.h>
#endif

#if defined(__SSE3__) && defined(__AVX__) && USE_DOUBLE_PRECISION==1
#include <immintrin.h>
#endif

#ifdef __MIC__
#include <immintrin.h>
#endif

namespace SGPP {
namespace datadriven {

void OperationMultiEvalStreaming::multTransposeImpl(
SGPP::base::DataMatrix* level,
SGPP::base::DataMatrix* index,
SGPP::base::DataMatrix* dataset, SGPP::base::DataVector& source,
SGPP::base::DataVector& result, const size_t start_index_grid,
		const size_t end_index_grid, const size_t start_index_data,
		const size_t end_index_data) {
	float_t* ptrLevel = level->getPointer();
	float_t* ptrIndex = index->getPointer();
	float_t* ptrSource = source.getPointer();
	float_t* ptrData = dataset->getPointer();
	float_t* ptrResult = result.getPointer();
	size_t sourceSize = source.getSize();
	size_t dims = dataset->getNrows();

#if defined(__SSE3__) && !defined(__AVX__)

	for (size_t k = start_index_grid; k < end_index_grid;
			k += std::min<size_t>(getChunkGridPoints(), (end_index_grid - k))) {
		size_t grid_inc = std::min<size_t>((size_t) getChunkGridPoints(),
				(end_index_grid - k));

		long long imask = 0x7FFFFFFFFFFFFFFF;
		float_t* fmask = (float_t*) &imask;

		for (size_t i = start_index_data; i < end_index_data; i += 12) {
			for (size_t j = k; j < k + grid_inc; j++) {
				__m128d support_0 = _mm_load_pd(&(ptrSource[i]));
				__m128d support_1 = _mm_load_pd(&(ptrSource[i + 2]));
				__m128d support_2 = _mm_load_pd(&(ptrSource[i + 4]));
				__m128d support_3 = _mm_load_pd(&(ptrSource[i + 6]));
				__m128d support_4 = _mm_load_pd(&(ptrSource[i + 8]));
				__m128d support_5 = _mm_load_pd(&(ptrSource[i + 10]));

				__m128d mask = _mm_set1_pd(*fmask);
				__m128d one = _mm_set1_pd(1.0);
				__m128d zero = _mm_set1_pd(0.0);

				for (size_t d = 0; d < dims; d++) {
					__m128d eval_0 = _mm_load_pd(
							&(ptrData[(d * sourceSize) + i]));
					__m128d eval_1 = _mm_load_pd(
							&(ptrData[(d * sourceSize) + i + 2]));
					__m128d eval_2 = _mm_load_pd(
							&(ptrData[(d * sourceSize) + i + 4]));
					__m128d eval_3 = _mm_load_pd(
							&(ptrData[(d * sourceSize) + i + 6]));
					__m128d eval_4 = _mm_load_pd(
							&(ptrData[(d * sourceSize) + i + 8]));
					__m128d eval_5 = _mm_load_pd(
							&(ptrData[(d * sourceSize) + i + 10]));

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
					eval_0 = _mm_and_pd(mask, eval_0);
					eval_1 = _mm_and_pd(mask, eval_1);
					eval_2 = _mm_and_pd(mask, eval_2);
					eval_3 = _mm_and_pd(mask, eval_3);
					eval_4 = _mm_and_pd(mask, eval_4);
					eval_5 = _mm_and_pd(mask, eval_5);

					eval_0 = _mm_sub_pd(one, eval_0);
					eval_1 = _mm_sub_pd(one, eval_1);
					eval_2 = _mm_sub_pd(one, eval_2);
					eval_3 = _mm_sub_pd(one, eval_3);
					eval_4 = _mm_sub_pd(one, eval_4);
					eval_5 = _mm_sub_pd(one, eval_5);

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

				__m128d res_0 = _mm_setzero_pd();
				res_0 = _mm_loadl_pd(res_0, &(ptrResult[j]));

				support_0 = _mm_add_pd(support_0, support_1);
				support_2 = _mm_add_pd(support_2, support_3);
				support_4 = _mm_add_pd(support_4, support_5);
				support_0 = _mm_add_pd(support_0, support_2);
				support_0 = _mm_add_pd(support_0, support_4);

				support_0 = _mm_hadd_pd(support_0, support_0);
				res_0 = _mm_add_sd(res_0, support_0);

				_mm_storel_pd(&(ptrResult[j]), res_0);
			}
		}
	}
}
#endif

#if defined(__SSE3__) && defined(__AVX__)

	for (size_t k = start_index_grid; k < end_index_grid;
			k += std::min<size_t>(getChunkGridPoints(), (end_index_grid - k))) {
		size_t grid_inc = std::min<size_t>((size_t) getChunkGridPoints(),
				(end_index_grid - k));

		int64_t imask = 0x7FFFFFFFFFFFFFFF;
		float_t* fmask = (float_t*) &imask;

		for (size_t i = start_index_data; i < end_index_data; i += 24) {
			for (size_t j = k; j < k + grid_inc; j++) {
				__m256d support_0 = _mm256_load_pd(&(ptrSource[i]));
				__m256d support_1 = _mm256_load_pd(&(ptrSource[i + 4]));
				__m256d support_2 = _mm256_load_pd(&(ptrSource[i + 8]));
				__m256d support_3 = _mm256_load_pd(&(ptrSource[i + 12]));
				__m256d support_4 = _mm256_load_pd(&(ptrSource[i + 16]));
				__m256d support_5 = _mm256_load_pd(&(ptrSource[i + 20]));

				__m256d mask = _mm256_broadcast_sd(fmask);
				__m256d one = _mm256_set1_pd(1.0);
				__m256d zero = _mm256_set1_pd(0.0);

				for (size_t d = 0; d < dims; d++) {
					__m256d eval_0 = _mm256_load_pd(
							&(ptrData[(d * sourceSize) + i]));
					__m256d eval_1 = _mm256_load_pd(
							&(ptrData[(d * sourceSize) + i + 4]));
					__m256d eval_2 = _mm256_load_pd(
							&(ptrData[(d * sourceSize) + i + 8]));
					__m256d eval_3 = _mm256_load_pd(
							&(ptrData[(d * sourceSize) + i + 12]));
					__m256d eval_4 = _mm256_load_pd(
							&(ptrData[(d * sourceSize) + i + 16]));
					__m256d eval_5 = _mm256_load_pd(
							&(ptrData[(d * sourceSize) + i + 20]));

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
					eval_0 = _mm256_sub_pd(_mm256_mul_pd(eval_0, level), index);
					eval_1 = _mm256_sub_pd(_mm256_mul_pd(eval_1, level), index);
					eval_2 = _mm256_sub_pd(_mm256_mul_pd(eval_2, level), index);
					eval_3 = _mm256_sub_pd(_mm256_mul_pd(eval_3, level), index);
					eval_4 = _mm256_sub_pd(_mm256_mul_pd(eval_4, level), index);
					eval_5 = _mm256_sub_pd(_mm256_mul_pd(eval_5, level), index);
#endif
					eval_0 = _mm256_and_pd(mask, eval_0);
					eval_1 = _mm256_and_pd(mask, eval_1);
					eval_2 = _mm256_and_pd(mask, eval_2);
					eval_3 = _mm256_and_pd(mask, eval_3);
					eval_4 = _mm256_and_pd(mask, eval_4);
					eval_5 = _mm256_and_pd(mask, eval_5);

					eval_0 = _mm256_sub_pd(one, eval_0);
					eval_1 = _mm256_sub_pd(one, eval_1);
					eval_2 = _mm256_sub_pd(one, eval_2);
					eval_3 = _mm256_sub_pd(one, eval_3);
					eval_4 = _mm256_sub_pd(one, eval_4);
					eval_5 = _mm256_sub_pd(one, eval_5);

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

				const __m256i ldStMaskAVX = _mm256_set_epi64x(
						0x0000000000000000, 0x0000000000000000,
						0x0000000000000000, 0xFFFFFFFFFFFFFFFF);

				support_0 = _mm256_add_pd(support_0, support_1);
				support_2 = _mm256_add_pd(support_2, support_3);
				support_4 = _mm256_add_pd(support_4, support_5);
				support_0 = _mm256_add_pd(support_0, support_2);
				support_0 = _mm256_add_pd(support_0, support_4);

				support_0 = _mm256_hadd_pd(support_0, support_0);
				__m256d tmp = _mm256_permute2f128_pd(support_0, support_0,
						0x81);
				support_0 = _mm256_add_pd(support_0, tmp);

				// Workaround: bug with maskload in GCC (4.6.1)
#ifdef __ICC
				__m256d res_0 = _mm256_maskload_pd(&(ptrResult[j]), ldStMaskAVX);
				res_0 = _mm256_add_pd(res_0, support_0);
				_mm256_maskstore_pd(&(ptrResult[j]), ldStMaskAVX, res_0);
#else
				float_t tmp_reduce;
				_mm256_maskstore_pd(&(tmp_reduce), ldStMaskAVX, support_0);
				ptrResult[j] += tmp_reduce;
#endif
			}
		}
	}
}
#endif

#ifdef __MIC__

#define MIC_UNROLLING_WIDTH 24

for (size_t i = start_index_data; i < end_index_data; i +=
		getChunkDataPoints()) {
	for (size_t j = start_index_grid; j < end_index_grid; j++) {
		__m512d support_0 = _mm512_load_pd(&(ptrSource[i + 0]));
		__m512d support_1 = _mm512_load_pd(&(ptrSource[i + 8]));
		__m512d support_2 = _mm512_load_pd(&(ptrSource[i + 16]));

		_mm_prefetch((const char* ) &(ptrLevel[((j + 1) * dims)]),
				_MM_HINT_T0);
		_mm_prefetch((const char* ) &(ptrIndex[((j + 1) * dims)]),
				_MM_HINT_T0);
		// Prefetch exclusive -> non sync is needed
		_mm_prefetch((const char* ) &(ptrResult[j]), _MM_HINT_ET0);

		//basis function evaluation
		for (size_t d = 0; d < dims; d++) {
			__m512d eval_0 = _mm512_load_pd(
					&(ptrData[(d * sourceSize) + i + 0]));
			__m512d eval_1 = _mm512_load_pd(
					&(ptrData[(d * sourceSize) + i + 8]));
			__m512d eval_2 = _mm512_load_pd(
					&(ptrData[(d * sourceSize) + i + 16]));

			__m512d level = _mm512_extload_pd(
					&(ptrLevel[(j * dims) + d]), _MM_UPCONV_PD_NONE,
					_MM_BROADCAST_1X8, _MM_HINT_NONE);
			__m512d index = _mm512_extload_pd(
					&(ptrIndex[(j * dims) + d]), _MM_UPCONV_PD_NONE,
					_MM_BROADCAST_1X8, _MM_HINT_NONE);

			eval_0 = _mm512_fmsub_pd(eval_0, level, index);
			eval_1 = _mm512_fmsub_pd(eval_1, level, index);
			eval_2 = _mm512_fmsub_pd(eval_2, level, index);

			__m512d one = _mm512_set_1to8_pd(1.0);
			__m512d zero = _mm512_set_1to8_pd(0.0);
			__m512i abs2MaskLRBni = _mm512_set_1to8_epi64(
					0x7FFFFFFFFFFFFFFF);

			eval_0 = _mm512_castsi512_pd(
					_mm512_and_epi64(abs2MaskLRBni,
							_mm512_castpd_si512(eval_0)));
			eval_1 = _mm512_castsi512_pd(
					_mm512_and_epi64(abs2MaskLRBni,
							_mm512_castpd_si512(eval_1)));
			eval_2 = _mm512_castsi512_pd(
					_mm512_and_epi64(abs2MaskLRBni,
							_mm512_castpd_si512(eval_2)));

			eval_0 = _mm512_sub_pd(one, eval_0);
			eval_1 = _mm512_sub_pd(one, eval_1);
			eval_2 = _mm512_sub_pd(one, eval_2);

			eval_0 = _mm512_gmax_pd(zero, eval_0);
			eval_1 = _mm512_gmax_pd(zero, eval_1);
			eval_2 = _mm512_gmax_pd(zero, eval_2);

			support_0 = _mm512_mul_pd(support_0, eval_0);
			support_1 = _mm512_mul_pd(support_1, eval_1);
			support_2 = _mm512_mul_pd(support_2, eval_2);

		}

		support_0 = _mm512_add_pd(support_0, support_1);
		support_0 = _mm512_add_pd(support_0, support_2);

		ptrResult[j] += _mm512_reduce_add_pd(support_0);
	}
}
}
#endif

#if (!defined(__SSE3__) && !defined(__AVX__)) && !defined(__MIC__)
#warning "warning: using fallback implementation for OperationMultiEvalStreaming multTranspose kernel"

for (size_t k = start_index_grid; k < end_index_grid;
	k += std::min<size_t>(getChunkGridPoints(), (end_index_grid - k))) {
size_t grid_inc = std::min<size_t>((size_t) getChunkGridPoints(),
		(end_index_grid - k));

for (size_t i = start_index_data; i < end_index_data; i++) {
	for (size_t j = k; j < k + grid_inc; j++) {
		float_t curSupport = ptrSource[i];

		for (size_t d = 0; d < dims; d++) {
			float_t eval = ((ptrLevel[(j * dims) + d])
					* (ptrData[(d * sourceSize) + i]));
			float_t index_calc = eval - (ptrIndex[(j * dims) + d]);
			float_t abs = fabs(index_calc);
			float_t last = 1.0 - abs;
			float_t localSupport = std::max<float_t>(last, 0.0);
			curSupport *= localSupport;
		}

		ptrResult[j] += curSupport;
	}
}

}
}
#endif
//      }
//    }

}
}
