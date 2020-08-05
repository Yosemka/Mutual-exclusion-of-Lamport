## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = critic

critic_SOURCES = \
	common.h \
	ipc.c \
	ipc.h \
	main.c \
	pa2345.h \
	process.c \
	process.h \
	list.c \
	list.h \
	banking.h

critic_LDFLAGS = 

critic_LDADD =  \
	/home/linux/critic/src/libruntime.so

