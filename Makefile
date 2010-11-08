PKGS = libebook-1.0
CPPFLAGS = `pkg-config --cflags $(PKGS)`
CFLAGS = -g -Wall -O2
LDFLAGS = `pkg-config --libs $(PKGS)` -O1 -z defs
