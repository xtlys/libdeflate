#ifndef LIB_TIMING_X86_H
#define LIB_TIMING_X86_H

#if defined(LIBDEFLATE_ENABLE_RDTSC_TIMING) && (defined(__x86_64__) || defined(_M_X64))
#include <stdint.h>
#include <x86intrin.h>

static inline uint64_t libdeflate_rdtsc_begin(void)
{
        _mm_lfence();
        return __rdtsc();
}

static inline uint64_t libdeflate_rdtsc_end(unsigned int *aux)
{
        unsigned int aux_local;
        uint64_t tsc = __rdtscp(&aux_local);
        _mm_lfence();
        if (aux)
                *aux = aux_local;
        return tsc;
}

#endif /* defined(LIBDEFLATE_ENABLE_RDTSC_TIMING) && x86_64 */

#endif /* LIB_TIMING_X86_H */
