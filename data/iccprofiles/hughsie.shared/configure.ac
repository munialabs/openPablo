AC_PREREQ(2.65)

AC_INIT([shared-color-profiles],[0.1.6],[richard@hughsie.com])
AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-xz tar-ustar])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_LN_S

dnl ---------------------------------------------------------------------------
dnl - Argyll
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE(argyll, AS_HELP_STRING([--enable-argyll],[install Argyll profiles]),
	      enable_argyll=$enableval,enable_argyll=yes)
AM_CONDITIONAL(ENABLE_ARGYLL, test $enable_argyll = "yes")

dnl ---------------------------------------------------------------------------
dnl - Oysonar
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE(oysonar, AS_HELP_STRING([--enable-oysonar],[install Oysonar profiles (modified Oyranos profiles)]),
	      enable_oysonar=$enableval,enable_oysonar=yes)
AM_CONDITIONAL(ENABLE_OYRANOS, test $enable_oysonar = "yes")

dnl ---------------------------------------------------------------------------
dnl - Yamma
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE(yamma, AS_HELP_STRING([--enable-yamma],[install yamma profiles]),
	      enable_yamma=$enableval,enable_yamma=yes)
AM_CONDITIONAL(ENABLE_YAMMA, test $enable_yamma = "yes")

AC_CONFIG_FILES([
Makefile
profiles/Makefile
profiles/Argyll/Makefile
profiles/Oysonar/Makefile
profiles/Yamma/Makefile
])
AC_OUTPUT

echo "
                        shared-color-profiles $VERSION
                     ================================
        prefix:                                     ${prefix}
        datadir:                                    ${datadir}
        Argyll profiles:                            ${enable_argyll}
        Oysonar profiles:                           ${enable_oysonar}
        Yamma profiles:                             ${enable_yamma}
"
