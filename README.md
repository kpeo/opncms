# opnCMS

[![Join the chat at https://gitter.im/opncms/opncms](https://badges.gitter.im/opncms/opncms.svg)](https://gitter.im/opncms/opncms?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=B3U4YZF4QXV6G&lc=EN&item_name=opnCMS%20donation&item_number=opncms&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHosted)
[![Donate with Bitcoin](https://en.cryptobadges.io/badge/micro/1Nc7WMLf9ZyKznEAaYzxhY2jEQo85uvzc2)](https://en.cryptobadges.io/donate/1Nc7WMLf9ZyKznEAaYzxhY2jEQo85uvzc2)
[![Donate with Ethereum](https://en.cryptobadges.io/badge/micro/0xec44da132d844ab028fbfdbb3323df2b1ad3c8e7)](https://en.cryptobadges.io/donate/0xec44da132d844ab028fbfdbb3323df2b1ad3c8e7)

__opnCMS__ is an simple environment based on [CppCMS web-framework](http://cppcms.com) for building high-performance modern web applications.

![overview](https://github.com/opncms/opncms/raw/master/overview.png)

## Features
* Configurable plugin system supported both synchronous (GET/POST) and asyncronous (JSON-RPC) connections to core app.
* Flexible template support for plugins with push and pull styles.
* Independant modules to handle view, data, media, authentication tasks with set of extendable drivers.
* Ready-made plugin's code generator like in modern web-frameworks
* Using CppDB for SQL databases, MongoDB, own NoSQL-driver
* Ability of changing type of storage on the fly (to use it simultaneously)
* Universal interface for all type of storages (with capability of using direct DB drivers)
* Basic user plugin to handle Bcrypt authorization (including registration, reset/change password features) with email notifications (supports TLS and authentication on SMTP server)
* [WIP] Basic administration plugin for configuration and pages

## Install

### Requirements
* CppCMS library 1.2.0+ (to build - see [instructions](http://cppcms.com/wikipp/en/page/cppcms_1x_build))

* Boost libraries (libboost-system, libboost-thread, libboost-filesystem). For Ubuntu/Debian:
  
  ``apt-get install libboost-system-dev libboost-thread-dev libboost-filesystem-dev``

### Getting the sources

```
git clone https://github.com/opncms/opncms.git opncms
git clone https://github.com/opncms/frameworks.git frameworks

# optional plugins
git clone https://github.com/opncms/plugins.git plugins
```

### Build

```
cd opncms
mkdir build
cmake ..
make
make test
```

### Deployment

#### Debian/Ubuntu
```
cd opncms

# opncms configs and static files 
cp -R install/var/www/opncms /var/www
cp -R static /var/www/opncms

# compiled files
cp build/opncms /var/www/opncms
cp build/libopncms_view.so /var/www/opncms

# plugins (optional)
cp ../plugins/<plugin_name>/lib<plugin_name>_view.so /var/www/opncms
mkdir -p /var/www/opncms/plugins && cp ../plugins/<plugin_name>/<plugin_name>.so /var/www/opncms/plugins
```

#### Nginx

Use Nginx config file [opncms.conf](https://github.com/kpeo/opncms/tree/master/install/etc/nginx/conf.d/opncms.conf)

### Run

Use bash [opncms.sh](https://github.com/opncms/opncms/blob/master/install/opncms.sh) script
