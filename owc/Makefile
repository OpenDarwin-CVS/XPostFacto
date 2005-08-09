TIME := .UploadTimeStamps

SOURCE := Compatibility.html FAQ.html L2CacheConfig.html XPostFacto3.html index.html Log.html

TARGET := $(SOURCE:%=$(TIME)/%.stamp)

all: $(TARGET)

$(TIME)/%.stamp : %
	scp $< eshop:public_html/XPostFacto/$<
	touch $@

XPostFacto% :
	mkdir -p "build/$@"
	ditto -rsrcFork ../build/app/XPostFacto "build/$@/XPostFacto"
	ditto -rsrcFork XPostFacto3.html "build/$@/XPostFacto.html"

	hdiutil create -ov -srcfolder "build/$@" "build/$@.dmg"
	hdiutil unflatten "build/$@.dmg"
	/Developer/Tools/Rez /Developer/Headers/FlatCarbon/*.r "License.r" -a -o "build/$@.dmg"
	hdiutil flatten "build/$@.dmg"
	hdiutil internet-enable "build/$@.dmg"

	stuff -f sit5 -n "build/$@.sit" -o -D "build/$@"

	scp "build/$@.dmg" "eshop:public_html/XPostFacto/Archive/$@.dmg"
	scp "build/$@.sit" "eshop:public_html/XPostFacto/Archive/$@.sit"
