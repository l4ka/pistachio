-include Config.mk

################ Source files ##########################################

SRCS	:= $(wildcard *.cc)
INCS	:= $(wildcard *.h)
OBJS	:= $(addprefix $O,$(SRCS:.cc=.o))
DEPS	:= ${OBJS:.o=.d}

################ Compilation ###########################################

.PHONY: all clean html check distclean maintainer-clean

all:	Config.mk config.h ${NAME}/config.h
ALLTGTS	:= Config.mk config.h ${NAME}/config.h

ifdef BUILD_SHARED
SLIBL	:= $O$(call slib_lnk,${NAME})
SLIBS	:= $O$(call slib_son,${NAME})
SLIBT	:= $O$(call slib_tgt,${NAME})
SLINKS	:= ${SLIBL}
ifneq (${SLIBS},${SLIBT})
SLINKS	+= ${SLIBS}
endif
ALLTGTS	+= ${SLIBT} ${SLINKS}

all:	${SLIBT} ${SLINKS}
${SLIBT}:	${OBJS}
	@echo "Linking $(notdir $@) ..."
	@${LD} -fPIC ${LDFLAGS} $(call slib_flags,$(subst $O,,${SLIBS})) -o $@ $^ ${LIBS}
${SLINKS}:	${SLIBT}
	@(cd $(dir $@); rm -f $(notdir $@); ln -s $(notdir $<) $(notdir $@))

endif
ifdef BUILD_STATIC
LIBA	:= $Olib${NAME}.a
ALLTGTS	+= ${LIBA}

all:	${LIBA}
${LIBA}:	${OBJS}
	@echo "Linking $@ ..."
	@rm -f $@
	@${AR} qc $@ ${OBJS}
	@${RANLIB} $@
endif

$O%.o:	%.cc
	@echo "    Compiling $< ..."
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@${CXX} ${CXXFLAGS} -MMD -MT "$(<:.cc=.s) $@" -o $@ -c $<

%.s:	%.cc
	@echo "    Compiling $< to assembly ..."
	@${CXX} ${CXXFLAGS} -S -o $@ -c $<

include bvt/Module.mk

################ Installation ##########################################

.PHONY:	install uninstall install-incs uninstall-incs

####### Install headers

ifdef INCDIR	# These ifdefs allow cold bootstrap to work correctly
LIDIR	:= ${INCDIR}/${NAME}
INCSI	:= $(addprefix ${LIDIR}/,$(filter-out ${NAME}.h,${INCS}))
RINCI	:= ${LIDIR}.h

install:	install-incs
install-incs: ${INCSI} ${RINCI}
${INCSI}: ${LIDIR}/%.h: %.h
	@echo "Installing $@ ..."
	@${INSTALLDATA} $< $@
${RINCI}: ${NAME}.h
	@echo "Installing $@ ..."
	@${INSTALLDATA} $< $@
uninstall:	uninstall-incs
uninstall-incs:
	@if [ -d ${LIDIR} -o -f ${LIDIR}.h ]; then\
	    echo "Removing ${LIDIR}/ and ${LIDIR}.h ...";\
	    rm -f ${INCSI} ${RINCI};\
	    rmdir ${LIDIR};\
	fi
endif

####### Install libraries (shared and/or static)

ifdef LIBDIR
ifdef BUILD_SHARED
LIBTI	:= ${LIBDIR}/$(notdir ${SLIBT})
LIBLI	:= $(addprefix ${LIBDIR}/,$(notdir ${SLINKS}))
install:	${LIBTI} ${LIBLI}
${LIBTI}:	${SLIBT}
	@echo "Installing $@ ..."
	@${INSTALLLIB} $< $@
${LIBLI}: ${LIBTI}
	@(cd ${LIBDIR}; rm -f $@; ln -s $(notdir $<) $(notdir $@))
endif
ifdef BUILD_STATIC
LIBAI	:= ${LIBDIR}/$(notdir ${LIBA})
install:	${LIBAI}
${LIBAI}:	${LIBA}
	@echo "Installing $@ ..."
	@${INSTALLLIB} $< $@
endif

uninstall:
	@echo "Removing library from ${LIBDIR} ..."
	@rm -f ${LIBTI} ${LIBLI} ${LIBSI} ${LIBAI}
endif

################ Maintenance ###########################################

clean:
	@if [ -d $O ]; then\
	    rm -f ${SLIBT} ${SLINKS} ${OBJS} ${DEPS};\
	    rmdir $O;\
	fi

html:	${SRCS} ${INCS} ${NAME}doc.in
	@${DOXYGEN} ${NAME}doc.in

distclean:	clean
	@rm -f Config.mk config.h config.status ${NAME}

maintainer-clean: distclean
	@if [ -d docs/html ]; then rm -f docs/html/*; rmdir docs/html; fi

INPLACE_INCS := $(addprefix ${NAME}/,$(filter-out config.h,${INCS}))
${INPLACE_INCS}: ${NAME}/%:	${NAME}/config.h
${NAME}/config.h:	config.h
	@echo "    Linking inplace header location ..."
	@rm -f ${NAME}; ln -s . ${NAME}

${OBJS}:		Makefile Config.mk config.h
Config.mk:		Config.mk.in
config.h:		config.h.in
Config.mk config.h:	configure
	@if [ -x config.status ]; then echo "Reconfiguring ..."; ./config.status; \
	else echo "Running configure ..."; ./configure; fi

-include ${OBJS:.o=.d}
