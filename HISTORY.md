
## v0.10.4 - 2016-08-22

### Fixed:

- Skip 'lib' in referring to libmatheval

## v0.10.3 - 2016-07-26

### Fixed:

- Bug in processing oph_dump input parameter

## v0.10.2 - 2016-07-19

### Fixed:

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
