AM_CPPFLAGS += -I$(srcdir) \
	-I$(top_srcdir)/tests/utils/testapp

if HAVE_LIBLTTNG_UST_CTL
noinst_PROGRAMS = gen-ust-tracef
gen_ust_tracef_SOURCES = gen-ust-tracef.c
gen_ust_tracef_LDADD = $(UST_LIBS) $(DL_LIBS)
endif
