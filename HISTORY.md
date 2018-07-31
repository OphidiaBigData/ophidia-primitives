
## New features

### Changed:

- Makefile to speed up building phase

### Fixed:

- Bug [#10](https://github.com/OphidiaBigData/ophidia-primitives/issues/10)

## v1.4.0 - 2018-07-27

### Changed:

- Extend operators associated with oph_normalize
- Extend values of argument 'comparison' in oph_predicate [#9](https://github.com/OphidiaBigData/ophidia-primitives/pull/9)

## v1.3.0 - 2018-06-18

### Added:

- New primitive oph_sequence [#7](https://github.com/OphidiaBigData/ophidia-primitives/pull/7)

### Fixed:

- Bug in oph_value_to_bin primitive [#6](https://github.com/OphidiaBigData/ophidia-primitives/issues/6)

### Changed:

- oph_gsl_quantile primitive primitive to return index of the closest item to quantile [#8](https://github.com/OphidiaBigData/ophidia-primitives/pull/8)

## v1.2.0 - 2018-02-16

### Added:

- New primitive oph_replace [#4](https://github.com/OphidiaBigData/ophidia-primitives/pull/4)
- New primitive oph_normalize [#3](https://github.com/OphidiaBigData/ophidia-primitives/pull/3)
- New primitive oph_padding

### Fixed:

- Bug in multi-fields primitives [#5](https://github.com/OphidiaBigData/ophidia-primitives/pull/5)

## v1.1.0 - 2017-07-28

### Fixed:

- Bug in init of oph_moving_avg primitive
- Possible memory leak in core library
- Bug [#2](https://github.com/OphidiaBigData/ophidia-primitives/issues/2)
- Bug in core STD. Dev. operation 
- Bug in oph_extend primitive
- Bug in memory allocation for oph_extend

### Added:

- New primitive oph_append
- arg_max and arg_min options in reduce primitives
- New primitive oph_concat2

### Changed:

- oph_dump primitive in order to include string termination char
- Allow partial reductions with oph_reduce
- oph_extend primitive to support both append and interlace modalities

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
