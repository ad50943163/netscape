#!gmake
#
# The contents of this file are subject to the Netscape Public License
# Version 1.0 (the "NPL"); you may not use this file except in
# compliance with the NPL.  You may obtain a copy of the NPL at
# http://www.mozilla.org/NPL/
#
# Software distributed under the NPL is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
# for the specific language governing rights and limitations under the
# NPL.
#
# The Initial Developer of this code under the NPL is Netscape
# Communications Corporation.  Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation.  All Rights
# Reserved.

#  

DEPTH		= ../..

include $(DEPTH)/config/config.mk

MODULE		= xp
LIBRARY_NAME	= xp

ifndef MODULAR_NETLIB
CSRCS		= \
		  xp_alloc.c \
		  xp_cntxt.c \
		  xp_core.c \
		  xp_error.c \
		  xp_intl.c \
		  xp_ncent.c \
		  xp_rgb.c \
		  xp_qsort.c \
		  xp_str.c \
		  xp_thrmo.c \
		  xp_time.c \
		  xp_trace.c \
		  xp_wrap.c \
		  xpassert.c \
		  xplocale.c \
		  $(NULL)
else
CSRCS =		  xp_stub.c 
endif

CSRCS +=          \
	 	  xp_file.c \
		  xp_list.c \
		  xp_hash.c \
		  allxpstr.c \
		  $(NULL)

REQUIRES	= js nspr dbm security img util jtools layer java lay style pref rdf

ifeq ($(STAND_ALONE_JAVA),1)
CSRCS		= xp_qsort.c
endif

ifeq ($(MOZ_MAIL_NEWS),1)
CSRCS		+= xp_md5.c
endif

include $(DEPTH)/config/rules.mk

EMACS		= lemacs

AUDIO_FILES	= $(wildcard *.au)
GIF_FILES	= $(wildcard *.gif)
JPEG_FILES	= $(wildcard *.jpg)

GEN_OUT		= $(AUDIO_FILES:.au=.h) $(GIF_FILES:.gif=.h) $(JPEG_FILES:.jpg=.h)

#
# Uncomment these to (re)generate the headers.
#
#GARBAGE		+= $(GEN_OUT) $(wildcard *~)
#
#all:: $(GEN_OUT)

$(LIBRARY): $(OBJS)

#
# Generate c header's for various binary files
#
.SUFFIXES: .au .gif .jpg

.au.h:
	@echo generating $*.h from $<...; \
	CNAME=`echo $*_au`; \
	$(EMACS) -batch -q -l bin-to-c.el -f batch-bin-to-c $$CNAME $< $*.h

.gif.h:
	@echo generating $*.h from $<...; \
	CNAME=`echo $*_gif`; \
	$(EMACS) -batch -q -l bin-to-c.el -f batch-bin-to-c $$CNAME $< $*.h

.jpg.h:
	@echo generating $*.h from $<...; \
	CNAME=`echo $*_jpg`; \
	$(EMACS) -batch -q -l bin-to-c.el -f batch-bin-to-c $$CNAME $< $*.h
