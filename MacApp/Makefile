.PHONY : patch fixlineendings clean distclean all

TEXTFILES := \( -name '*.cpp' -or -name '*.cp' -or -name '*.c' -or -name '*.h' -or -name '*.r' -or -name '*.xml' \)
DISKIMAGE := "/Volumes/MacApp Release XV.I/Apple C++ Frameworks"
FRAMEWORKS := "Apple C++ Frameworks"
MESSAGE := "Waiting for disk image to mount ..."
EXPANDER := "/Applications/Utilities/StuffIt Expander.app"
WAITFORDISKIMAGE := until [ -d $(DISKIMAGE) ]; do echo $(MESSAGE) && sleep 4; done

# We can't do it in one shot because the behaviour of Expander is a little hard to predict --
# (can't tell when it's finished or whether it will mount the image). So we do it in two.

all :
	if [ -f MacApp_Release_XV.I.smi ]; then make patch; else make MacApp_Release_XV.I.smi; fi

patch : fixlineendings
	bzcat macapp.diff.bz2 | patch -p1 -F0 -d $(FRAMEWORKS) -l

fixlineendings : Apple\ C++\ Frameworks
	find $(FRAMEWORKS) $(TEXTFILES) -print0 | xargs -0 perl -pi -e 's/\r/\n/g'

Apple\ C++\ Frameworks : 
	if [ ! -d $(DISKIMAGE) ]; then hdiutil mount MacApp_Release_XV.I.smi; fi
	@$(WAITFORDISKIMAGE)
	ditto -rsrcFork $(DISKIMAGE) $(FRAMEWORKS)
	-hdiutil unmount MacApp_Release_XV.I.smi

MacApp_Release_XV.I.smi : 
	curl -O --disable-epsv ftp://ftp.apple.com/developer/Tool_Chest/MacApp/MacApp_Release_XV.I.smi.hqx
	open -a $(EXPANDER) MacApp_Release_XV.I.smi.hqx
	@echo "---> Run make again once Stuffit has finished expanding the file <---"

distclean: clean
	-rm MacApp_Release_XV.I.smi

clean:
	-rm -rf "Apple C++ Frameworks"
