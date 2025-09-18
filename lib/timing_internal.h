#ifndef LIB_TIMING_INTERNAL_H
#define LIB_TIMING_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

struct libdeflate_decompressor;

#ifdef LIBDEFLATE_ENABLE_RDTSC_TIMING
uint64_t libdeflate_timing_section_begin(void);
void libdeflate_timing_begin(struct libdeflate_decompressor *d);
void libdeflate_timing_wrapper_end(struct libdeflate_decompressor *d,
                                   uint64_t start, bool record_timestamp);
void libdeflate_timing_core_begin(struct libdeflate_decompressor *d);
void libdeflate_timing_core_end(struct libdeflate_decompressor *d);
uint64_t libdeflate_timing_checksum_begin(void);
void libdeflate_timing_checksum_end(struct libdeflate_decompressor *d,
                                    uint64_t start);
void libdeflate_timing_set_checksum_done(struct libdeflate_decompressor *d);
void libdeflate_timing_finish(struct libdeflate_decompressor *d);
#else
static inline uint64_t libdeflate_timing_section_begin(void)
{
        return 0;
}
static inline void libdeflate_timing_begin(struct libdeflate_decompressor *d)
{
        (void)d;
}
static inline void
libdeflate_timing_wrapper_end(struct libdeflate_decompressor *d,
                              uint64_t start, bool record_timestamp)
{
        (void)d;
        (void)start;
        (void)record_timestamp;
}
static inline void libdeflate_timing_core_begin(struct libdeflate_decompressor *d)
{
        (void)d;
}
static inline void libdeflate_timing_core_end(struct libdeflate_decompressor *d)
{
        (void)d;
}
static inline uint64_t libdeflate_timing_checksum_begin(void)
{
        return 0;
}
static inline void
libdeflate_timing_checksum_end(struct libdeflate_decompressor *d,
                               uint64_t start)
{
        (void)d;
        (void)start;
}
static inline void
libdeflate_timing_set_checksum_done(struct libdeflate_decompressor *d)
{
        (void)d;
}
static inline void libdeflate_timing_finish(struct libdeflate_decompressor *d)
{
        (void)d;
}
#endif

#endif /* LIB_TIMING_INTERNAL_H */
