SHELL = /bin/sh
DEREZ = /Developer/Tools/Derez
REZ = /Developer/Tools/Rez
SETFILE = /Developer/Tools/SetFile

binaries := OWCLogo.rsrc XPFIcons.rsrc XPFAboutBox.rsrc XPFFatalErrorWindow.rsrc XPFGridWindow.rsrc \
            XPFLogWindow.rsrc XPFOptionsWindow.rsrc XPFProgressWindow.rsrc XPFSplashWindow.rsrc \
	    XPFConfirmDialogs.rsrc XPFUpdateWindow.rsrc XPFRegisterWindow.rsrc XPFVolumeInspectorWindow.rsrc \
	    XPFIconPict.rsrc

sources := $(binaries:.rsrc=.r)

includes := Types.r SysTypes.r ViewTypes.r

.PHONY : usage binary source

usage :
	$(usage_message)

binary : $(binaries)

source : $(sources)

# The rest is magic to define the implicit rules for going in either direction.
# The recursion is necessary to avoid circular dependencies. Well, to avoid
# warnings for circular dependencies--the dependencies are, in fact, circular.
# Which is the main reason that our default target does nothing.

ifdef making_binary

%.rsrc : %.r
	$(REZ) $(includes) $< -i /Developer/Headers/FlatCarbon -o $@
	$(SETFILE) -c RSED -t RSRC $@
else

ifdef making_source

%.r : %.rsrc
	$(DEREZ) $< $(includes) -i /Developer/Headers/FlatCarbon -m 80 > $@

else

# This is the default case. Basically, we force a recursive make which will
# use one or the other of the rules above. But not both, as they are circular.

$(binaries) :
	$(MAKE) making_binary=yes $@

$(sources) :
	$(MAKE) making_source=yes $@

.PHONY : $(binaries) $(sources)

endif

endif

define usage_message
@echo
@echo "Use 'make source' to convert all .rsrc files to .r files."
@echo "Use 'make binary' to convert all .r files to .rsrc files."
@echo
@echo "You can also make a specific file with 'make filename'."
@echo
@echo "CodeWarrior and CVS only uses the .r files. The .rsrc files are for your editing convenience. \
So the idea is that you would make .rsrc files, modify them with an editor like AdLib or IcePick, \
and then make the .r files so that your changes will actually get used."
endef
