#
# Copyright (c) 2002 IBM, Inc. 
#
#  File:     resources.mak
#  Location: <xercesc_root>\src\xercesc\util\MsgLoaders\ICU\resources
#
#  Windows nmake makefile for compiling (and packaging) the resources
#  for ICU message loader.
#
#  List of resource files to be built.
#
#    . When adding a resource source (.txt) file for a new locale, 
#           the corresponding .res file must be added to this list, 
#    . AND to the file res-file-list.txt if *dll is to be built
#
#
PKGNAME  = XercescErrMsg
TARGETS  = $(PKGNAME)_root.res $(PKGNAME)_en_US.res  $(PKGNAME)_fr_FR.res
ICUDIR   = ..\..\..\..\..\..\..\icu
GENRB    = $(ICUDIR)\bin\genrb.exe
GENRBOPT = -s. -d. --package-name $(PKGNAME)  

all : $(TARGETS)
	@echo All targets are up to date

clean : 
	-erase $(TARGETS)

$(PKGNAME)_root.res : root.txt
	$(GENRB) $(GENRBOPT) $?
    
$(PKGNAME)_en_US.res : en_US.txt
	$(GENRB) $(GENRBOPT) $?

#
# Note: $(GENRB) $(GENRBOPT) --encoding cp1251 $?
#
$(PKGNAME)_fr_FR.res : fr_FR.txt
	$(GENRB) $(GENRBOPT) $?
    