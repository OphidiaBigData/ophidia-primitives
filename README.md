# Ophidia Primitives

### Description

The Ophidia Primitives is a set of shared libraries for an Ophidia I/O node. Each function (or *primitive*) is a low-level operator which works on binary arrays. Each library can be plugged-in a MySQL instance as a User-Defined Function. The primitive can be exploited within SQL queries like the embedded functions.

### Requirements

In order to compile and run the Ophidia Primitives, make sure you have the following packages (all available through CentOS official repositories and the epel repository) properly installed:

1. mysql-community-server
2. zlib
3. GSL - GNU Scientific Library
4. GNU libmatheval

### How to Install

If you are building from git, you also need automake, autoconf and libtool. To prepare the code for building run:

```
$ ./bootstrap 
```

The source code has been packaged with GNU Autotools, so to install simply type:

```
$ ./configure --prefix=prefix
$ make
$ make install
```
Type:

```
$ ./configure --help
```

to see all available options.

To complete the installation, run the following: 

```
$ cp prefix/liboph_*.so $(mysql_config --plugindir)
$ mysql -u user -p -H hostname -P port mysql < prefix/etc/create_func.sql
```

Further information can be found at [http://ophidia.cmcc.it/documentation](http://ophidia.cmcc.it/documentation).
