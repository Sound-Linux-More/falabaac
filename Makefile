#  falab - free algorithm lab 
#  Copyright (C) 2012 luolongzhi  罗龙智 (Chengdu, China)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
#  filename: Makefile
#  version : 2.1.0.229
#  time    : 2019/07/14
#  author  : luolongzhi ( falab2012@gmail.com luolongzhi@gmail.com )
#  code URL: https://github.com/Sound-Linux-More/falabaac
#
#  comment : this file is the simple template which will be used in falab,
#            it will be changed according to the project
#

include Makefile.include

INCLUDEDIR = ./include

ifeq ($(ARCH),ARCH_ARM)
LDFLAGS += -L./$(PLIB) -static -l$(PNAME) -lm -lpthread -lrt
else
ifeq ($(DEBUG), Y)
LDFLAGS += -lm -lpthread -lrt 
else
ifeq ($(SOLIB), Y)
LDFLAGS += $(PLIB).so.$(VERSION_MAJOR) -lm -lpthread -lrt
else
LDFLAGS += -L./$(PLIB) -static -l$(PNAME) -lm -lpthread -lrt
endif
endif
endif

ifeq ($(DEBUG), Y)
TARGET      =  $(PNAME)_g
CSRCFILES =  $(shell ls ./frontend/*.c)
CSRCFILES += $(shell ls ./$(PLIB)/*.c)
COBJFILES =  $(patsubst %.c,%.o,$(CSRCFILES))
else 
TARGET      =  $(PNAME)
CSRCFILES   =  $(shell ls ./frontend/*.c)
COBJFILES   =  $(patsubst %.c,%.o,$(CSRCFILES))
endif


SRCFILES    =  $(CSRCFILES)
OBJFILES    =  $(COBJFILES)

CFLAGS      += -I.  -I$(INCLUDEDIR)

all: clean preaction $(OBJFILES)
	@rm $(TARGET) -f
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

preaction:
ifeq ($(DEBUG), Y)
	cd ./$(PLIB)
	@rm *.o -f
else 
	cd ./$(PLIB) && $(MAKE) 
endif
	cd ..

clean : 
	@rm ./$(PLIB)/*.o -f
	@rm ./$(PLIB)/$(PLIB).* -f
	@rm ./frontend/*.o -f
	@rm *.out -f	
	@rm $(PLIB).* -f	
	@rm $(PNAME) -f	
	@rm $(PNAME)_g -f	

install :
	sudo cp ./$(PLIB)/$(PLIB).a $(PREFIX)/lib/
	sudo cp ./$(PLIB)/$(PLIB).so.$(VERSION_MAJOR) $(PREFIX)/lib/
	sudo cp ./include/fa_aacapi.h $(PREFIX)/include/
	sudo cp ./include/fa_inttypes.h $(PREFIX)/include/
	sudo cp ./$(PNAME) $(PREFIX)/bin/

