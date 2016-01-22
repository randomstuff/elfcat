bin_PROGRAMS= elfcat
elfcat_SOURCES= elfcat.c

elfcat_CFLAGS=	$(AM_CFLAGS)
elfcat_LDADD=	$(AM_LDADD)

elfcat_CFLAGS+=	$(LIBELF_CFLAGS)
elfcat_LDADD+=	$(LIBELF_LIBS)

man_MANS = elfcat.1
