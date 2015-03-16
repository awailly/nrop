nrop
====

[![Build Status](https://travis-ci.org/awailly/nrop.svg)](https://travis-ci.org/awailly/nrop)

Automated Return-Oriented Programming Chaining

More informations
-----------------

All informations to build, configure, solve errors, versions used etc etc:

http://aurelien.wail.ly/nrop/

You will need
-------------

To work with submodules (init && update):

    git clone http://github.com/awailly/nrop
    git submodule update --init --recursive

Then try to compile with:

    make prepare
    make -j32

Dockerfile
----------

Using:

    $ sudo docker pull awailly/nrop
    $ sudo docker run -h nrop -t -i awailly/nrop:latest /bin/bash


Building:

    $ sudo docker build -t awailly/nrop:latest .

Contributors
------------

* Aurélien Wailly @awailly
* Axel Souchet @0vercl0k
* Jonathan Salwan @JonathanSalwan
* Anthony Verez @netantho
* Tiphaine Romand @tromand
