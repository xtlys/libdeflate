/*
 * gzip_decompress.c - decompress with a gzip wrapper
 *
 * Copyright 2016 Eric Biggers
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "lib_common.h"
#include "timing_internal.h"
#include "gzip_constants.h"

LIBDEFLATEAPI enum libdeflate_result
libdeflate_gzip_decompress_ex(struct libdeflate_decompressor *d,
                              const void *in, size_t in_nbytes,
                              void *out, size_t out_nbytes_avail,
                              size_t *actual_in_nbytes_ret,
                              size_t *actual_out_nbytes_ret)
{
        const u8 *in_next = in;
        const u8 * const in_end = in_next + in_nbytes;
        u8 flg;
        size_t actual_in_nbytes;
        size_t actual_out_nbytes;
        enum libdeflate_result result = LIBDEFLATE_BAD_DATA;
        uint64_t wrapper_start;
        uint64_t checksum_start;
        u32 computed_crc;
        u32 expected_isize;

        libdeflate_timing_begin(d);
        wrapper_start = libdeflate_timing_section_begin();

        if (in_nbytes < GZIP_MIN_OVERHEAD)
                goto out;

        /* ID1 */
        if (*in_next++ != GZIP_ID1)
                goto out;
        /* ID2 */
        if (*in_next++ != GZIP_ID2)
                goto out;
        /* CM */
        if (*in_next++ != GZIP_CM_DEFLATE)
                goto out;
        flg = *in_next++;
        /* MTIME */
        in_next += 4;
        /* XFL */
        in_next += 1;
        /* OS */
        in_next += 1;

        if (flg & GZIP_FRESERVED)
                goto out;

        /* Extra field */
        if (flg & GZIP_FEXTRA) {
                u16 xlen = get_unaligned_le16(in_next);
                in_next += 2;

                if (in_end - in_next < (u32)xlen + GZIP_FOOTER_SIZE)
                        goto out;

                in_next += xlen;
        }

        /* Original file name (zero terminated) */
        if (flg & GZIP_FNAME) {
                while (*in_next++ != 0 && in_next != in_end)
                        ;
                if (in_end - in_next < GZIP_FOOTER_SIZE)
                        goto out;
        }

        /* File comment (zero terminated) */
        if (flg & GZIP_FCOMMENT) {
                while (*in_next++ != 0 && in_next != in_end)
                        ;
                if (in_end - in_next < GZIP_FOOTER_SIZE)
                        goto out;
        }

        /* CRC16 for gzip header */
        if (flg & GZIP_FHCRC) {
                in_next += 2;
                if (in_end - in_next < GZIP_FOOTER_SIZE)
                        goto out;
        }

        libdeflate_timing_wrapper_end(d, wrapper_start, true);

        /* Compressed data  */
        result = libdeflate_deflate_decompress_ex(d, in_next,
                                        in_end - GZIP_FOOTER_SIZE - in_next,
                                        out, out_nbytes_avail,
                                        &actual_in_nbytes,
                                        actual_out_nbytes_ret);
        if (result != LIBDEFLATE_SUCCESS)
                goto out;

        if (actual_out_nbytes_ret)
                actual_out_nbytes = *actual_out_nbytes_ret;
        else
                actual_out_nbytes = out_nbytes_avail;

        wrapper_start = libdeflate_timing_section_begin();
        in_next += actual_in_nbytes;
        libdeflate_timing_wrapper_end(d, wrapper_start, false);

        /* CRC32 */
        checksum_start = libdeflate_timing_checksum_begin();
        computed_crc = libdeflate_crc32(0, out, actual_out_nbytes);
        libdeflate_timing_checksum_end(d, checksum_start);
        if (computed_crc != get_unaligned_le32(in_next))
                goto out;
        in_next += 4;

        /* ISIZE and trailer */
        wrapper_start = libdeflate_timing_section_begin();
        expected_isize = get_unaligned_le32(in_next);
        if ((u32)actual_out_nbytes != expected_isize) {
                libdeflate_timing_wrapper_end(d, wrapper_start, false);
                goto out;
        }
        in_next += 4;
        if (actual_in_nbytes_ret)
                *actual_in_nbytes_ret = in_next - (u8 *)in;
        libdeflate_timing_wrapper_end(d, wrapper_start, false);

        result = LIBDEFLATE_SUCCESS;

out:
        libdeflate_timing_set_checksum_done(d);
        libdeflate_timing_finish(d);
        return result;
}

LIBDEFLATEAPI enum libdeflate_result
libdeflate_gzip_decompress(struct libdeflate_decompressor *d,
			   const void *in, size_t in_nbytes,
			   void *out, size_t out_nbytes_avail,
			   size_t *actual_out_nbytes_ret)
{
	return libdeflate_gzip_decompress_ex(d, in, in_nbytes,
					     out, out_nbytes_avail,
					     NULL, actual_out_nbytes_ret);
}
