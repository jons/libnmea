SUBDIRS = . tests

lib_LTLIBRARIES = libnmea.la
include_HEADERS = libnmea.h
libnmea_la_SOURCES = libnmea2.c

ACLOCAL_AMFLAGS=-I m4

AM_CFLAGS = -ansi -pedantic

libnmea_la_LDFLAGS = -version-info 1:0:0

doc::
	doxygen Doxyfile

