# Stargazer
[![Build Status](https://travis-ci.org/madf/stg.svg?branch=master)](https://travis-ci.org/madf/stg)

A billing system for small home and office networks.

## Dependencies

* CMake - build system.
* Iconv - character encoding conversion.
* Expat (optional) - required by `mod_conf_sg` configuration plugin.
* xmlrpc-c (optional) - required by `mod_conf_rpc` configuration plugin.
* PCap (optional) - required by `mod_cap_pcap` traffic capture plugin.
* `libnetfilter_queue`, `libnfnetlink`, `libmnl` (optional) - required by `mod_cap_nfqueue` traffic capture plugin.
* Firebird (optional) - required by `mod_store_firebird` storage plugin.
* PostgreSQL (optional) - required by `mod_store_postgresql` storage plugin.
* MySQL Connector (optional) - required by `mod_store_mysql` storage plugin.
* boost (optional) - unit tests.

## Compilation and Installation

```
mkdir build
cd build
cmake ..
make
make install
```

It will install everything in /usr/local by default. If you want to install with a different destdir:

```
$ make DESTDIR=/path/to/your/destdir install
```

It will automatically append usr/local to your destdir. So if you specify DESTDIR=foo you will result in the following directory structure:

```
foo/usr/local/bin
foo/usr/local/include
foo/usr/local/lib
```

If you want a custom install dir prefix use CMAKE_INSTALL_PREFIX at compile time:

```
$ cmake -DCMAKE_INSTALL_PREFIX=/your/prefix ..
$ make
$ make install
```

If you specify -DCMAKE_INSTALL_PREFIX=foo you will result in the following directory structure:

```
foo/bin
foo/include
foo/lib
```

### Notes for MacOS X

1. It is not easy to use Firebird on MacOS X, so you may want to opt-out its storage plugin. Use `-DBUILD_NO_MOD_STORE_FIREBIRD=ON` as `cmake` command line option.
2. Homebrew XMLRPC-C version is too old. You may want to build it from scratch. In order to make it visible to CMake, pass `-DXMLRPC_C_CONFIG=/path/to/xmlrpc-c-config` to `cmake`.
3. CMake usually does not see MySQL Connector library installed by Homebrew. Pass `-DMySQLConnector_ROOT=/usr/local/opt/mysql-client/` to `cmake` to make it visible.

## Documentation

https://stg.net.ua/doc/index.html
