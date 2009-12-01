import socket
try:
        import simplejson as json
except ImportError:
        import json


# Helpers

def request(req):
	client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
	client.connect("/tmp/bubba-networkmanager.sock")
	client.send(json.dumps(req))
	ret=json.loads(client.recv(16384))
	client.close()
	
	return ret

def docall(req, dump):
	ret=request(req)
	if ret["status"]:
		if dump:
			print json.dumps( ret, indent=4 )
		return (True,ret)
	else:
		if dump:
			print "Request failed with error '%s'"%ret["error"]
		return (False,"Request failed with error '%s'"%ret["error"])


# API implementeation

def getifcfg(ifnam, dump=False):
	return docall({"cmd":"getifcfg","ifname":ifnam},dump)
		
def getlanif(dump=False):
	return docall({"cmd":"getlanif"},dump)

def getwlanif(dump=False):
	return docall({"cmd":"getwlanif"},dump)


def setlanif(ifnam, dump=False):
	return docall({"cmd":"setlanif", "lanif":ifnam},dump)

def getwanif(dump=False):
	return docall({"cmd":"getwanif"},dump)

def setwanif(ifnam, dump=False):
	return docall({"cmd":"setwanif", "wanif":ifnam},dump)

def getdefaultroute(dump=False):
	return docall({"cmd":"getdefaultroute"},dump)

def getinterfaces(type="", dump=False):
	return docall({"cmd":"getinterfaces", "type":type},dump)

def getroutes(dump=False):
	return docall({"cmd":"getroutes"},dump)

def getns(dump=False):
	return docall({"cmd":"getnameservers"},dump)

def setns(arg, dump=False):
	return docall({"cmd":"setnameservers","resolv":arg},dump)	

def getmtu(ifnam, dump=False):
	return docall({"cmd":"getmtu", "ifname":ifnam},dump)

def setmtu(ifnam,mtu, dump=False):
	return docall({"cmd":"setmtu", "ifname":ifnam,"mtu":mtu},dump)

def setstaticcfg(ifnam, cfg, dump=False):
	return docall({"cmd":"setstaticcfg", "ifname":ifnam,"config":cfg},dump)

def setdynamiccfg(ifnam, cfg, dump=False):
	return docall({"cmd":"setdynamiccfg", "ifname":ifnam,"config":cfg},dump)

def setrawcfg(ifnam, dump=False):
    return docall({"cmd":"setrawcfg", "ifname":ifnam},dump)

def haswlan(dump=False):
	return docall({"cmd":"haswlan"},dump)

def setapif(ifnam, dump=False):
	return docall({"cmd":"setapif", "ifname":ifnam},dump)

def setapcfg(ifnam, cfg, dump=False):
	return docall({"cmd":"setapcfg", "ifname":ifnam,"config":cfg},dump)

def setapssid(ifnam, ssid, dump=False):
	return docall({"cmd":"setapssid", "ifname":ifnam,"ssid":ssid},dump)

def setapmode(ifnam, mode, dump=False):
	return docall({"cmd":"setapmode", "ifname":ifnam,"mode":mode},dump)

def setapchannel(ifnam, channel, dump=False):
	return docall({"cmd":"setapchannel", "ifname":ifnam,"channel":channel},dump)

def setapauthnone(ifnam, dump=False):
	return docall({"cmd":"setapauthnone", "ifname":ifnam},dump)

def setapauthwep(ifnam,cfg, dump=False):
	return docall({"cmd":"setapauthwep", "ifname":ifnam,"config":cfg},dump)

def setapauthwpa(ifnam,cfg, dump=False):
	return docall({"cmd":"setapauthwpa", "ifname":ifnam,"config":cfg},dump)

# Test functions

def setbridgestatic(ifnam):
	print "\n   ----- Get br0 ----- \n"
	getifcfg(ifnam)
	cfg={"address":["192.168.22.1"],
		 "netmask":["255.255.255.0"],
		 "bridge_maxwait":["10"],
		 "bridge_ports":["eth2","eth3"]
		 }
	if setstaticcfg(ifnam, cfg):
		getifcfg(ifnam)
	else:
		print "Failed to update cfg"

def setbridgedynamic(ifnam):
	getifcfg(ifnam)
	cfg={"bridge_maxwait":["0"],"bridge_ports":["eth0","eth1"]}
	if setdynamiccfg(ifnam, cfg):
		getifcfg(ifnam)
	else:
		print "Failed to update cfg"

def setethstatic(ifnam):
	print "\n   ----- Set static %s ----- \n"%ifnam
	getifcfg(ifnam)
	cfg={"address":["192.168.22.1"],"netmask":["255.255.255.0"]}
	if setstaticcfg(ifnam, cfg):
		getifcfg(ifnam)
	else:
		print "Failed to update cfg"

def setethdynamic(ifnam):
	print "\n   ----- Set dynamic %s ----- \n"%ifnam
	getifcfg(ifnam)
	cfg={}
	if setdynamiccfg(ifnam, cfg):
		getifcfg(ifnam)
	else:
		print "Failed to update cfg"

def setraw(ifnam):
	print "\n   ----- Set raw %s ----- \n"%ifnam
	getifcfg(ifnam)
	if setrawcfg(ifnam):
		getifcfg(ifnam)
	else:
		print "Failed to update cfg"

def setapnone(ifnam):
	print "\n   ----- Set auth none c %s ----- \n"%ifnam
	getifcfg(ifnam)
	if setapauthnone(ifnam):
		getifcfg(ifnam)
	else:
		print "Failed to update cfg"

def setapwep(ifnam):
	print "\n   ----- Set auth wep c %s ----- \n"%ifnam
	getifcfg(ifnam)
	cfg={"defaultkey":0,"keys":["12345","456789"]}
	if setapauthwep(ifnam, cfg):
		getifcfg(ifnam)
	else:
		print "Failed to update cfg"

def setapwpa(ifnam):
	print "\n   ----- Set auth wpa c %s ----- \n"%ifnam
	getifcfg(ifnam)
	cfg={"mode":"wpa12","keys":["wpa12345","wpa456789"]}
	if setapauthwpa(ifnam, cfg):
		getifcfg(ifnam)
	else:
		print "Failed to update cfg"

if __name__=='__main__':

	getlanif()

	#getinterfaces()
	#haswlan()
	#setapchannel("wlan0", 13)


	#getifcfg("eth1")
	#getdefaultroute()
	#ns=getns()[1]
	#print ns["resolv"]["search"]
	#print ns["resolv"]["domain"]
	#print ns["resolv"]["servers"]
	#ns["resolv"]["servers"].append(u"1.2.3.4")
	#print ns["resolv"]["servers"]
	#ns["resolv"]["domain"]="mydomain"
	#ns["resolv"]["search"]="mysearch"
	#setns(ns["resolv"]);

	#setapwep("wlan0")
	#setapnone("wlan0")
	#setapwpa("wlan0")
	#setapchannel("wlan0", 1)
	#setapchannel("wlan0", 3)
	#setapchannel("wlan0", 2)
	#setapchannel("wlan0", 6)


	#setapmode("wlan0", "a")
	#setapmode("wlan0", "n")
	#setapmode("wlan0", "b")
	#setapmode("wlan0", "g")

	#setapssid("wlan0", "MittSSID")


	#cfg=getifcfg("wlan0")["config"]["wlan"]
	#cfg["config"]["ssid"]="change"
	#cfg["config"]["mode"]="g"
	#cfg["config"]["channel"]=4
	#print json.dumps(cfg,indent=4)
	#setapcfg("wlan0", cfg)

	#etbridgestatic("br0")
	#setbridgedynamic("br0")

	#setethdynamic("eth0")
	#setethstatic("eth0")

	#getlanif()
	#setlanif("eth1")
	#getlanif()
	#setwanif("eth0")
	#getwanif()




	#print "\n   ----- Get eth1 ----- \n"
	#getifcfg("eth1")


	#getdefaultroute()
	#getroutes()
	#getmtu("eth0")
	#setmtu("eth0", 1500)
	#getmtu("eth0")
	#getmtu("eth1")
