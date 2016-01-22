AC_INIT([elfcat], [0.0], [])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC_C99
AC_GNU_SOURCE
AC_SYS_LARGEFILE

PKG_CHECK_MODULES([LIBELF], [libelf], [have_libelf=yes], [have_libelf=no])

if test "x$have_libelf" = "xno"; then
	AC_CHECK_LIB([elf], [elf_begin], [have_libelf=yes], [])
	AC_CHECK_HEADER([libelf.h], [], [have_libelf=no])

	if test "x$have_libelf" = "xno"; then
		AC_MSG_ERROR([*** libelf support not found])
	fi

	LIBELF_LIBS=-lelf
	LIBELF_CFLAGS=
fi

AC_SUBST([LIBELF_CFLAGS])
AC_SUBST([LIBELF_LIBS])

AC_CONFIG_FILES([
	Makefile
])
AC_OUTPUT
