MODULE		= clickpb

EXE				= $(MODULE)

AR				= ar
CC				= g++
RANLIB		= ranlib
JAVA			= javac

STRIP			= strip

DOXYGEN		= doxygen

TARGET_ARCH = linux

INCDIR		= ./src/include
LIBDIR		= ./lib
SRCDIR 		= ./src
BINDIR 		= ./bin
OBJDIR		= ./obj
TESTDIR		= ./tests
DOCDIR		= ./doc/

PREFIX		= /usr/local

DEBUG  		= -g -ggdb 

INCLUDE		= \
						-I$(INCDIR) \
						`pkg-config --cflags jlibcpp` \

LIBRARY 	= \
						-L$(LIBDIR) \
						`pkg-config --libs jlibcpp` \

ARFLAGS		= -rc

CFLAGS		= $(INCLUDE) $(DEBUG) -fPIC -funroll-loops -O2 -DJDEBUG_ENABLED -D__LOCAL_MODULE_NAME__=\"$(MODULE)\" -D__LOCAL_MODULE_PREFIX__=\"$(PREFIX)/$(MODULE)\"

ECHO			= echo

OK 				= \033[30;32mOK\033[m

OBJS			= \
						animation.o\
						camerasettings.o\
						composeframe.o\
						cropframe.o\
						fadeanimation.o\
						framelistener.o\
						gridanimation.o\
						language.o\
						levelframe.o\
						mainframe.o\
						menuframe.o\
						painter.o\
						photoframe.o\
						preferences.o\
						slideanimation.o\
						main.o\
	   
SRCS	= $(addprefix src/,$(OBJS))

all: $(EXE)
	
$(EXE): $(SRCS)
	@$(CC) $(CFLAGS) -o $(EXE) $(SRCS) $(LIBRARY) && $(ECHO) "Generating $(EXE) ...  $(OK)"
	@mkdir -p $(BINDIR) $(BINDIR) && mv $(EXE) $(BINDIR)

.cpp.o: $<  
	@$(CC) $(CFLAGS) -c $< -o $@ && $(ECHO) "Compiling $< ...  $(OK)" 

strip:
	@$(ECHO) "Strip $(EXE)...  $(OK)"
	@$(STRIP) $(BINDIR)/$(EXE)

tests:
	@cd $(TESTDIR) && make && cd .. &&  $(ECHO) "Compiling $< ...  $(OK)" 

doc:
	@mkdir -p $(DOCDIR) 

install:
	@$(ECHO) "Installing $(MODULE) in $(PREFIX)/bin $(OK)"
	@install -o root -g users -m 755 $(BINDIR)/$(EXE) $(PREFIX)/bin
	@install -o root -g users -m 755 ./man/* $(PREFIX)/man/man1
	@install -o root -g users -m 755 -d $(PREFIX)/$(MODULE)
	@$(ECHO) "Installing resources in $(PREFIX)/$(MODULE) $(OK)"
	@install -o root -g users -m 777 -d $(PREFIX)/$(MODULE)/photos
	@cp -r ./res/images $(PREFIX)/$(MODULE)
	@cp -r ./res/sounds $(PREFIX)/$(MODULE)
	@cp -r ./res/styles $(PREFIX)/$(MODULE)
	@cp -r ./res/share $(PREFIX)/$(MODULE)
	@$(ECHO) "Replace configuration files /etc/$(MODULE) $(OK)"
	@install -o root -g users -m 755 -d /etc/$(MODULE)
	@install -o root -g users -m 666 ./res/config/system.conf /etc/$(MODULE)
	@install -o root -g users -m 666 ./res/config/strings.xml /etc/$(MODULE)
	@#install -o root -g users -m 444 ./res/share/default.png $(PREFIX)/$(MODULE)
	@sed -e 's/$$BINARY_PATH/$(subst /,\/,$(PREFIX)/bin/$(EXE))/' -e 's/$$MODULE_PATH/$(subst /,\/,$(PREFIX)/$(MODULE))/' res/share/$(MODULE).desktop > /usr/share/applications/$(MODULE).desktop

uninstall:
	@read -r -p "Remover o '$(MODULE)' pode ocasionar a remoção das fotos. Deseja continuar ? [yN]" CONTINUE; \
	if [ $$CONTINUE = "y" ] || [ $$CONTINUE = "Y" ]; then \
		rm -rf $(PREFIX)/bin/$(EXE) $(PREFIX)/man/man1/$(MODULE).1 /etc/$(MODULE) /usr/share/applications/$(MODULE).desktop $(PREFIX)/$(MODULE); \
	fi

clean:
	@rm -rf $(SRCS) *~ 2> /dev/null && $(ECHO) "$(MODULE) clean $(OK)" 

ultraclean: uninstall clean 
	@find -iname "*.o" -exec rm {} \;;
	@rm -rf $(EXE) $(BINDIR) $(LIBDIR) $(DOCDIR) 2> /dev/null && $(ECHO) "$(MODULE) ultraclean $(OK)" 

