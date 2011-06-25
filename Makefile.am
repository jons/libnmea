SUBDIRS = . tests

lib_LTLIBRARIES = libnmea.la
include_HEADERS = libnmea.h
libnmea_la_SOURCES = libnmea.c libnmea.h

ACLOCAL_AMFLAGS=-I m4

AM_CFLAGS = -ansi -pedantic -Wall

libnmea_la_LDFLAGS = -version-info 2:0:1

doc::
	doxygen Doxyfile

