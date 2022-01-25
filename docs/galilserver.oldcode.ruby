#!/usr/bin/ruby
require 'socket'
include Process
#
#We use 'flock' to implement locking so multiple clients can talk to the
#galil over its single communication channel
#
$lockfile = "/tmp/galil.lock"
$statefile = "/home/primefocus/galil.state"
#a global to keep track of forks with each fork incrementing it
$globalclientnum = 1
$moves = Hash.new
myclientnum = 0
#Galil state variables
#
#galil status variables
#a2d
$adbytes = Hash.new
$adbytes["a"] = 98
$adbytes["b"] = 154
$adbytes["c"] = 210
#move in progress
$mipbytes = Hash.new
$mipbytes["a"] = 45
$mipbytes["b"] = 73
$mipbytes["c"] = 101
$mipbytes["e"] = 157
$mipbytes["f"] = 185
$mipbytes["g"] = 213
$mipbytes["h"] = 241
$mip = 128

$adconv = 10.0 / 32767.0
#
#nominal plane array
$npla = Array.new
$npread = false

                                                                                
class Filter
                attr_reader :filtmagcode, :filtshortname, :filtcolor
        def initialize(filtstr)
                fsa = filtstr.split(' ')
                @filtmagcode = fsa[0].to_i
                @filtshortname = fsa[1]
                @filtcolor  = fsa[3]
                @filtercolor = Hash.new
        end
end


class Galil
	def initialize(telescope, instrument, cmdfile, serverport, galiladdress, galilport)
	begin
		puts "Opening socket to galil"
		$galilsock = TCPSocket.new("10.30.3.31", galilport)
		puts "Starting command server"
		@serversock = TCPServer.new( "", serverport)	
		puts "Command server listening"
		rescue SystemCallError =>sce
		e = "#{sce}"
		if /e/.match("Connection refused") != nil
			$stderr.print "Connection refused. Will sleep 30 seconds and retry connecting\n";
			sleep 30
			retry
		end
			$stderr.print "System Call Error: " + $!
		raise
	end
		$cmds = Hash.new;
		rfile = File.new(cmdfile, "r")
		rfile.each_line do |cl|
		cc  = cl.split(':')	
		$cmds[cc[0]] = cc[1].chomp
		end
		$readlist = 0
	end

	def showcmds
		$cmds.each_key {|key| puts "hlc: #{key} llc: #{$cmds[key]}"}
	end

	def sendclient(mycn, clientsock, str)
		puts "sending to client #{mycn}"
		clientsock.send(str, 0)
	end

	def checkgalilstate(mycn)
		r = lsendgalil("DISTA=?;", mycn)
		if /OK:/.match(r) == nil
			puts "Cannot get initial DISTA"
		else
			d = r.split(":")
			$da = d[1].split(":", 2)
			puts "Initial DISTA #{$da}"
		end
		r = lsendgalil("DISTB=?;", mycn)
		if /OK:/.match(r) == nil
			puts "Cannot get initial DISTB"
		else
			d = r.split(":")
			$da = d[1].split(":", 2)
			puts "Initial DISTB #{$da}"
		end
		r = lsendgalil("DISTC=?;", mycn)
		if /OK:/.match(r) == nil
			puts "Cannot get initial DISTC"
		else
			d = r.split(":")
			$da = d[1].split(":", 2)
			puts "Initial DISTC #{$da}"
		end
		r = lsendgalil("FILTVAL=?;", mycn)
		if /OK:/.match(r) == nil
			puts "Cannot get initial FILTVAL"
		else
			d = r.split(":")
			$fv = d[1].split(":", 2)
			puts "Initial FILTVAL #{$fv}"
		end
		r = lsendgalil("REQFILT=?;", mycn)
		if /OK:/.match(r) == nil
			puts "Cannot get initial REQFILT"
		else
			d = r.split(":")
			$rf = d[1].split(":", 2)
			puts "Initial REQFILT #{$rf}"
		end
	end

	
        def lsendgalil(cmd, mycn)
	    puts "In lsendgalil"
            lf = File.new($lockfile)
#first get the lock
              while lf.flock(File::LOCK_EX | File:: LOCK_NB) == false
                       puts "galil locked"
                       sleep 0.1
               end
#now we have the lock
	     try = true
	     tn = 0
	     while try == true
		tn = tn + 1
		if tn > 20
			puts "Too many retries. Bailing out"
#unset the lock
                       lf.flock(File::LOCK_UN)
			return "?: Too many retries. Bailing out"
		end
   	    	 puts "client #{@myclientnum} sending to galil: #{cmd}"
                 puts "try: #{tn}"
             	$galilsock.send(cmd, 0)
                 retv = $galilsock.recv(272, 0)
		rlen = retv.length
		puts "received length #{rlen}"
		if rlen >30
			puts "errant buffer of size #{rlen}. Retrying"
			next
		end
                 #puts "retv from galil: #{retv}"
                 if /\?$/.match(retv) != nil
                         puts "in error branch"
                         $galilsock.send("TC 1;", 0)
                         $error = $galilsock.recv(100)
			 puts "Errorcode: #{$error}"
                 elsif /:/.match(retv) != nil
                         last = retv.length - 2
                         rv = retv.slice(0, last)
                         puts "galil returns OK"
			 try = false
#unset the lock
                         lf.flock(File::LOCK_UN)
                         return "OK: #{rv}"
                  else
			 puts "Unknown error branch"
                  end
		  sleep 0.1
	end
end

	def runserver
		while true do
			puts "waiting for connections from clients"
			newsock = @serversock.accept
			if newsock == nil
				puts "galilserver accept error"
				next
			end
			
			if (cpid = fork) == nil
				myclientnum = $globalclientnum
				puts "got connection from client #{myclientnum}"
				sleep 1
				sendclient(myclientnum, newsock, "OK: Connected to galilserver. Client #{myclientnum}")
				highlevelcmd = ""
				while highlevelcmd != "CLIENTDONE"
					$error = ""
					highlevelcmd = newsock.recv(100)
					puts "got hlc: #{highlevelcmd}"
					h2lc = High2lowcmd.new(highlevelcmd, myclientnum)
					if $error.length != 0
						puts "High-level command Error: #{$error}"
						sendclient(myclientnum, newsock, $error)
					else
						retv = h2lc.dolowlevel
						sendclient(myclientnum, newsock, retv)
					end
				end
				puts "Client finished"
				newsock.close
				exit 0
			else
				puts "This is the parent. Next client #{$globalclientnum}"
				$globalclientnum = $globalclientnum + 1
				newsock.close
			end
		end
	end

	def shutdown
		puts "Closing socket to galil"
		$galilsock.close
		puts "Closing listening server socket"
		@serversock.close
	end

end

class High2lowcmd
	def initialize(hlc, mycn)
		$error = ""
		@myclientnum = mycn
		@hlca = Array.new
		@hlca = hlc.split(' ')
		k = @hlca[0]
		hl = @hlca.length
		(1...hl).each do |i|
			k = k + '_$' + i.to_s
		end
		if $cmds.has_key?(k) == false
			$error = "?: No such high-level command"
			return "#{$error}"
		end
		llc = String.new($cmds[k])
		(1...hl).each do |i|
			p = '$' + i.to_s
			puts "#{p}"
			llc.sub!(p, @hlca[i])
		end
		puts "substituted #{llc}"
		@llca = Array.new
		@llca = llc.split(';')
		@llca.each do |cc|
			puts "cc: #{cc}"
		end
	end

	def dolowlevel
		puts "In dolowlevel: client #{@myclientnum}"
		$error = ""
		cnum = 1
		rrv = ""
		llcc = Lowlevelcmds.new(@myclientnum)
		mc = llcc.mycli
		puts "Got a low-level cmd for client #{mc}"
		@llca.each do |c|
			puts "cnum: #{cnum} doing method #{c}"
			cs = c.split('_')
			meth = cs.shift
			puts "meth: #{meth}"
			x = "#{meth}".intern
			cs.push @myclientnum
			rv = llcc.send(x, cs)
			c = llcc.mycli
			puts "Client is #{c}"
			rrv = rrv + rv
			cnum = cnum + 1
		end
		return rrv
	end
end

class Lowlevelcmds
attr_reader :mycli

	def initialize(mycn)
		@mycli = mycn
		puts "new LowLevel cmd for client #{mycn}"
	end

        def sendgalil(cmd, mycn)
            lf = File.new($lockfile)
#first get the lock
              while lf.flock(File::LOCK_EX | File:: LOCK_NB) == false
                       puts "galil locked"
                       sleep 0.1
               end
#now we have the lock
	     try = true
	     tn = 0
	     while try == true
		tn = tn + 1
		if tn > 20
			puts "Too many retries. Bailing out"
#unset the lock
                       lf.flock(File::LOCK_UN)
			return "?: Too many retries. Bailing out"
		end
   	    	 puts "client #{mycn} sending to galil: #{cmd}"
                 puts "try: #{tn}"
             	$galilsock.send(cmd, 0)
                 retv = $galilsock.recv(272, 0)
		rlen = retv.length
		puts "received length #{rlen}"
                 puts "retv from galil: #{retv}"
		if rlen > 30
			puts "errant buffer of size #{rlen}. Retrying"
			next
		end
                 if /\?$/.match(retv) != nil
                         puts "in error branch"
                         $galilsock.send("TC 1;", 0)
                         $error = $galilsock.recv(100)
			 puts "Errorcode: #{$error}"
                 elsif /:/.match(retv) != nil
                         last = retv.length - 2
                         rv = retv.slice(0, last)
                         puts "galil returns OK"
			 try = false
#unset the lock
                         lf.flock(File::LOCK_UN)
                         return "OK: #{rv}"
                  else
			 puts "Unknown error branch"
                  end
		  sleep 0.1
	end
end

	 def sendgalilreadn(cmd, n)
            lf = File.new($lockfile)
#first get the lock
              while lf.flock(File::LOCK_EX | File:: LOCK_NB) == false
                       puts "galil locked-sendgalilreadn chicken dance"
                       sleep 0.1
               end
#now we have the lock
		try = true
		tn = 0
		while try == true
		tn = tn + 1
		if tn > 20
			puts "Too many retries. Bailing out"
                        lf.flock(File::LOCK_UN)
			return
		end
			puts "try #{tn}"
                        $galilsock.send(cmd, 0)
                        retv = String.new
                        retv = $galilsock.recv(269, 0)
                	if /\*$/.match(retv) != nil
                                 puts retv.length
                       		 puts "in error branch"
                        	$galilsock.send("TC 1;", 0)
	                        $error = $galilsock.recv(100)
				puts "Errorcode: #{$error}"
               		elsif /:$/.match(retv) != nil
	                        last = retv.length - 1
				puts "in good branch"
       		                 rv = retv.slice(0, last)
#unset the lock
                      		lf.flock(File::LOCK_UN)
				try = false
				puts "last #{last}"
       		                 return "OK: #{rv}"
       		         else
				puts "unknown error"
				puts "bad return value #{rv}"
                	end
			sleep 0.1
		end
        end

	def setfilt(filtername)
	end

	def getgalilstatus(ca)
		mycn = ca[0]
		puts "In getgalilstatus for client #{mycn}"
		(1..10).each do |t|
                	$gstat = sendgalilreadn("QR;", 269)
	                gstatlength = $gstat.length
			puts "stattry #{t}"
	                if gstatlength  != 272
       		                 puts "length #{gstatlength} bad"
				t = t + 1
				next;
			else	
				puts "status return OK"
       			         return($gstat)
			end
		end
        end

	def getmips(ca)
		mycn = ca[0]
	        $mipbytes.each do |k, v|
       		         mb = $gstat[v, 1]
       		         mba = mb.unpack("b8")
       		         mbb = mb.unpack("C")
       		         mbbb = ((mbb[0] & $mip) != 0)?1:0
       		         $moves[k] = mbbb
       		         puts "moves: #{k} #{mba[0]} #{mbbb}"
 	       end
	end

def  getaxisinfo(aa)
		a = aa[0]
		mycn = aa[1]
		puts "In getaxisinfo for client #{mycn}"	
		cstr = "QR#{a};"
		astat = sendgalilreadn(cstr, 33)
		s = astat[4,28]
		sl = s.length
		puts "sl #{sl}"
		astatarray = s.unpack("B16 B8 C l l l l l s s")
		astatarray.each do |v|
			puts "v: #{v}"
		end
		return astatarray
	end

	
	def fwmoving(ca)
		mycn = ca[0]
		puts "In fwmoving for client #{mycn}"
		r = getaxisvelocity("F")
		if r == "OK:0"
			return "OK: 0"
		else
			return "OK: 1"
		end
	end

	def lfwmoving(ca)
		mycn = ca[0]
		puts "In lfwmoving for client #{mycn}"
		cstr = "TV F;"
		puts "sending #{cstr} to galil"
		r = sendgalil(cstr, mycn) 
			
		puts "got velocity #{r}"
		return "OK: 0"
	end

	def filtisin(c)
		mycn = c[0]
		ta = Array.new
		ta[0] = "FILTISIN"
		ta[1] = mycn
		puts "In filtisin for client #{mycn}"
		r = read(ta)
		puts "got filtin #{r}"
		return r
	end


	def getclientnum(ca)
		mycn = ca[0]
		puts "In getclientnum for client #{mycn}"
		return "OK: #{mycn}"
	end

	def read(ca)
		c = ca[0]
		cln = ca[1]
		cstr = "#{c}=?;"
		puts "sending #{cstr} to galil"
		puts "in read for client #{cln}"
		resp = sendgalil(cstr, cln)
		return(resp)
	end

	def set(ca)
		mycn = ca[2]
		cstr = "#{ca[0]}=#{ca[1]};"
		puts "In set for client #{mycn}"
		puts "sending #{cstr} to galil"
		resp = sendgalil(cstr, @myclientnum)
		return(resp)
	end

	def sendraw(ca)
		mycn = ca[1]
		cstr = "#{ca[0]};"
		puts "sending #{cstr} to galil"
		puts "In sendraw for client #{mycn}"
		resp = sendgalil(cstr, @myclientnum)
		return resp
	end

	def ex(ca)
		c = ca[0]
		mycn = ca[1]
		puts "in ex for Client #{mycn}"
		cstr = "XQ ##{c};"
		puts "sending #{cstr} to galil"
		resp = sendgalil(cstr, mycn)
		return(resp)
	end

	def zeropos(axislist)
		astr = "ABCDEFGH"
		cstr = "DP "
		(0..7).each do |i|
			a = astr[i, 1]
			puts "#{a}"
			if axislist.index(a) != nil
				cstr = cstr + "0,"
			else
				cstr = cstr + ","
			end
		end
		cstr.chop!
		cstr = cstr + ";"
		puts "sending #{cstr} to galil"
		resp = sendgalil(cstr, @myclientnum)
		return resp
	end

		
	def focind(ca)
		mycn = ca[0]
		cstr = "XQ #FOCIND;"
		puts "In focind for Client #{mycn}"
		resp = sendgalil(cstr, mycn)
		return resp
	end		

	def focall(ca)
		mycn = ca[0]
		puts "In focall for client #{mycn}"
		cstr = "XQ #COORDMV;"
		resp = sendgalil(cstr, mycn)
		return resp
	end		

	def filtin(ca)
		mycn = ca[0]
		puts "In filtin for Client #{mycn}"
		cstr = "XQ #FILTIN,3;"
		resp = sendgalil(cstr, mycn)
		return(resp)
	end

	def filtout(ca)
		mycn = ca[0]
		puts "In filtout for Client #{mycn}"
		cstr = "XQ #FILTOUT;"
		resp = sendgalil(cstr, mycn)
		return(resp)
	end

	def populate(ca)
		mycn = ca[0]
		puts "In populate for client #{mycn}"
		cstr = "XQ #FILTLDM;"
		resp = sendgalil(cstr, mycn)
		return(resp)
	end

	def popdone(ca)
		mycn = ca[0]
		puts "In popdone for client #{mycn}"
		cstr = "HX;"
		resp = sendgalil(cstr, mycn)
		return(resp);
	end

	def filtmov(ca)
		mycn = ca[0]
		puts "In filtmov for client #{mycn}"
		fin = Array.new
		fin[0] = mycn
		r = filtisin(fin)
		puts "filtisin #{r}"
                if r.match("OK: 1") != nil
                        return "?: Cannot rotate wheel with filter in"
		else
			cstr = "XQ #FILTMOV;"
			resp = sendgalil(cstr, mycn)
			ea = Array.new
			ea[0] = "ERRFILT"
			ea[1] = mycn
			erfilt = read(ea)
			if(erfilt == 1)
				puts "Error finding requested filter"
				return("?:")
			end
			return "OK:"
		end
	end

	def sfiltmov(ca)
		mycn = ca[0]
		puts "In sfiltmov for client #{mycn}"
		fin = Array.new
		fin[0] = mycn
		r = filtisin(fin)
		puts "filtisin #{r}"
                if r.match("OK: 1") != nil
                        return "?: Cannot rotate wheel with filter in"
		else
			cstr = "XQ #FILTMOV;"
			resp = sendgalil(cstr, mycn)
			ea = Array.new
			ea[0] = "ERRFILT"
			ea[1] = mycn
			erfilt = read(ea)
			if(erfilt == 1)
				puts "Error finding requested filter"
				return("?:")
			end
			return "OK:"
		end
	end

	def getreqfilt(ca)
		mycn = ca[0]
		puts "In getreqfilt for Client #{mycn}"
		cstr = "REQFILT = ?;"
		r = sendgalil(cstr, mycn)
		if /OK:/.match(r) == nil
			puts "Error getting requested filter"
			return("?: Error getting requested filter")
		else
			d = r.split(":")
			return "OK: #{d[1]}"
		end
	end

	def setreqfilt(codea)
		code = codea[0]
		mycn = codea[1]
		puts "In setreqfilt for Client #{mycn}"
		cstr = "REQFILT=#{code};"
		r = sendgalil(cstr, mycn)
		if /OK:/.match(r) == nil
			puts "Error setting requested filter"
			return "?: Error setting requested filter"
		else
			return "OK: "
		end
	end

	def sloadfilt(codea)
		fn = codea[0]
		mycn = codea[1]
		puts "In sloadfilt for Client #{mycn}"
		if $filthash.has_key?(fn) == false
			puts "no such filter in master list"
			return "?"
		end
		code = $filthash[fn].to_i
		cstr = "REQFILT=#{code};"
		r = sendgalil(cstr, mycn)
		if /OK:/.match(r) == nil
			puts "Error setting requested script filter"
			return "?: Error setting requested filter"
		end
		ca = Array.new
		ca[0] = mycn
		checkfilt = filtisin(ca)
		puts "checkfilt #{checkfilt}"
		if checkfilt != 0
			puts "filter in beam"
		else
			puts "Filter out. OK to move"
		end
		sfiltmov(ca)
		(1..50).each do |e|
			cstr = "REQFILT=?;"
			rr = sendgalil(cstr, ca[0])
			cstr = "FILTVAL=?;"
			fv = sendgalil(cstr, ca[0])
			rra = rr.split(":")
			fva = fv.split(":")
			rreq = rra[1].chomp!
			rval = fva[1].chomp!
			puts "req #{rreq} val #{rval}"
			sleep 1
			if rreq == rval
				puts "got filter"
				break
			end
		end
			filtin(ca)
		return "OK: 0"
	end

	def sunloadfilt(ca)
		mycn = ca[0]
		puts "In sunloadfilt for client #{mycn}"
		filtout(ca)
		return "OK: 0"
	end


	def getfiltval(ca)
		mycn = ca[0]
		puts "In getfiltval for Client #{mycn}"
		cstr = "FILTVAL = ?;"
		r = sendgalil(cstr, mycn)
		if /OK:/.match(r) == nil
			puts "Error getting filtval"
			return("?: Error getting filtval")
		else
			d = r.split(":")
			return "OK: #{d[1]}.to_i"
		end
	end

	def initfw(ca)
		mycn = ca[0]
		puts "In initfw for Client #{mycn}"
		ta = Array.new
		ta[0] = mycn
		r = filtisin(ta)
		puts "filtisin #{r}"
		if /1/.match(r) != nil
			return "?: Cannot initialize filterwheel with filter in beam"
		else
			cstr = "XQ #FILTRD;"
			puts "about to execute FILTRD"
			r = sendgalil(cstr, mycn)
			if /OK:/.match(r) == nil 
				return "?: error initializing filterwheel"
			else
				return "OK: "
			end
		end
	end

	def filtrd(ca)
		mycn = ca[0]
		ta = Array.new
		ta[0] = mycn
		r = filtisin(ta)
		puts "In filtrd for Client #{mycn}"
		r = filtisin(ta)
		puts "filtisin #{r}"
		if /1/.match(r) != nil
			return "?: Cannot read filters with filter in beam"
		else
			cstr = "XQ #FILTRD;"
			puts "about to execute FILTRD"
			r = sendgalil(cstr, mycn)
			if /OK:/.match(r) == nil 
				return "?: error reading filters"
			else
				return "OK: "
			end
		end
	end

	def showfilters(ca)
		mycn = ca[0]
		puts "In showfilters for client #{mycn}"
		lf = Array.new
		lf[0] = "OK:"
		(0..5).each do |l|
			r = sendgalil("FILTVALS[#{l}]=?;", mycn)
			puts "r #{r}"
			if /OK:/.match(r) == nil
				puts "Error reading filter info from galil"
			else
				d = r.split(":")
				puts "d[1] #{d[1]}"
				if $filters[d[1].to_i] == nil
					lf[l + 1] = "Unknown"
				else
					lf[l + 1] = $filters[d[1].to_i]
				end
				puts "lf[#{l + 1}] #{lf[l + 1]}"
			end
		end
			fstr = lf.join(" ")
			return fstr
	end

	def showfiltersandcodes(ca)
		mycn = ca[0]
		puts "In showfiltersandcodes for client #{mycn}"
		lf = Array.new
		lf[0] = "OK:"
		(0..5).each do |l|
			r = sendgalil("FILTVALS[#{l}]=?;", mycn)
			puts "r #{r}"
			if /OK:/.match(r) == nil
				puts "Error reading filter info from galil"
			else
				d = r.split(":")
				puts "d[1] #{d[1]}"
				if $filters[d[1].to_i] == nil
					lf[l + 1] = "Unknown"
				else
					lf[l + 1] = $filters[d[1].to_i]
				end
				puts "lf[#{l + 1}] #{lf[l + 1]}"
			end
			lf[l + 1] = lf[l + 1] + ":" + $filthash[lf[l + 1]].to_s
		end
			fstr = lf.join(" ")
			return fstr
	end
	
	def showloadedfilter(ca)
		mycn = ca[0]
		puts "In showloadedfilter fot client #{mycn}"
		r = sendgalil("FILTVAL = ?;", mycn)
		if /OK:/.match(r) == nil
			puts "Error reading loaded filter from galil"
		else
			d = r.split(":")
			f = $filters[d[1].to_i]
			return "OK: #{f}"
		end
	end	

	def loadfilter(c)
		fname = c[0]	
		if $filthash.has_key?(fname) == nil
			puts "no such filter in master filterlist"
		else
			reqfilt = $filthash[fname].to_i
			puts "requested filter magcode #{reqfilt}"
			r = checkloaded()
			if r
				filtout
				puts "unloaded filter"
			end
			return "OK: "
		end
	end	

def checkloaded()
end

def shownp(ca)
	mycn = ca[0]
	puts "In shownp for Client #{mycn}"
	npfile = "/home/primefocus/90prime/galil/nominalplane.txt"
	if $npread == false
		npf = File.new(npfile, "r")
		if npf == nil
			return "?: Cannot open nominal plane file"	
		else
			npf.each_line do |npl|
				$npla = npl.split("\t", 3)
			end
			$npread = true
		end
	end
	nprr = $npla.join(" ")
	return "OK: #{nprr}"
end
	
		
		
	def showactvals(ca)
		mycn = ca[0]
		puts "In showactvals for client #{mycn}"
		avals = Array.new
		avals[0] = "OK: "
		r = sendgalil("DISTA = ?;", mycn)
		if /OK:/.match(r) == nil
			return "?: error reading actuator A"
		else
			d = r.split(":")
			avals[1] = d[1].to_i
		end	
		r = sendgalil("DISTB = ?;", mycn)
		if /OK:/.match(r) == nil
			return "?: error reading actuator B"
		else
			d = r.split(":")
			avals[2] = d[1].to_i
		end	
		r = sendgalil("DISTC = ?;", mycn)
		if /OK:/.match(r) == nil
			return "?: error reading actuator C"
		else
			d = r.split(":")
			avals[3] = d[1].to_i
		end	
		v = avals.join(" ")
		puts "avals #{v}"
		return(v)
	end

	def setactvals(c)
		av = c[0]
		bv = c[1]
		cv = c[2]
		mycn = c[3]
		puts "In setactvals for client #{mycn}"
		r = "OK:"
		puts "set a #{av} b #{bv} c #{cv}"
		r = sendgalil("DISTA=#{av};", mycn)
		if /OK:/.match(r) == nil
			return "?: Error setting actuator A steps"
		end
		r = sendgalil("DISTB=#{bv};", mycn)
		if /OK:/.match(r) == nil
			return "?: Error setting actuator B steps"
		end
		r = sendgalil("DISTC=#{cv};", mycn)
		if /OK:/.match(r) == nil
			return "?: Error setting actuator C steps"
		end
		return "OK: "
	end

	def setallactvals(c)
		allv = Array.new
		allv[0] = c[0]
		allv[1] = c[0]
		allv[2] = c[0]
		mycn = c[1]
		allv[3] = mycn
		puts "In setallactvals for Client #{mycn}"
		setactvals(allv)
		r = sendgalil("DISTALL=#{c[0]};", mycn)
		if /OK:/.match(r) == nil
			return "?: Error setting DISTALL steps"
		end
		return "OK: "
	end

	def showalllvdtvals(ca)
		mycn = ca[0]
		puts "In showalllvdtvals for client #{mycn}"
		 lvdtvals = Array.new
		 lvdtvals[0] = "OK: "
		ta = Array.new
		ta[0] = mycn
		 gst = getgalilstatus(ta)
		gst1 = gst[4, gst.length]
       	         v = $adbytes["a"]
       	         adb = gst1[v, 2]
       	         adba = adb.unpack("s")
       	         adbv = adba[0] * $adconv
       	         lvdtvals[1]  = adbv.to_s[0,5]
       	         v = $adbytes["b"]
       	         adb = gst1[v, 2]
       	         adba = adb.unpack("s")
       	         adbv = adba[0] * $adconv
       	         lvdtvals[2] = adbv.to_s[0,5]
       	         v = $adbytes["c"]
       	         adb = gst1[v, 2]
       	         adba = adb.unpack("s")
       	         adbv = adba[0] * $adconv
       	         lvdtvals[3] = adbv.to_s[0,5]
               	lvs = lvdtvals.join(" ")
		puts "lvs #{lvs}"
		 return lvs
	end

	def setgfocus(c)
		guidersteps = c[0]
		mycn = c[1]
		puts "In setgfocus for Client #{mycn}"
		r = sendgalil("DISTGCAM=#{guidersteps};", @myclientnum)
		if /OK/.match(r) == nil
			return("?: Error setting guider focus steps")
		else
			return("OK: ")
		end
	end

	def initgcam(ca)
		mycn = ca[0]
		cmd = "XQ #GFWINIT;"
		puts "In initgcam for client #{mycn}"
		r = sendgalil(cmd, mycn)
		return r
	end

	def focusgcam(ca)
		mycn = ca[0]
		cmd = "XQ #FOCGCAM;"
		puts "In focusgcam for client #{mycn}"
		r = sendgalil(cmd, mycn)
		return r
	end

	def setgfiltnum(ca)
		gfiltnum = ca[0]
		mycn = ca[1]
		r = sendgalil("GFILTN=#{gfiltnum};", mycn)
		puts "In setgfilternum for client #{mycn}"
		if /OK/.match(r) == nil
			return "?: Error setting guider filter number"
		else
			return("OK: ")
		end
	end

	def getgfilter(ca)
		mycn = ca[0]
		r = sendgalil("MG GFILTN;", mycn)
		if /OK/.match(r) == nil
			return("?: Error getting guider filter number")
		else
			d = r.split(":")
			gfn = d[1].to_i
			puts "in getgfilter for client #mycn}"
			return "OK: #{gfn}"
		end
	end
			  

	def gfwmov(ca)
		mycn = ca[0]
		c = "XQ #GFWMOV;"
		r = sendgalil(c, mycn)
		puts "in gfwmov for client #{mycn}"
		return r
	end


	def clientdone(ca)
		mycn = ca[0]
		puts "in clientdone for client #{mycn}"
		return "OK: Client #{mycn} Shutting Down"
	end
		
		
end
#
#
#signal handler for SIGCLD to clean up when children exit
trap("SIGCLD") do
pid = Process.wait
	puts "child process pid #{pid} exited"
end
#End class definitions
#Start program

#g =  Galil.new("bok", "90P", "./realcmds.list", 9875, "bokgmc.as.arizona.edu", 9876)
tstr="\033]0;Galilserver\007"
$stdout.write(tstr)
$stdout.flush
g =  Galil.new("", "90prime", "/home/primefocus/90prime/galil/cmds.list", 9874, "10.30.3.31", 9876)
$stathash = Hash.new
$filthash = Hash.new
$filthash["Unknown"] = 256
filtfile = File.new("/home/primefocus/90prime/galil/filters.txt", "r")
$filters = Array.new
$i = 1
filtfile.each_line do |l|
        next if l.match('#')
	l.chomp!
        ff = Filter.new(l)
        fk = ff.filtmagcode.to_i
        sn = ff.filtshortname
        fc = ff.filtcolor
        $filters[fk] = sn
        puts "filter mag code #{fk} shortname #{sn} color #{fc}"
	$filthash[sn] = fk
	$i = $i + 1
end
#make sure we have a lock file
if test(?e, $lockfile) == false
	puts "Need to create lockfile #{$lockfile}"
	lf = File.new($lockfile, File::CREAT|File::TRUNC|File::RDWR, 0644)
else
	puts "Found lockfile #{$lockfile}"
end

g.showcmds
g.checkgalilstate(myclientnum)
g.runserver
g.shutdown
