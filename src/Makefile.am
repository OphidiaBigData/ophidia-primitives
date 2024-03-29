#
#    Ophidia Primitives
#    Copyright (C) 2012-2023 CMCC Foundation
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

SUBDIRS=.

LZO_CFLAGS=
LZO_LIBS=-llzo2

XZ_CFLAGS=
XZ_LIBS=-llzma

CFLAGS+=${OPT}

GSL_CFLAG=
GSL_LDFLAG=

if GSL_SUPPORT
GSL_CFLAG+=$(GSL_INCLUDE) -DGSL_SUPPORTED
GSL_LDFLAG+=$(GSL_LIB)
endif

# Common primitive libraries

lib_LTLIBRARIES = liboph_debug.la
liboph_debug_la_SOURCES = oph_debug.c oph_debug.h
liboph_debug_la_CFLAGS = -prefer-pic $(CFLAGS)
liboph_debug_la_LDFLAGS = -avoid-version -static $(LIBS)

lib_LTLIBRARIES += liboph_core.la
liboph_core_la_SOURCES = oph_core.c oph_core.h
liboph_core_la_CFLAGS = -prefer-pic $(MYSQL_CFLAGS) $(CFLAGS)
liboph_core_la_LDFLAGS = -avoid-version -static $(LIBS)

lib_LTLIBRARIES += liboph_core_subset.la
liboph_core_subset_la_SOURCES = oph_core_subset.c oph_core_subset.h
liboph_core_subset_la_CFLAGS = -prefer-pic $(MYSQL_CFLAGS) $(CFLAGS)
liboph_core_subset_la_LDFLAGS = -avoid-version -static $(LIBS)

# Primitives operating on IDs

lib_LTLIBRARIES += liboph_id_to_index.la
liboph_id_to_index_la_SOURCES = oph_id_to_index.c oph_id_to_index.h
liboph_id_to_index_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_id_to_index_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug

lib_LTLIBRARIES += liboph_id3.la
liboph_id3_la_SOURCES = oph_id3.c oph_id3.h
liboph_id3_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_id3_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug

lib_LTLIBRARIES += liboph_id_of_subset.la
liboph_id_of_subset_la_SOURCES = oph_id_of_subset.c oph_id_of_subset.h
liboph_id_of_subset_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_id_of_subset_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core -loph_core_subset

# Primitives operating on measures

lib_LTLIBRARIES += liboph_aggregate_stats.la
liboph_aggregate_stats_la_SOURCES = oph_aggregate_stats.c oph_aggregate_stats.h
liboph_aggregate_stats_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_aggregate_stats_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_aggregate_stats_partial.la
liboph_aggregate_stats_partial_la_SOURCES = oph_aggregate_stats_partial.c oph_aggregate_stats_partial.h
liboph_aggregate_stats_partial_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_aggregate_stats_partial_la_LDFLAGS = -avoid-version -shared $(LIBS) -lm -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_aggregate_stats_final.la
liboph_aggregate_stats_final_la_SOURCES = oph_aggregate_stats_final.c oph_aggregate_stats_final.h
liboph_aggregate_stats_final_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_aggregate_stats_final_la_LDFLAGS = -avoid-version -shared $(LIBS) -lm -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_append.la
liboph_append_la_SOURCES = oph_append.c oph_append.h
liboph_append_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_append_la_LDFLAGS = -avoid-version -shared $(LIBS) -lm -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_compress.la
liboph_compress_la_SOURCES = oph_compress.c oph_compress.h
liboph_compress_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_compress_la_LDFLAGS = -avoid-version -shared $(LIBS) -lm -lz -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_uncompress.la
liboph_uncompress_la_SOURCES = oph_uncompress.c oph_uncompress.h
liboph_uncompress_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_uncompress_la_LDFLAGS = -avoid-version -shared $(LIBS) -lz -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_aggregate_operator.la
liboph_aggregate_operator_la_SOURCES = oph_aggregate_operator.c oph_aggregate_operator.h
liboph_aggregate_operator_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_aggregate_operator_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_normalize.la
liboph_normalize_la_SOURCES = oph_normalize.c oph_normalize.h
liboph_normalize_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_normalize_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_reduce.la
liboph_reduce_la_SOURCES = oph_reduce.c oph_reduce.h
liboph_reduce_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_reduce_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_reduce2.la
liboph_reduce2_la_SOURCES = oph_reduce2.c oph_reduce2.h
liboph_reduce2_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_reduce2_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_reduce3.la
liboph_reduce3_la_SOURCES = oph_reduce3.c oph_reduce3.h
liboph_reduce3_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_reduce3_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_dump.la
liboph_dump_la_SOURCES = oph_dump.c oph_dump.h
liboph_dump_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_dump_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_convert_d.la
liboph_convert_d_la_SOURCES = oph_convert_d.c oph_convert_d.h
liboph_convert_d_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_convert_d_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_convert_l.la
liboph_convert_l_la_SOURCES = oph_convert_l.c oph_convert_l.h
liboph_convert_l_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_convert_l_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_count_array.la
liboph_count_array_la_SOURCES = oph_count_array.c oph_count_array.h
liboph_count_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_count_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_get_subarray.la
liboph_get_subarray_la_SOURCES = oph_get_subarray.c oph_get_subarray.h
liboph_get_subarray_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_get_subarray_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_get_subarray2.la
liboph_get_subarray2_la_SOURCES = oph_get_subarray2.c oph_get_subarray2.h
liboph_get_subarray2_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_get_subarray2_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core -loph_core_subset

lib_LTLIBRARIES += liboph_get_subarray3.la
liboph_get_subarray3_la_SOURCES = oph_get_subarray3.c oph_get_subarray3.h
liboph_get_subarray3_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_get_subarray3_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core -loph_core_subset

lib_LTLIBRARIES += liboph_sum_scalar.la
liboph_sum_scalar_la_SOURCES = oph_sum_scalar.c oph_sum_scalar.h
liboph_sum_scalar_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_sum_scalar_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_mul_scalar.la
liboph_mul_scalar_la_SOURCES = oph_mul_scalar.c oph_mul_scalar.h
liboph_mul_scalar_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_mul_scalar_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_sum_scalar2.la
liboph_sum_scalar2_la_SOURCES = oph_sum_scalar2.c oph_sum_scalar2.h
liboph_sum_scalar2_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_sum_scalar2_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_mul_scalar2.la
liboph_mul_scalar2_la_SOURCES = oph_mul_scalar2.c oph_mul_scalar2.h
liboph_mul_scalar2_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_mul_scalar2_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_affine.la
liboph_affine_la_SOURCES = oph_affine.c oph_affine.h oph_debug.c oph_debug.h oph_core.c oph_core.h
liboph_affine_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_affine_la_LDFLAGS = -avoid-version -shared $(LIBS)

lib_LTLIBRARIES += liboph_get_index_array.la
liboph_get_index_array_la_SOURCES = oph_get_index_array.c oph_get_index_array.h
liboph_get_index_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_get_index_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_to_bin.la
liboph_to_bin_la_SOURCES = oph_to_bin.c oph_to_bin.h
liboph_to_bin_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_to_bin_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_compare.la
liboph_compare_la_SOURCES = oph_compare.c oph_compare.h
liboph_compare_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_compare_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug

lib_LTLIBRARIES += liboph_extract.la
liboph_extract_la_SOURCES = oph_extract.c oph_extract.h
liboph_extract_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_extract_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_find.la
liboph_find_la_SOURCES = oph_find.c oph_find.h
liboph_find_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_find_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_interlace.la
liboph_interlace_la_SOURCES = oph_interlace.c oph_interlace.h
liboph_interlace_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_interlace_la_LDFLAGS = -avoid-version -shared $(LIBS) -lm -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_interlace2.la
liboph_interlace2_la_SOURCES = oph_interlace2.c oph_interlace2.h
liboph_interlace2_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_interlace2_la_LDFLAGS = -avoid-version -shared $(LIBS) -lm -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_math.la
liboph_math_la_SOURCES = oph_math.c oph_math.h oph_core_math.c oph_core_math.h
liboph_math_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_math_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_padding.la
liboph_padding_la_SOURCES = oph_padding.c oph_padding.h
liboph_padding_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_padding_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_permute.la
liboph_permute_la_SOURCES = oph_permute.c oph_permute.h
liboph_permute_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_permute_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_reverse.la
liboph_reverse_la_SOURCES = oph_reverse.c oph_reverse.h
liboph_reverse_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_reverse_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_roll_up.la
liboph_roll_up_la_SOURCES = oph_roll_up.c oph_roll_up.h
liboph_roll_up_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_roll_up_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_rotate.la
liboph_rotate_la_SOURCES = oph_rotate.c oph_rotate.h
liboph_rotate_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_rotate_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_sequence.la
liboph_sequence_la_SOURCES = oph_sequence.c oph_sequence.h
liboph_sequence_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_sequence_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_size_array.la
liboph_size_array_la_SOURCES = oph_size_array.c oph_size_array.h
liboph_size_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_size_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_shift.la
liboph_shift_la_SOURCES = oph_shift.c oph_shift.h
liboph_shift_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_shift_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_value_to_bin.la
liboph_value_to_bin_la_SOURCES = oph_value_to_bin.c oph_value_to_bin.h
liboph_value_to_bin_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_value_to_bin_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_operation_array.la
liboph_operation_array_la_SOURCES = oph_operation_array.c oph_operation_array.h
liboph_operation_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_operation_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_sum_array.la
liboph_sum_array_la_SOURCES = oph_sum_array.c oph_sum_array.h
liboph_sum_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_sum_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_mul_array.la
liboph_mul_array_la_SOURCES = oph_mul_array.c oph_mul_array.h
liboph_mul_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_mul_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_mask_array.la
liboph_mask_array_la_SOURCES = oph_mask_array.c oph_mask_array.h
liboph_mask_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_mask_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_sub_array.la
liboph_sub_array_la_SOURCES = oph_sub_array.c oph_sub_array.h
liboph_sub_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_sub_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_div_array.la
liboph_div_array_la_SOURCES = oph_div_array.c oph_div_array.h
liboph_div_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_div_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_abs_array.la
liboph_abs_array_la_SOURCES = oph_abs_array.c oph_abs_array.h
liboph_abs_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_abs_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_arg_array.la
liboph_arg_array_la_SOURCES = oph_arg_array.c oph_arg_array.h
liboph_arg_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_arg_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_max_array.la
liboph_max_array_la_SOURCES = oph_max_array.c oph_max_array.h
liboph_max_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_max_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_min_array.la
liboph_min_array_la_SOURCES = oph_min_array.c oph_min_array.h
liboph_min_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_min_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_arg_max_array.la
liboph_arg_max_array_la_SOURCES = oph_arg_max_array.c oph_arg_max_array.h
liboph_arg_max_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_arg_max_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_arg_min_array.la
liboph_arg_min_array_la_SOURCES = oph_arg_min_array.c oph_arg_min_array.h
liboph_arg_min_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_arg_min_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_operator.la
liboph_operator_la_SOURCES = oph_operator.c oph_operator.h
liboph_operator_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_operator_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_operator_array.la
liboph_operator_array_la_SOURCES = oph_operator_array.c oph_operator_array.h
liboph_operator_array_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_operator_array_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_cast.la
liboph_cast_la_SOURCES = oph_cast.c oph_cast.h
liboph_cast_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_cast_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_concat.la
liboph_concat_la_SOURCES = oph_concat.c oph_concat.h
liboph_concat_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_concat_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_concat2.la
liboph_concat2_la_SOURCES = oph_concat2.c oph_concat2.h
liboph_concat2_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_concat2_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_moving_avg.la
liboph_moving_avg_la_SOURCES = oph_moving_avg.c oph_moving_avg.h
liboph_moving_avg_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_moving_avg_la_LDFLAGS = -avoid-version -shared $(LIBS) -lm -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_accumulate.la
liboph_accumulate_la_SOURCES = oph_accumulate.c oph_accumulate.h
liboph_accumulate_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_accumulate_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_deaccumulate.la
liboph_deaccumulate_la_SOURCES = oph_deaccumulate.c oph_deaccumulate.h
liboph_deaccumulate_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_deaccumulate_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_replace.la
liboph_replace_la_SOURCES = oph_replace.c oph_replace.h
liboph_replace_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_replace_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_ccluster_kcluster.la
liboph_ccluster_kcluster_la_SOURCES = oph_ccluster_kcluster.c oph_ccluster_kcluster.h cluster.c cluster.h
liboph_ccluster_kcluster_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_ccluster_kcluster_la_LDFLAGS = -avoid-version -shared $(LIBS) -lm -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_quantize.la
liboph_quantize_la_SOURCES = oph_quantize.c oph_quantize.h
liboph_quantize_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_quantize_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_expand.la
liboph_expand_la_SOURCES = oph_expand.c oph_expand.h
liboph_expand_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_expand_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_extend.la
liboph_extend_la_SOURCES = oph_extend.c oph_extend.h
liboph_extend_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS)
liboph_extend_la_LDFLAGS = -avoid-version -shared $(LIBS) -L. -loph_debug -loph_core

if MATHEVAL_SUPPORT
lib_LTLIBRARIES += liboph_predicate.la
liboph_predicate_la_SOURCES = oph_predicate.c oph_predicate.h oph_core_matheval.c oph_core_matheval.h
liboph_predicate_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(MATHEVAL_CFLAGS)
liboph_predicate_la_LDFLAGS = -avoid-version -shared $(LIBS) $(MATHEVAL_LIBS) -lpthread -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_predicate2.la
liboph_predicate2_la_SOURCES = oph_predicate2.c oph_predicate2.h oph_core_matheval.c oph_core_matheval.h
liboph_predicate2_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(MATHEVAL_CFLAGS)
liboph_predicate2_la_LDFLAGS = -avoid-version -shared $(LIBS) $(MATHEVAL_LIBS) -lpthread -L. -loph_debug -loph_core
endif

if GSL_SUPPORT
lib_LTLIBRARIES += liboph_gsl_boxplot.la
liboph_gsl_boxplot_la_SOURCES = oph_gsl_boxplot.c oph_gsl_boxplot.h oph_core_gsl.h
liboph_gsl_boxplot_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_boxplot_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_sd.la
liboph_gsl_sd_la_SOURCES = oph_gsl_sd.c oph_gsl_sd.h oph_core_gsl.h
liboph_gsl_sd_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_sd_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_complex_get_real.la
liboph_gsl_complex_get_real_la_SOURCES = oph_gsl_complex_get_real.c oph_gsl_complex_get_real.h oph_core_gsl.h
liboph_gsl_complex_get_real_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_complex_get_real_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_complex_get_imag.la
liboph_gsl_complex_get_imag_la_SOURCES = oph_gsl_complex_get_imag.c oph_gsl_complex_get_imag.h oph_core_gsl.h
liboph_gsl_complex_get_imag_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_complex_get_imag_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_complex_get_arg.la
liboph_gsl_complex_get_arg_la_SOURCES = oph_gsl_complex_get_arg.c oph_gsl_complex_get_arg.h oph_core_gsl.h
liboph_gsl_complex_get_arg_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_complex_get_arg_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_complex_get_abs.la
liboph_gsl_complex_get_abs_la_SOURCES = oph_gsl_complex_get_abs.c oph_gsl_complex_get_abs.h oph_core_gsl.h
liboph_gsl_complex_get_abs_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_complex_get_abs_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_complex_to_polar.la
liboph_gsl_complex_to_polar_la_SOURCES = oph_gsl_complex_to_polar.c oph_gsl_complex_to_polar.h oph_core_gsl.h
liboph_gsl_complex_to_polar_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_complex_to_polar_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_complex_to_rect.la
liboph_gsl_complex_to_rect_la_SOURCES = oph_gsl_complex_to_rect.c oph_gsl_complex_to_rect.h oph_core_gsl.h
liboph_gsl_complex_to_rect_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_complex_to_rect_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_ifft.la
liboph_gsl_ifft_la_SOURCES = oph_gsl_ifft.c oph_gsl_ifft.h oph_core_gsl.h
liboph_gsl_ifft_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_ifft_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_idwt.la
liboph_gsl_idwt_la_SOURCES = oph_gsl_idwt.c oph_gsl_idwt.h oph_core_gsl.h
liboph_gsl_idwt_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_idwt_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_fft.la
liboph_gsl_fft_la_SOURCES = oph_gsl_fft.c oph_gsl_fft.h oph_core_gsl.h
liboph_gsl_fft_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_fft_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_filter.la
liboph_filter_la_SOURCES = oph_filter.c oph_filter.h oph_core_gsl.h
liboph_filter_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_filter_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LIBS) -L. -loph_debug -loph_core


lib_LTLIBRARIES += liboph_gsl_dwt.la
liboph_gsl_dwt_la_SOURCES = oph_gsl_dwt.c oph_gsl_dwt.h oph_core_gsl.h
liboph_gsl_dwt_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_dwt_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_histogram.la
liboph_gsl_histogram_la_SOURCES = oph_gsl_histogram.c oph_cast.h oph_core_gsl.h
liboph_gsl_histogram_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_histogram_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_quantile.la
liboph_gsl_quantile_la_SOURCES = oph_gsl_quantile.c oph_gsl_quantile.h oph_core_gsl.h
liboph_gsl_quantile_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_quantile_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_stats.la
liboph_gsl_stats_la_SOURCES = oph_gsl_stats.c oph_gsl_stats.h oph_core_gsl.h
liboph_gsl_stats_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_stats_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_sort.la
liboph_gsl_sort_la_SOURCES = oph_gsl_sort.c oph_gsl_sort.h oph_core_gsl.h
liboph_gsl_sort_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_sort_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_spline.la
liboph_gsl_spline_la_SOURCES = oph_gsl_spline.c oph_gsl_spline.h oph_core_gsl.h
liboph_gsl_spline_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_spline_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_fit_linear.la
liboph_gsl_fit_linear_la_SOURCES = oph_gsl_fit_linear.c oph_gsl_fit_linear.h oph_core_gsl.h
liboph_gsl_fit_linear_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_fit_linear_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_fit_linear_coeff.la
liboph_gsl_fit_linear_coeff_la_SOURCES = oph_gsl_fit_linear_coeff.c oph_gsl_fit_linear_coeff.h oph_core_gsl.h
liboph_gsl_fit_linear_coeff_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_fit_linear_coeff_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core

lib_LTLIBRARIES += liboph_gsl_correlation.la
liboph_gsl_correlation_la_SOURCES = oph_gsl_correlation.c oph_gsl_correlation.h oph_core_gsl.h
liboph_gsl_correlation_la_CFLAGS = $(MYSQL_CFLAGS) $(CFLAGS) $(GSL_CFLAG)
liboph_gsl_correlation_la_LDFLAGS = -avoid-version -shared $(LIBS) $(GSL_LDFLAG) -L. -loph_debug -loph_core
endif

