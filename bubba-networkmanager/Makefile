prefix ?= usr
sbindir ?= $(prefix)/sbin
bindir ?= $(prefix)/bin
datadir ?= $(prefix)/share
sysconfigdir ?= etc

APP=bubba-networkmanager
APP_VERSION=0.1

APP_SRC=main.cpp \
		Dispatcher.cpp

DATAMODEL_SRC=datamodel/Interface.cpp \
			  datamodel/EthernetInterface.cpp \
			  datamodel/WlanInterface.cpp \
			  datamodel/Configuration.cpp \
			  datamodel/BridgeInterface.cpp

CONTROLLER_SRC=\
			   controllers/InterfaceController.cpp \
			   controllers/PolicyController.cpp \
			   controllers/WlanController.cpp

UTIL_SRC=utils/Route.cpp \
		 utils/Resolv.cpp \
		 utils/WlanCfg.cpp \
		 utils/InterfacesCfg.cpp \
		 utils/Sockios.cpp \
		 utils/JsonUtils.cpp \
		 utils/SysConfig.cpp \
		 utils/Nl80211.cpp \
		 utils/Hosts.cpp

CLIENT = bubba-networkmanager-cli
CLIENT_SRC = client.cpp

DHCPPING = dhcpping
DHCPPING_SRC = dhcp-ping/main.cpp

SOURCES = $(APP_SRC) $(DATAMODEL_SRC) $(CONTROLLER_SRC) $(UTIL_SRC)
OBJS = $(SOURCES:%.cpp=%.o)

CXXFLAGS += -g -Wall $(shell pkg-config --cflags libeutils libnl-1) -DPACKAGE_VERSION="\"$(APP_VERSION)\""
LDFLAGS += $(shell pkg-config --libs libeutils libnl-1)

APP_OBJS=$(APP_SRC:%.cpp=%.o)
DATAMODEL_OBJS=$(DATAMODEL_SRC:%.cpp=%.o)
UTIL_OBJS=$(UTIL_SRC:%.cpp=%.o)
CONTROLLER_OBJS=$(CONTROLLER_SRC:%.cpp=%.o)

all: $(APP) $(CLIENT) $(DHCPPING)

$(CLIENT): $(CLIENT_SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^

$(DHCPPING): $(DHCPPING_SRC:%.cpp=%.o)
	$(CXX) $(LDFLAGS) -o $@ $^


$(APP): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:                                                                          
	$(RM) $(APP) $(CLIENT) $(DHCPPING)
	$(RM) $(OBJS) $(CLIENT_SRC:%.cpp=%.o) $(DHCPPING_SRC:%.cpp=%.o)
	$(RM) $(SOURCES:%.cpp=%.d) $(CLIENT_SRC:%.cpp=%.d) $(DHCPPING_SRC:%.cpp=%.d)

install: all 
	install -s $(APP) $(DESTDIR)/$(sbindir)/$(APP)
	install -s $(DHCPPING) $(DESTDIR)/$(sbindir)/$(DHCPPING)
	install -s $(CLIENT) $(DESTDIR)/$(bindir)/$(CLIENT)
	install -d $(DESTDIR)/$(datadir)/$(APP)
	install tz-lc.txt $(DESTDIR)/$(datadir)/$(APP)
	install -T examplecfg/nmconfig $(DESTDIR)/$(sysconfigdir)/$(APP).conf

uninstall:
	$(RM) $(DESTDIR)/$(sbindir)/$(APP)
	$(RM) $(DESTDIR)/$(datadir)/$(APP)/tz-lc.txt
	$(RM) $(DESTDIR)/$(sysconfigdir)/$(APP).conf
	$(RM) $(DESTDIR)/$(bindir)/$(CLIENT)
	$(RM) -r $(DESTDIR)/$(datadir)/$(APP)

depend: $(SOURCES:%.cpp=%.d)
	@echo -n

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:%.cpp=%.d)
endif

%.d: %.cpp
	$(COMPILE.cpp) -MP -MM -MG -E $^ -MF $@

.PHONY: all clean install depend

