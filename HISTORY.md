
## v1.0.0 - 2017-03-23

### Changed:

- Code indentation style

## v0.11.0 - 2017-01-31

### Fixed:

- Bug in oph_sub_array and oph_div_array
- Bug in oph_dump when using base64-encoding
- Bug in oph_to_bin argument usage

### Added:

- Support for missing values in several primitives

### Changed:

- Cast functions to handle missing values

## v0.10.6 - 2016-10-20

### Fixed:

- Bug in oph_aggregate_operator primitive
- Bug in oph_gsl_fit_linear primitive
- Bug in oph_gsl_fit_linear_coeff primitive
- Bug in oph_moving_avg primitive
- Bug in multiparam free core function

## v0.10.5 - 2016-08-24

### Fixed:

- Bug in oph_reduce3 primitive
- Bug in multistring free core function
- Skip 'lib' in referring to libmatheval
- Bug in processing oph_dump input parameter
- Bug in core function of oph_dump
- Several warnings when building

### Added:

- Support for base64 encoding in oph_dump

## v0.10.1 - 2016-06-27

### Fixed:
 
- Version number in files

## v0.10.0 - 2016-06-23

### Fixed:

- Bug in evaluating standard deviation in case of approximation errors
- Bug in oph_cast primitive
- Bug in aggregating arrays containing NaNs to obtain mean values
- Bug in reducing arrays containing only NaNs
- Bug [\#1](https://github.com/OphidiaBigData/ophidia-primitives/issues/1)
- Bug in handling non-double data in oph_predicate core

### Added:

- New primitive oph_extend
- New primitive oph_gsl_correlation
- New primitive oph_expand
- New primitive oph_predicate2
- New option oph_count in oph_aggregate_operator primitive

## v0.9.0 - 2016-02-01

- Initial public release
