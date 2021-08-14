CC               :=gcc
INCLUDE          :=-I.
CFLAGS           :=-Wall $(INCLUDE) `pkg-config gtk+-3.0 --cflags` -export-dynamic
LDFLAGS          := `pkg-config gtk+-3.0 --libs` -export-dynamic
INSTALL_BIN      :=/usr/local/bin
INSTALL_RESOURCE :=$(HOME)/.usbrelay

SHORTCUT_INFO =\
"[Desktop Entry]\n"\
"Name=usbrelay\n"\
"Exec=$(INSTALL_BIN)/usbrelay\n"\
"Terminal=false\n"\
"Type=Application\n"\
"Icon=$(INSTALL_RESOURCE)/images/usbrelay_128x128.png\n"\
"Categories=GTK;Development;"

OBJS += \
usbrelay.o \
serial.o \
util.o

all: usbrelay

#
# glade converter
#
gladeconvert: usb-relay-gui.glade
	xxd -i ./usb-relay-gui.glade >usbrelay-gui.h
	
#
# objects to build
#
%.o: %.c %.h
	@echo 'Building file: $<'
	$(CC) $(CFLAGS) -c $< 
	
#
# building binary
#
usbrelay: gladeconvert $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)


#
# install
#
install:
	mkdir -p $(INSTALL_RESOURCE)
	cp -ar ./images $(INSTALL_RESOURCE)
	@printf "\033[0;32m\033[1mThe binary and shortcut installation needs SUDO permission\n\033[0m"
	sudo cp ./usbrelay $(INSTALL_BIN)
	@echo $(SHORTCUT_INFO)>./usbrelay.desktop
	sudo rm -f /usr/share/applications/usbrelay.desktop
	sudo cp ./usbrelay.desktop /usr/share/applications/usbrelay.desktop

.PHONY: clean	
clean:
	rm -rf *.o usbrelay usbrelay-gui.h

