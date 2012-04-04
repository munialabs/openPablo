profilesdir = $(datadir)/color/icc
profiles_DATA =						\
	FakeBRG.icc					\
	FakeRBG.icc					\
	AdobeGammaTest.icm				\
	Fogra27L.icc					\
	bluish.icc

SUBDIRS =

if ENABLE_ARGYLL
SUBDIRS += Argyll
endif

if ENABLE_OYRANOS
SUBDIRS += Oysonar
endif

if ENABLE_YAMMA
SUBDIRS += Yamma
endif

EXTRA_DIST =						\
	$(profiles_DATA)

