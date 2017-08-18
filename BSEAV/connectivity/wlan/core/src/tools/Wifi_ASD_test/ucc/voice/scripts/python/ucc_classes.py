
class card (name)
       macAddr = '0:0:0:0:0:0'
       ipAddr = '0.0.0.0'
   def __init__(self, name, type, status):
       self.name = name
       self.type  = type 
       self.mode = mode   # independent or infrastructure
       self.status = status 

   def set_ip(self, ip):
       ipAddr = ip;

   def get_ip(self):
       return self.ipAddr;

   def get_mac(self):
       return self.mac

   def record_mac(self, macaddr):
       self.macAddr = macaddr

   def get_type(self)
       return self.type

   def set_mode(self, mode)
       return self.mode = mode

   def get_mode(self)
       return self.mode

   def set_status(self, status)
       self.status = status

   def get_status(self)
       return self.status
       
class capi_command:
    type = 'NONE'
    def __init__(self, cmdString)
        proc_cmd_str(cmdStr)

    def __proc_cmd_str(cmdStr)
        

    def __get__(self, obj, objtype)
        return self.type

    def __set__(self, obj, type)
        self.type = type

class capi_cmd_pingStart(capi_command):
    

      cmd_tg_params = { 'profile':'',
		    'direction:'',
		    'destination':'',
		    'destinationport':-1,
                    'source':'',
		    'sourceport':-1,
                    'framerate':0,
		    'duration':0,
		    'payloadsize':0,
		    'trafficclass':0,
		    'streamid':[0, 0, 0, 0],
		    'startdelay':0,
		    'unmframes':0,
		    'usesyncclock':'FALSE' }

      cmd_
      def __init__(self, cmd_string)

      
           
class Device:
   wlan = card('ath0', 'wlan', 'inactive') 
   usb = card('usb0', 'usb', 'inactive') 
   eth0 = card('eth0', 'eth', 'inactive')

   def __init__(self, device_name):
       self.name = device_name   

   

