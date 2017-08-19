from socket import *
import thread, time

conntable = {}
streamvaltable = {}
wlanif = 'NULL'

def scanner (fileobject, linehandler): 
    for line in fileobject.readlines( ):
       if not line: break
       linehandler(line)

def sock_tcp_conn(ipaddr, ipport):
    buf = 1024
    addr = (ipaddr, ipport)

    mysock = socket(AF_INET, SOCK_STREAM)
    mysock.connect(addr)
    return mysock;

def process_ipadd(line):
    global conntable
    i = 0
    addrlist = []
    addrlist = line.split(':') 
    naddr = len(addrlist)
    while i<naddr:  
        ip = addrlist[i].split(',', 1)
        ipa = ip[0].split('=')[1]    # ip adress
        ipp = ip[1].split('=')[1]    # ip port
        print ipa 
        print ipp
	sockhdlr = sock_tcp_conn(ipa, int(ipp))
        conntable[ipa] = sockhdlr
        i=i+1

def process_cmd(line):
    global conntable
    global streamvaltable
    global wlanif
    line = line.rstrip()
    str=line.split ('#')
    if str[0] == '' : 
       return 
    command=str[0].split('!')
    if len(command) != 3:
       print 'incorrect format'
       return 

    ret_data_def = command[2]
    print ret_data_def
    ret_data_def_type = ret_data_def.split(',')
    print 'ret data ' + ret_data_def_type[0].lower()
    if ret_data_def_type[0] == 'STREAMID':
        ret_data_idx = ret_data_def_type[1]
    elif ret_data_def_type[0] == 'INTERFACEID':
        wlanif_def = ret_data_def_type[1]
	print wlanif_def

    toaddr = command[0]
    print toaddr
    capi_run = command[1].strip( )
    capi_elem = command[1].split(',')
    print capi_elem
    if capi_elem[0] == 'traffic_agent_receive_start' or capi_elem[0] == 'traffic_agent_receive_stop' or capi_elem[0] == 'traffic_agent_send' or capi_elem[0] == 'traffic_stop_ping' :
        print 'traffic_agent_receive_start' 
        idx = capi_elem.index('streamID')
        print idx
        capi_elem[idx+1] = streamvaltable[capi_elem[idx+1]] 
        print capi_elem
        capi_run = ','.join(capi_elem)
        print capi_run

    elif capi_elem[0] == 'sta_get_info':
        print 'sta_get_info command'
        capi_elem[2] = wlanif.rstrip('\r\n') 
	capi_run = ','.join(capi_elem)

    elif capi_elem[0] == 'sta_set_encyption':
        print 'sta_set_encyption'
        capi_elem[2] = wlanif 
	capi_run = ','.join(capi_elem)

    elif capi_elem[0] == 'sta_set_ip_config':
        print 'sta_set_ip_config'
        capi_elem[2] = wlanif 
	capi_run = ','.join(capi_elem)
        
    elif capi_elem[0] == 'sta_associate':
        print 'sta_associate'
        capi_elem[2] = wlanif 
	capi_run = ','.join(capi_elem)

    elif capi_elem[0] == 'sta_is_connected':
        print 'sta_associate'
        capi_elem[2] = wlanif 
	capi_run = ','.join(capi_elem)

    elif capi_elem[0] == 'sta_get_ip_config':
        print 'sta_get_ip_config'
        capi_elem[2] = wlanif 
	capi_run = ','.join(capi_elem)

    capi_cmd = capi_run + ' \r\n'
    asock = conntable.get(toaddr)
    asock.send(capi_cmd)
    print 'capi command is -> '+ capi_cmd
    status = asock.recv(1024)
 # Status,Running
    print status 

    status = asock.recv(1024)
    print status
    ss = status.rstrip('\r\n')
    print ss
    stitems = ss.split(',')
    print stitems
    print stitems[1]
    if cmp (stitems[1], 'COMPLELE') == 1 and len(stitems) > 3:
#    if cmp (stitems[1], 'COMPLELE') == 1:
        print 'complete'
        if stitems[2] == 'streamID':
            streamvaltable.setdefault(ret_data_idx, stitems[3]) 
        elif stitems[2] == 'version':
            print 'it is version ' + stitems[3] 
        elif stitems[2] == 'interfaceType':
	    print stitems[5]
	    wlanif = stitems[5]
	    print wlanif
    else:
        print "no complete"

    if capi_elem[0] == 'sta_associate': 
       time.sleep(10)

def process_cmdfile(line):
    i = 0
    line = line.rstrip()
    filelist = []
    filelist = line.split(',')
    nfile = len(filelist)
    while i < nfile:
       print 'command file-> ' + filelist[i] 
       file = open(filelist[i])
       scanner(file, process_cmd)
       file.close()
       i = i+1

#def addconn(theIndex, port)

def firstword(line): 
       str=line.split ('#')
       command=str[0].split('!')
       print command
       if command[0] == 'wfa_control_agent':
          print 'process wfa_control_agent connection info'
	  process_ipadd(command[1])
       elif command[0] == 'wfa_console_tg':
          print 'process wfa_console_tg'
	  process_ipadd(command[1])
       elif command[0] == 'wfa_testbed_sta_tg':
          print 'process wfa_testbed_sta_tg'
	  process_ipadd(command[1])
       elif command[0] == 'wfa_test_commands':
          print 'process wfa_testbed_sta_tg'
	  process_cmdfile(command[1])

       if len(command) == 2:
           print command[1]
