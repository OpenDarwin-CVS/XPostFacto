TIME := .UploadTimeStamps

SOURCE := Compatibility.html FAQ.html L2CacheConfig.html XPostFacto.html XPostFacto3.html index.html

TARGET := $(SOURCE:%=$(TIME)/%.stamp)

all: $(TARGET)

$(TIME)/%.stamp : %
	curl -n -T $< ftp://legacymacs.macsales.com/XPostFacto/$<
	touch $@

XPostFacto% :
	mkdir -p "build/$@"
	ditto -rsrcFork ../build/XPostFacto "build/$@/XPostFacto"
	ditto -rsrcFork XPostFacto3.html "build/$@/XPostFacto.html"
	stuff -f sit5 -n "build/$@.sit" -o -D "build/$@"
