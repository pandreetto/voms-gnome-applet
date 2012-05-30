SRCFILES=main.cpp
GLADEDEFS=-I /usr/include/libglademm-2.4 -lglademm-2.4
all: voms-gnome-applet

voms-gnome-applet: $(SRCFILES)
	g++ -o voms-gnome-applet `pkg-config gtkmm-2.4 --cflags --libs` $(GLADEDEFS) $(SRCFILES)

clean:
	rm voms-gnome-applet
