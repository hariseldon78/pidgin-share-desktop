## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src po

pidgin_sharedeskdocdir = ${prefix}/doc/pidgin_sharedesk
pidgin_sharedeskdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = $(pidgin_sharedeskdoc_DATA) \
	$(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(pidgin_sharedeskdocdir)
