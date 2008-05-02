noinst_HEADERS = simcaqp.h

EXTRA_DIST = libchemgps.pc libchemgps.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libchemgps.pc

lib_LTLIBRARIES = libchemgps.la
libchemgps_la_SOURCES = chemgps.h simcaqp.h project.c predict.c result.c \
  chemgps.c simcaqp.c data.c cpuinfo.c
libchemgps_la_LDFLAGS = -version-info 1:0:1 @SIMCAQLIBS@ @SIMCAQLDFLAGS@
libchemgps_la_CPPFLAGS = @SIMCAQCPPFLAGS@
