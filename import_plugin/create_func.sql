--
--    Ophidia Primitives
--    Copyright (C) 2012-2016 CMCC Foundation
--
--    This program is free software: you can redistribute it and/or modify
--    it under the terms of the GNU General Public License as published by
--    the Free Software Foundation, either version 3 of the License, or
--    (at your option) any later version.
--
--    This program is distributed in the hope that it will be useful,
--    but WITHOUT ANY WARRANTY; without even the implied warranty of
--    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--    GNU General Public License for more details.
--
--    You should have received a copy of the GNU General Public License
--    along with this program.  If not, see <http://www.gnu.org/licenses/>.
--

DROP PROCEDURE IF EXISTS mysql.oph_drill_down;
DROP PROCEDURE IF EXISTS mysql.oph_subset;

DROP FUNCTION IF EXISTS mysql.oph_id;
DROP FUNCTION IF EXISTS mysql.oph_id2;
DROP FUNCTION IF EXISTS mysql.oph_is_in_subset;
DROP FUNCTION IF EXISTS mysql.oph_id_to_index2;

DROP FUNCTION IF EXISTS oph_id_to_index;
DROP FUNCTION IF EXISTS oph_id3;
DROP FUNCTION IF EXISTS oph_id_of_subset;

DROP FUNCTION IF EXISTS oph_get_subarray;
DROP FUNCTION IF EXISTS oph_operator;
DROP FUNCTION IF EXISTS oph_dump;
DROP FUNCTION IF EXISTS oph_convert_d;
DROP FUNCTION IF EXISTS oph_size_array;
DROP FUNCTION IF EXISTS oph_count_array;
DROP FUNCTION IF EXISTS oph_sum_scalar;
DROP FUNCTION IF EXISTS oph_mul_scalar;
DROP FUNCTION IF EXISTS oph_sum_array;
DROP FUNCTION IF EXISTS oph_sum_array_r;
DROP FUNCTION IF EXISTS oph_find;
DROP FUNCTION IF EXISTS oph_compress;
DROP FUNCTION IF EXISTS oph_uncompress;
DROP FUNCTION IF EXISTS oph_reduce;
DROP FUNCTION IF EXISTS oph_reduce2;
DROP FUNCTION IF EXISTS oph_reduce3;
DROP FUNCTION IF EXISTS oph_aggregate_operator;
DROP FUNCTION IF EXISTS oph_gsl_sd;
DROP FUNCTION IF EXISTS oph_shift;
DROP FUNCTION IF EXISTS oph_rotate;
DROP FUNCTION IF EXISTS oph_reverse;
DROP FUNCTION IF EXISTS oph_math;
DROP FUNCTION IF EXISTS oph_gsl_boxplot;
DROP FUNCTION IF EXISTS oph_concat;
DROP FUNCTION IF EXISTS oph_predicate;
DROP FUNCTION IF EXISTS oph_to_bin;
DROP FUNCTION IF EXISTS oph_get_index_array;
DROP FUNCTION IF EXISTS oph_roll_up;
DROP FUNCTION IF EXISTS oph_get_subarray2;
DROP FUNCTION IF EXISTS oph_get_subarray3;
DROP FUNCTION IF EXISTS oph_permute;
DROP FUNCTION IF EXISTS oph_aggregate_stats;
DROP FUNCTION IF EXISTS oph_compress2;
DROP FUNCTION IF EXISTS oph_uncompress2;
DROP FUNCTION IF EXISTS oph_gsl_complex_get_abs;
DROP FUNCTION IF EXISTS oph_gsl_complex_get_arg;
DROP FUNCTION IF EXISTS oph_gsl_complex_get_imag;
DROP FUNCTION IF EXISTS oph_gsl_complex_get_real;
DROP FUNCTION IF EXISTS oph_gsl_complex_to_polar;
DROP FUNCTION IF EXISTS oph_gsl_complex_to_rect;
DROP FUNCTION IF EXISTS oph_gsl_dwt;
DROP FUNCTION IF EXISTS oph_gsl_fft;
DROP FUNCTION IF EXISTS oph_gsl_idwt;
DROP FUNCTION IF EXISTS oph_gsl_ifft;
DROP FUNCTION IF EXISTS oph_gsl_quantile;
DROP FUNCTION IF EXISTS oph_gsl_sort;
DROP FUNCTION IF EXISTS oph_gsl_stats;
DROP FUNCTION IF EXISTS oph_gsl_histogram;
DROP FUNCTION IF EXISTS oph_sum_scalar2;
DROP FUNCTION IF EXISTS oph_mul_scalar2;
DROP FUNCTION IF EXISTS oph_cast;
DROP FUNCTION IF EXISTS oph_compare;
DROP FUNCTION IF EXISTS oph_extract;
DROP FUNCTION IF EXISTS oph_moving_avg;
DROP FUNCTION IF EXISTS oph_value_to_bin;
DROP FUNCTION IF EXISTS oph_convert_l;
DROP FUNCTION IF EXISTS oph_convert_d_old;
DROP FUNCTION IF EXISTS oph_mul_array;
DROP FUNCTION IF EXISTS oph_sub_array;
DROP FUNCTION IF EXISTS oph_div_array;
DROP FUNCTION IF EXISTS oph_abs_array;
DROP FUNCTION IF EXISTS oph_arg_array;
DROP FUNCTION IF EXISTS oph_max_array;
DROP FUNCTION IF EXISTS oph_min_array;
DROP FUNCTION IF EXISTS oph_operator_array;
DROP FUNCTION IF EXISTS oph_ccluster_kcluster;
DROP FUNCTION IF EXISTS oph_aggregate_stats_partial;
DROP FUNCTION IF EXISTS oph_aggregate_stats_final;
DROP FUNCTION IF EXISTS oph_mask_array;
DROP FUNCTION IF EXISTS oph_interlace;
DROP FUNCTION IF EXISTS oph_quantize;
DROP FUNCTION IF EXISTS oph_gsl_spline;
DROP FUNCTION IF EXISTS oph_gsl_fit_linear;
DROP FUNCTION IF EXISTS oph_gsl_fit_linear_coeff;
DROP FUNCTION IF EXISTS oph_accumulate;
DROP FUNCTION IF EXISTS oph_deaccumulate;

DELIMITER //
CREATE PROCEDURE mysql.oph_drill_down(IN table_in VARCHAR(100), IN outer_size INT, IN inner_size INT, IN oph_type VARCHAR(30), IN table_out VARCHAR(100), IN compressed INT)
BEGIN
SET @s = CONCAT('DROP TABLE IF EXISTS ', table_out);
PREPARE stm FROM @s;
EXECUTE stm;
DEALLOCATE PREPARE stm;
SET @s = CONCAT('CREATE TABLE ', table_out, ' (id_dim INTEGER, measure LONGBLOB) ENGINE=MyISAM DEFAULT CHARSET=latin1');
PREPARE stm FROM @s;
EXECUTE stm;
DEALLOCATE PREPARE stm; 
IF compressed = 0 THEN SET @s = CONCAT('INSERT INTO ', table_out, ' SELECT ((id_dim-1)*', outer_size, '+?) AS id_dim, oph_get_subarray("', oph_type, '","', oph_type, '", measure, ?, ', inner_size, ') AS measure FROM ', table_in);
ELSE SET @s = CONCAT('INSERT INTO ', table_out, ' SELECT ((id_dim-1)*', outer_size, '+?) AS id_dim, oph_compress("","", oph_get_subarray("', oph_type, '","', oph_type, '", oph_uncompress("", "", measure), ?, ', inner_size, ')) AS measure FROM ', table_in);
END IF;
PREPARE stm FROM @s;
SET @counter = 1;
SET @pointer = 1;
WHILE @counter <= outer_size DO
EXECUTE stm USING @counter, @pointer;
SET @counter = @counter + 1;
SET @pointer = @pointer + inner_size;
END WHILE;
DEALLOCATE PREPARE stm;
END //
DELIMITER ;

DELIMITER //
CREATE PROCEDURE mysql.oph_subset(IN table_in VARCHAR(100), IN start_id INT, IN get_sub_array VARCHAR(1000), IN table_out VARCHAR(100), IN where_clause VARCHAR(1000))
BEGIN
IF where_clause = '' THEN 
SET @s = CONCAT('CREATE TABLE ', table_out, ' (id_dim INTEGER, measure LONGBLOB) ENGINE=MyISAM DEFAULT CHARSET=latin1 AS SELECT id_dim, ', get_sub_array ,' AS measure FROM ', table_in,';');
ELSE
SET @ID = start_id - 1;
SET @s = CONCAT('CREATE TABLE ', table_out, ' (id_dim INTEGER, measure LONGBLOB) ENGINE=MyISAM DEFAULT CHARSET=latin1 AS SELECT @ID:=@ID+1 AS id_dim, ', get_sub_array ,' AS measure FROM ', table_in,' WHERE ', where_clause, ';');
END IF;
PREPARE stm FROM @s;
EXECUTE stm;
DEALLOCATE PREPARE stm; 
END //
DELIMITER ;

CREATE FUNCTION mysql.oph_id(id INT, size INT) RETURNS INT RETURN 1+FLOOR((id-1)/size);
CREATE FUNCTION mysql.oph_id2(id INT, size INT, block_size INT) RETURNS INT RETURN 1+MOD(id-1,block_size)+(FLOOR((id-1)/(size*block_size)))*block_size;
CREATE FUNCTION mysql.oph_is_in_subset(id INT, start INT, step INT, max INT) RETURNS INT RETURN NOT MOD(id-start,step) AND id>=start AND id<=max;
CREATE FUNCTION mysql.oph_id_to_index2(id INT, block_size INT, size INT) RETURNS INT RETURN 1+MOD(FLOOR((id-1)/block_size),size);

CREATE FUNCTION oph_id_to_index RETURNS INTEGER SONAME 'liboph_id_to_index.so';
CREATE FUNCTION oph_id3 RETURNS INTEGER SONAME 'liboph_id3.so';
CREATE FUNCTION oph_id_of_subset RETURNS INTEGER SONAME 'liboph_id_of_subset.so';

CREATE FUNCTION oph_get_subarray RETURNS STRING SONAME 'liboph_get_subarray.so';
CREATE FUNCTION oph_operator RETURNS STRING SONAME 'liboph_operator.so';
CREATE FUNCTION oph_dump RETURNS STRING SONAME 'liboph_dump.so';
CREATE FUNCTION oph_convert_d RETURNS REAL SONAME 'liboph_convert_d.so';
CREATE FUNCTION oph_convert_l RETURNS INTEGER SONAME 'liboph_convert_l.so';
CREATE FUNCTION oph_size_array RETURNS INTEGER SONAME 'liboph_size_array.so';
CREATE FUNCTION oph_count_array RETURNS INTEGER SONAME 'liboph_count_array.so';
CREATE FUNCTION oph_sum_scalar RETURNS STRING SONAME 'liboph_sum_scalar.so';
CREATE FUNCTION oph_mul_scalar RETURNS STRING SONAME 'liboph_mul_scalar.so';
CREATE FUNCTION oph_sum_array RETURNS STRING SONAME 'liboph_sum_array.so';
CREATE FUNCTION oph_find RETURNS INTEGER SONAME 'liboph_find.so';
CREATE FUNCTION oph_compress RETURNS STRING SONAME 'liboph_compress.so';
CREATE FUNCTION oph_uncompress RETURNS STRING SONAME 'liboph_uncompress.so';
CREATE FUNCTION oph_reduce RETURNS STRING SONAME 'liboph_reduce.so';
CREATE FUNCTION oph_reduce2 RETURNS STRING SONAME 'liboph_reduce2.so';
CREATE FUNCTION oph_reduce3 RETURNS STRING SONAME 'liboph_reduce3.so';
CREATE AGGREGATE FUNCTION oph_aggregate_operator RETURNS STRING SONAME 'liboph_aggregate_operator.so';
CREATE FUNCTION oph_gsl_sd RETURNS STRING SONAME 'liboph_gsl_sd.so';
CREATE FUNCTION oph_shift RETURNS STRING SONAME 'liboph_shift.so';
CREATE FUNCTION oph_rotate RETURNS STRING SONAME 'liboph_rotate.so';
CREATE FUNCTION oph_reverse RETURNS STRING SONAME 'liboph_reverse.so';
CREATE FUNCTION oph_math RETURNS STRING SONAME 'liboph_math.so';
CREATE FUNCTION oph_gsl_boxplot RETURNS STRING SONAME 'liboph_gsl_boxplot.so';
CREATE FUNCTION oph_concat RETURNS STRING SONAME 'liboph_concat.so';
CREATE FUNCTION oph_predicate RETURNS STRING SONAME 'liboph_predicate.so';
CREATE FUNCTION oph_to_bin RETURNS STRING SONAME 'liboph_to_bin.so';
CREATE FUNCTION oph_get_index_array RETURNS STRING SONAME 'liboph_get_index_array.so';
CREATE AGGREGATE FUNCTION oph_roll_up RETURNS STRING SONAME 'liboph_roll_up.so';
CREATE FUNCTION oph_get_subarray2 RETURNS STRING SONAME 'liboph_get_subarray2.so';
CREATE FUNCTION oph_get_subarray3 RETURNS STRING SONAME 'liboph_get_subarray3.so';
CREATE FUNCTION oph_permute RETURNS STRING SONAME 'liboph_permute.so';
CREATE AGGREGATE FUNCTION oph_aggregate_stats RETURNS STRING SONAME 'liboph_aggregate_stats.so';
CREATE FUNCTION oph_gsl_complex_get_abs RETURNS STRING SONAME 'liboph_gsl_complex_get_abs.so';
CREATE FUNCTION oph_gsl_complex_get_arg RETURNS STRING SONAME 'liboph_gsl_complex_get_arg.so';
CREATE FUNCTION oph_gsl_complex_get_imag RETURNS STRING SONAME 'liboph_gsl_complex_get_imag.so';
CREATE FUNCTION oph_gsl_complex_get_real RETURNS STRING SONAME 'liboph_gsl_complex_get_real.so';
CREATE FUNCTION oph_gsl_complex_to_polar RETURNS STRING SONAME 'liboph_gsl_complex_to_polar.so';
CREATE FUNCTION oph_gsl_complex_to_rect RETURNS STRING SONAME 'liboph_gsl_complex_to_rect.so';
CREATE FUNCTION oph_gsl_dwt RETURNS STRING SONAME 'liboph_gsl_dwt.so';
CREATE FUNCTION oph_gsl_fft RETURNS STRING SONAME 'liboph_gsl_fft.so';
CREATE FUNCTION oph_gsl_idwt RETURNS STRING SONAME 'liboph_gsl_idwt.so';
CREATE FUNCTION oph_gsl_ifft RETURNS STRING SONAME 'liboph_gsl_ifft.so';
CREATE FUNCTION oph_gsl_quantile RETURNS STRING SONAME 'liboph_gsl_quantile.so';
CREATE FUNCTION oph_gsl_sort RETURNS STRING SONAME 'liboph_gsl_sort.so';
CREATE FUNCTION oph_gsl_stats RETURNS STRING SONAME 'liboph_gsl_stats.so';
CREATE FUNCTION oph_gsl_histogram RETURNS STRING SONAME 'liboph_gsl_histogram.so';
CREATE FUNCTION oph_sum_scalar2 RETURNS STRING SONAME 'liboph_sum_scalar2.so';
CREATE FUNCTION oph_mul_scalar2 RETURNS STRING SONAME 'liboph_mul_scalar2.so';
CREATE FUNCTION oph_cast RETURNS STRING SONAME 'liboph_cast.so';
CREATE FUNCTION oph_compare RETURNS INTEGER SONAME 'liboph_compare.so';
CREATE FUNCTION oph_extract RETURNS STRING SONAME 'liboph_extract.so';
CREATE FUNCTION oph_moving_avg RETURNS STRING SONAME 'liboph_moving_avg.so';
CREATE FUNCTION oph_value_to_bin RETURNS STRING SONAME 'liboph_value_to_bin.so';
CREATE FUNCTION oph_mul_array RETURNS STRING SONAME 'liboph_mul_array.so';
CREATE FUNCTION oph_sub_array RETURNS STRING SONAME 'liboph_sub_array.so';
CREATE FUNCTION oph_div_array RETURNS STRING SONAME 'liboph_div_array.so';
CREATE FUNCTION oph_abs_array RETURNS STRING SONAME 'liboph_abs_array.so';
CREATE FUNCTION oph_arg_array RETURNS STRING SONAME 'liboph_arg_array.so';
CREATE FUNCTION oph_max_array RETURNS STRING SONAME 'liboph_max_array.so';
CREATE FUNCTION oph_min_array RETURNS STRING SONAME 'liboph_min_array.so';
CREATE FUNCTION oph_operator_array RETURNS STRING SONAME 'liboph_operator_array.so';
CREATE FUNCTION oph_ccluster_kcluster RETURNS STRING SONAME 'liboph_ccluster_kcluster.so';
CREATE AGGREGATE FUNCTION oph_aggregate_stats_partial RETURNS STRING SONAME 'liboph_aggregate_stats_partial.so';
CREATE AGGREGATE FUNCTION oph_aggregate_stats_final RETURNS STRING SONAME 'liboph_aggregate_stats_final.so';
CREATE FUNCTION oph_mask_array RETURNS STRING SONAME 'liboph_mask_array.so';
CREATE FUNCTION oph_interlace RETURNS STRING SONAME 'liboph_interlace.so';
CREATE FUNCTION oph_quantize RETURNS STRING SONAME 'liboph_quantize.so';
CREATE FUNCTION oph_gsl_spline RETURNS STRING SONAME 'liboph_gsl_spline.so';
CREATE FUNCTION oph_gsl_fit_linear RETURNS STRING SONAME 'liboph_gsl_fit_linear.so';
CREATE FUNCTION oph_gsl_fit_linear_coeff RETURNS STRING SONAME 'liboph_gsl_fit_linear_coeff.so';
CREATE FUNCTION oph_accumulate RETURNS STRING SONAME 'liboph_accumulate.so';
CREATE FUNCTION oph_deaccumulate RETURNS STRING SONAME 'liboph_deaccumulate.so';

