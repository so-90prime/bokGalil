#!/usr/bin/ruby
$nocheck = false
$first = 1
require 'gtk2'
require 'socket'
#signal handler for ^C
trap("SIGINT") do
	$gc.send("CLIENTDONE")
	sleep 1
	exit(0)
end                                                                                    
#GUI State information
#
#state variables stored as "parameter:value", one set per line
#i.e. "fwinitialized:yes" or "loadedfiler:r"
#
$fwguistatefile = "/home/primefocus/fwgui.state"
$fwguistatehash = Hash.new

def readfwguistatefile
	if $state = File.open($fwguistatefile, "r")
       		 puts "Cannot open gui statefile for reading. No state known"
	end
	$state.each_line do |l|
		puts "l #{l}"
		la = l.split(":")
		$fwguistatehash[la[0]] = la[1]
		puts "read GUI state #{la[0]} #{la[1]}"
	end
	$state.close
end

def writefwguistatefile
        if !$state = File.open($fwguistatefile, "w")
                puts "Cannot open fwgui state file for writing. No state saved."
                return(1)
        end
        $fwguistatehash.each_key do |k|
                $state.puts "#{k}:#{$fwguistatehash[k]}"
		puts "writing state #{k}:#{$fwguistatehash[k]}"
        end
	$state.close
end


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
	def initialize(telescope, instrument, serveraddress, serverport)
		#@cs =  TCPSocket.new(serveraddress, serverport)
		@cs =  TCPSocket.new("localhost", serverport)
		rescue SystemCallError =>sce
                e = "#{sce}"
                if /e/.match("Connection refused") != nil
                        $stderr.print "Connection refused. Will sleep 30 seconds and retry connecting\n";
                        sleep 30
                        retry
                end
                        $stderr.print "System Call Error: " + $!
                raise
		puts "Connected to galilserver"
		initcon = @cs.recv(100)
		puts "initcon: #{initcon}"
	end

	def send(s)
		try = 1
		good = false
		while !good
			puts "try #{try}"
			@cs.send(s, 0)
			rbuf = String.new
			rbuf = @cs.recv(300)
			puts "length of rbuf #{rbuf.length}"
			puts "rbuf: #{rbuf}"
			if /\?/.match(rbuf) != nil
				puts "Error: #{$error}"
				try = try + 1
			elsif /OK:/.match(rbuf)
				puts "OK from send"
				good = true
			else
				puts "Bad receive from send"
				try = try + 1
			end
			if try > 20
				puts "Too many retries Bailing"
				rbuf = "?"
				break
			end
		end
		return rbuf
	end

	def nullrecv
		junk = String.new
		junk = @cs.recv(100)
		puts "got junk #{junk}"
	end

	def sendreadn(s, n)
		@cs.send(s, 0)
		cnt = 0
		buf = String.new
		rn = n
		while rn > 0 
			rbuf = String.new
			rbuf = @cs.recv(rn)
			rl = rbuf.length
			buf = buf + rbuf
			rn = rn - rl
			puts "need #{rn} more bytes"
		end
		puts "got all #{n} bytes"
		return buf[4,buf.length]
	end
		
end

class StatusBox < Gtk::Frame
        def initialize(text, value)
                sb = Gtk::VBox.new
                @slabel = Gtk::Label.new(text)
                @svalue = Gtk::Label.new(value)
                sb.pack_start(@svalue)
                sb.pack_start(@slabel)
                super()
                set_shadow_type(Gtk::SHADOW_IN)
                add sb
        end
                                                                                
        def update(new_text, new_label,  color)
             @svalue.set_markup("<span background='#{color}' size='larger'>#{new_text}</span>")
             @slabel.set_markup("<span background='#{color}' size='larger'>#{new_label}</span>")
        end
end
#
$radiobutton = Array.new
#initialize some variables
$unloadflag= false
$loadflag = false
$initialized = false
$lfname = ""
$nmcount = 0

#
#
def makefilterbuttons
#loop to create filter select buttons
#no callbacks yet as we don't know filters
	(1..6).each do |i|
		filtername = "unknown"
       		 if i == 1 then
       		         $radiobutton[i - 1] = Gtk::RadioButton.new("#{filtername} Filter")
       		 else
       		         $radiobutton[i - 1] = Gtk::RadioButton.new($radiobutton[0], "#{filtername} Filter")
       		 end
       		 tooltips = Gtk::Tooltips.new
       		 tooltips.set_tip($radiobutton[i - 1], "Click to select #{filtername} Filter", nil)
       		 puts "doing radiobutton #{i}"
		$vb.pack_start($radiobutton[i - 1], true, true, 0)
	end
end
#
def setfilterbuttons
#callbacks and tooltips for filter select buttons
	$fwguistatehash["selectedfilter"] = "none"
	r = $gc.send("SHOWFILTERS")
	if /OK:/.match(r) == nil
		setnotify(10, "Error getting filternames")
	else
		d = r.split(":")
		$filters = d[1].split(" ")
		(1..6).each do |i|
			filtername = $filters[i - 1]
			$radiobutton[i - 1].set_label("#{filtername} Filter")
       		        tooltips = Gtk::Tooltips.new
       			 tooltips.set_tip($radiobutton[i - 1], "Click to select #{filtername} Filter", nil)
			$radiobutton[i - 1].signal_connect("clicked") do
				puts "setting reqfilt to magcode for #{filtername}"
				fmc = $filterhash[filtername]
				$gc.send("SETREQFILT #{fmc};")
				if /OK:/.match(r) == nil
					setnotify(10, "Error setting reqfilt")
				else
					$fwguistatehash["selectedfilter"] = filtername
					writefwguistatefile
				end
			end
			writefwguistatefile
		end
	end
end

def ogs
#open socket to galilserver
	#$gc = Galil.new("bokap3", "primefocus", "10.30.1.2", 9874)
	$gc = Galil.new("bart", "primefocus", "10.30.1.2", 9874)
	#
	#gobbble up connection message
	$gc.nullrecv
end
#
def cgs
	$gc.send("CLIENTDONE")
end

#program start
$ncount = 0
$aref = 0
$bref = 0
$cref = 0
$alvdt = 0
$blvdt = 0
$clvdt = 0
$stepsperlvdtu = -1.0 / 0.00055 
#byte positions in status word
#for each limit switch
                                                                                
$limswbytes = Hash.new
$limswbytes["a"] = 46
$limswbytes["b"] = 74
$limswbytes["c"] = 102
$limswbytes["e"] = 158
$limswbytes["f"] = 186
$limswbytes["g"] = 214
#
#Hash to store limit switch status
$flimits = Hash.new
$rlimits = Hash.new
#
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
$moves = Hash.new
$moveinprog = Hash.new
#
#encoder positions
$encbytes = Hash.new
$encbytes["a"] = 52
$encbytes["b"] = 80
$encbytes["c"] = 108
#
#a2d
$adbytes = Hash.new
$adbytes["a"] = 98
$adbytes["b"] = 154
$adbytes["c"] = 210
$adconv = 10.0 / 32767.0
#
$radiobutton = Array.new
#read master filter file
#
infofile = File.new("/home/primefocus/90prime/galil/filters.txt", "r")
#
#read state file
readfwguistatefile
#
$filterinfo = Array.new
$filterhash = Hash.new
$loadpos = [-1, 3, 2, 1, 0, 5, 4]
infofile.each_line do |l|
        next if l.match('#')
	ff = Filter.new(l.chomp)
	fk = ff.filtmagcode
	sn = ff.filtshortname
	fc = ff.filtcolor
	$filterinfo[fk] = ff
	$filterhash[sn] = fk
	puts "filter mag code #{fk} shortname #{sn} color #{fc}"
end
#array for loaded filters
#open socket to galil
ogs()
$filters = Array.new
$noupdate = false
#initialize Gtk and set up initial style (colors) for buttons
#
Gtk.init
Gtk::RC.parse_string <<EOS
style "radiobutton"
{
        bg[NORMAL] = {0.5 ,0.5, 0.5}
        bg[PRELIGHT] = {0.85, 0.81, 0.22}
        bg[ACTIVE] = {0.0, 0.5, 0.0}
}
widget_class "*GtkRadioButton" style "radiobutton"
EOS
Gtk::RC.parse_string <<EOS
style "button"
{
        bg[NORMAL] = {0.0 ,0.45, 0.0}
        bg[PRELIGHT] = {0.0, 0.75, 0.0}
        bg[ACTIVE] = {0.0, 1.0, 0.0}
	bg[INSENSITIVE] = {0.4, 0.4, 0.4}
}
widget_class "*GtkButton" style "button"
EOS
Gtk::RC.parse_string <<EOS
style "lbutton" = "button"
{
        bg[NORMAL] = {0.0 ,0.55, 0.4}
	bg[PRELIGHT] = {0.0, 0.75, 0.6}
}
widget "*loadbutton*" style "lbutton"
EOS
#
#
#Gtk.init
$w = Gtk::Window.new
$w.title = "90-Prime Focus and Filterwheel GUI"

$vb = Gtk::VBox.new(false, 0)
$focusbox = Gtk::VBox.new(false, 0)
$setlvdtbox = Gtk::VBox.new(false, 0)
$vsep = Gtk::VSeparator.new
$vsep1 = Gtk::VSeparator.new
$vsep2 = Gtk::VSeparator.new
$vsep3 = Gtk::VSeparator.new
$vsep4 = Gtk::VSeparator.new
$vsep5 = Gtk::VSeparator.new
$vsep6 = Gtk::VSeparator.new
$hsep = Gtk::HSeparator.new
$hsep1 = Gtk::HSeparator.new
$hsep2 = Gtk::HSeparator.new
$hsep3 = Gtk::HSeparator.new
$hsep4 = Gtk::HSeparator.new
$hsep5 = Gtk::HSeparator.new
$hsep6 = Gtk::HSeparator.new
$hsep7 = Gtk::HSeparator.new
$hsep8 = Gtk::HSeparator.new
$hsep9 = Gtk::HSeparator.new
$hsep10 = Gtk::HSeparator.new
$hsep11 = Gtk::HSeparator.new
$hsep12 = Gtk::HSeparator.new
#$dabox = Gtk::HBox.new(false, 0)
$statusbox = Gtk::VBox.new(false, 0)
$encdabox = Gtk::VBox.new(false, 0)
$allbox = Gtk::HBox.new(false, 0)

$w.set_size_request(900, 460)
#$statusbox.set_size_request(20, 350)
$w.signal_connect('delete_event') do
	$gc.send("CLIENTDONE")
	sleep 1
	Gtk.main_quit
end

#
#Label for filter load part of gui
#
$ll = Gtk::Label.new
$ll.set_markup("<b>FILTER LOADING</b>")
$vb.pack_start($ll)
makefilterbuttons
#Select Button
$selbut = Gtk::Button.new("Select")
$selbut.set_name("selectbutton")
seltooltip = Gtk::Tooltips.new
seltooltip.set_tip($selbut, "Click to rotate wheel for loading selected filter ", nil)
$selbut.signal_connect("clicked") do
	$nocheck = false
	$lbut.set_sensitive(false)
	$fwguistatehash["selectedfilter"] = "none"
	r = $gc.send("SHOW FILTISIN")
	if /OK:/.match(r) == nil
		puts "Cannot read filtisin"
	else	
		d = r.split(/:/, 2)
		fin = d[1].to_i
		if fin == 1
			$warnstr = "Cannot rotate with filter in beam!"
			$warncolor = "red"
		else
			$warnstr = "None"
			$warncolor = "grey"
			$gc.send("FILTMOV")
		end
	end
end
#load button
$lbut = Gtk::Button.new("Load")
$lbut.set_name("loadbutton")
ltooltip = Gtk::Tooltips.new
ltooltip.set_tip($lbut, "Click to load current filter into beam", nil)
$lbut.signal_connect("clicked") do
	$loadflag = true
	$fwguistatehash["loaded?"] = "no"
	$gc.send("FILTIN")
	$fwguistatehash["loaded?"] = "yes"
	writefwguistatefile
	$lbut.set_sensitive(false)
	$selbut.set_sensitive(false)
	$ulbut.set_sensitive(true)
	$popbut.set_sensitive(false)
	$popdonebut.set_sensitive(false)
	writefwguistatefile
end
#unload button
$ulbut = Gtk::Button.new("Unload")
ultooltip = Gtk::Tooltips.new
ultooltip.set_tip($ulbut, "Click to unload current filter", nil)
$ulbut.signal_connect("clicked") do
	$fwguistatehash["loaded?"] = "no"
	writefwguistatefile
	$unloadflag = true
	$gc.send("FILTOUT")
	$ulbut.set_sensitive(false)
	$selbut.set_sensitive(true)
	$lbut.set_sensitive(true)
	$popbut.set_sensitive(true)
        puts "in handler"
end
$ulbut.set_sensitive(false)
$vb.pack_start($selbut, true, true, 0)
$vb.pack_start($lbut, true, true, 0)
$vb.pack_start($ulbut, true, true, 0)
#
#
#read filters button
$filtrdbut = Gtk::Button.new("Read Filters")
$filtrdbut.set_name("filtrdbutton")
filtrdtooltip = Gtk::Tooltips.new
filtrdtooltip.set_tip($filtrdbut, "Click to read filter codes", nil)
$filtrdbut.signal_connect("clicked") do
	$fwguistatehash["filtersread"] = "no"
	$fwguistatehash["filterlist"] = ""
        puts "reading filters"
	filtrd
	fl = ""
	(0..5).each do |e|
		if $filters[e] != nil
			fl = fl + $filters[e]
			if e < 5
				fl = fl + "-"
			end
		end
	end
	$fwguistatehash["filterlist"] = fl
	$fwguistatehash["filtersread"] = "yes"
	writefwguistatefile
end
#initialize filterwheel button
$ibut = Gtk::Button.new("Initialize")
$ibut.set_name("initializebutton")
itooltip = Gtk::Tooltips.new
itooltip.set_tip($ibut, "Click to initialize filterwheel", nil)
$ibut.signal_connect("clicked") do
	r = $gc.send("SHOW FILTISIN")
	puts "filtisin #{r}"
#
		$fwguistatehash["initialized"] = "no"
		initfw
		setfilterbuttons
		$radiobutton[3].clicked
		$savelast = 3
		$fwguistatehash["initialized"] = "yes"
		writefwguistatefile
end
#populate button
$popbut = Gtk::Button.new("Populate")
poptooltip = Gtk::Tooltips.new
poptooltip.set_tip($popbut, "Click to populate filterwheel", nil)
$popbut.signal_connect("clicked") do
	r = $gc.send("SHOW FILTISIN")
	if /OK:/.match(r) == nil
		puts "Cannot read filtisin"
		return
	else	
		d = r.split(/:/, 2)
		fin = d[1].to_i
		if fin == 1
			$warnstr = "Cannot populate with filter in beam!"
			$warncolor = "red"
			$popbut.set_sensitive(false)
			$popdonebut.set_sensitive(false)
			return
		else
			populate
			$popdonebut.set_sensitive(true)
			$popbut.set_sensitive(false)
		end
	end
end
#disable populate button if filter in beam
r = $gc.send("SHOW FILTISIN")
if /OK:/.match(r) == nil
	puts "Cannot read filtisin"
else	
	d = r.split(/:/, 2)
	fin = d[1].to_i
	if fin == 1
		$popbut.set_sensitive(false)
	else
		$popbut.set_sensitive(true)
	end
end
#populate done button
$popdonebut = Gtk::Button.new("Populate Done")
popdonetooltip = Gtk::Tooltips.new
popdonetooltip.set_tip($popdonebut, "Click when done populating", nil)
$popdonebut.signal_connect("clicked") do
	popdone
	$popbut.set_sensitive(true)
	$popdonebut.set_sensitive(false)
end
$popdonebut.set_sensitive(false)
$vb.pack_start($ibut, true, true, 0)
$vb.pack_start($filtrdbut, true, true, 0)
$vb.pack_start($popbut, true, true, 0)
$vb.pack_start($popdonebut, true, true, 0);
#
#
$rfbut = Gtk::Button.new("Restore\nFocus")
$rfbut.signal_connect("clicked") do
	restorefocus
end

def restorefocus
	$rfflag = true
	adiff = $aref.to_f - $alvdt.to_f
	bdiff = $bref.to_f - $blvdt.to_f
	cdiff = $cref.to_f - $clvdt.to_f
	asteps = adiff * $stepsperlvdtu
	bsteps = bdiff * $stepsperlvdtu
	csteps = cdiff * $stepsperlvdtu
	cstr =  "SETACTVALS #{asteps.to_i} #{bsteps.to_i} #{csteps.to_i}"
	puts "will do command #{cstr}"
	r = $gc.send(cstr)
	puts "galil returns #{r} from SETACTVALS command"
	cstr1 = "FOCIND"
	r = $gc.send(cstr1)
	puts "galil returns #{r} from FOCIND command"
	setas()
end
#
#Label for setlvdt part of gui
#
$setlvdtl = Gtk::Label.new
$setlvdtl.set_markup("<b>GO TO\nLVDT VALS\n/1000</b>")
#SpinButtons for setting lvdt inputs
#
$alvdtsav = Gtk::SpinButton.new(-200, 2900, 2)
$alvdtlab = Gtk::Label.new("Set A LVDT")
#
#
$blvdtsav = Gtk::SpinButton.new(-200, 2900, 2)
$blvdtlab = Gtk::Label.new("Set B LVDT")
#
#
$clvdtsav = Gtk::SpinButton.new(-200, 2900, 2)
$clvdtlab = Gtk::Label.new("Set C LVDT")
#
#
#Spin buttobn for lvdt tolerence
$tollvdtsav = Gtk::SpinButton.new(1.0, 10.0, 1.0)
$tollvdtlab = Gtk::Label.new("Set lvdt tolerence")
#Label for Focus part of gui
#
$fl = Gtk::Label.new
$fl.set_markup("<b>FOCUS</b>")
#SpinButtons for setting actuator steps
#
$afocusav = Gtk::SpinButton.new(-1000, 1000, 2)
$alab = Gtk::Label.new("Set A Steps")
#
#
$bfocusav = Gtk::SpinButton.new(-1000, 1000, 2)
$blab = Gtk::Label.new("Set B Steps")
#
#
$cfocusav = Gtk::SpinButton.new(-1000, 1000, 2)
$clab = Gtk::Label.new("Set C Steps")
#
#
#Actuator lock button
#
$lockbut = Gtk::CheckButton.new("Lock")
#
#get client number
r = $gc.send("GETCLIENTNUM")
ra = r.split(/:/)
puts "I am client #{ra[1]}"
#get initial values set for actuator steps
r = $gc.send("SHOW DISTA")
if /OK/.match(r) == nil
	puts "Error reading actuator A step setting"
else
	d = r.split(/:/, 2)
	$afocusav.value= d[1].to_i
	puts "A init #{$afocusav.value}"
end
r = $gc.send("SHOW DISTB")
if /OK/.match(r) == nil
	puts "Error reading actuator B step setting"
else
	d = r.split(/:/, 2)
	$bfocusav.value= d[1].to_i
	puts "B init #{$bfocusav.value}"
end
r = $gc.send("SHOW DISTC")
if /OK/.match(r) == nil
	puts "Error reading actuator C step setting"
else
	d = r.split(/:/, 2)
	$cfocusav.value= d[1].to_i
	puts "C init #{$cfocusav.value}"
end
#
#Callbacks for actuator buttons
#
$afocusav.signal_connect("value-changed") do
	if $lockbut.active? == true then
		$lv = $afocusav.value
		$bfocusav.value=($lv)
		$cfocusav.value=($lv)
	end
end

$bfocusav.signal_connect("value-changed") do
	if $lockbut.active? == true then
		$lv = $bfocusav.value
		$afocusav.value=($lv)
		$cfocusav.value=($lv)
	end
end
$cfocusav.signal_connect("value-changed") do
	if $lockbut.active? == true then
		$lv = $cfocusav.value
		$afocusav.value=($lv)
		$bfocusav.value=($lv)
	end
end
#
def setav
	$aactsteps = $afocusav.value
	puts "setav Actuator A steps set to #{$aactsteps}"
	$bactsteps = $bfocusav.value
	puts "setav Actuator B steps set to #{$bactsteps}"
	$cactsteps = $cfocusav.value
	puts "setav Actuator C steps set to #{$cactsteps}"
	r = $gc.send("SET DISTA #{$aactsteps}")
	if /OK/.match(r) == nil
		puts "Error setting DISTA"
	end
	r = $gc.send("SET DISTB #{$bactsteps}")
	if /OK/.match(r) == nil
		puts "Error setting DISTB"
	end
	r = $gc.send("SET DISTC #{$cactsteps}")
	if /OK/.match(r) == nil
		puts "Error setting DISTC"
	end
end

#focus button
fbut = Gtk::Button.new("Focus")
fbut.signal_connect("clicked") do
	$focusflag = true
	$aactsteps = $afocusav.value
	puts "Foc Actuator A steps set to #{$aactsteps}"
	$bactsteps = $bfocusav.value
	puts "Foc Actuator B steps set to #{$bactsteps}"
	$cactsteps = $cfocusav.value
	puts "Foc Actuator C steps set to #{$cactsteps}"
	r = $gc.send("SET DISTA #{$aactsteps}")
	if /OK/.match(r) == nil
		puts "Error setting DISTA"
	end
	r = $gc.send("SET DISTB #{$bactsteps}")
	if /OK/.match(r) == nil
		puts "Error setting DISTB"
	end
	r = $gc.send("SET DISTC #{$cactsteps}")
	if /OK/.match(r) == nil
		puts "Error setting DISTC"
	end
	r = $gc.send("RUN FOCIND")
	if /OK/.match(r) == nil
		puts "Error moving focus actuators"
	end
end
#
#nominal plane button
$npbut = Gtk::Button.new("Nominal\nPlane")
$npbut.signal_connect("clicked") do
		nplane
end

def nplane
	npfile = File.open("/home/primefocus/90prime/galil/nominalplane.txt", "r")
	npdata = npfile.readlines
	nparray = npdata[0].split(" ")
	npa = nparray[0]
	npb = nparray[1]
	npc = nparray[2]
	puts "nominal plane values: #{npa} #{npb} #{npc}"
	newa = npa.to_f + $blvdt.to_f
	newc = npc.to_f + $blvdt.to_f
	adiff = newa - $alvdt.to_f
	bdiff = 0.0
	cdiff = newc - $clvdt.to_f
	asteps = adiff * $stepsperlvdtu
	bsteps = 0
	csteps = cdiff * $stepsperlvdtu
	puts "asteps #{asteps} bsteps #{bsteps} csteps #{csteps}"
	cstr =  "SETACTVALS #{asteps.to_i} #{bsteps.to_i} #{csteps.to_i}"
        puts "will do command #{cstr}"
        r = $gc.send(cstr)
        puts "galil returns #{r} from SETACTVALS command"
	cstr1 = "FOCIND"
	r = $gc.send(cstr1)
	puts "galil returns #{r} from FOCIND command"
	setas()
end



#go to set lvdt values button
#
$golvdtbut = Gtk::Button.new("Go To\nSet lvdts")
$golvdtbut.signal_connect("clicked") do
	golvdt
end

def golvdt
	al = $alvdtsav.value.to_f / 1000.0
	bl = $blvdtsav.value.to_f / 1000.0
	cl = $clvdtsav.value.to_f / 1000.0
	tol = $tollvdtsav.value.to_i
	puts "will go to lvdt values a #{al} b #{bl} c #{cl} with tolerence #{tol}"
	lp = 0
	notthere = true
	while notthere
		puts "lp #{lp}"
		getgalilstatus()
		getad()
		alvdtdiff = al - $alvdt.to_f
		blvdtdiff = bl - $blvdt.to_f
		clvdtdiff = cl - $clvdt.to_f
		alvdtsteps = alvdtdiff * $stepsperlvdtu
		blvdtsteps = blvdtdiff * $stepsperlvdtu
		clvdtsteps = clvdtdiff * $stepsperlvdtu
		cstr =  "SETACTVALS #{alvdtsteps.to_i} #{blvdtsteps.to_i} #{clvdtsteps.to_i}"
		puts "will do command #{cstr}"
		r = $gc.send(cstr)
		puts "galil returns #{r} from SETACTVALS command"
		cstr1 = "FOCIND"
		r = $gc.send(cstr1)
		puts "galil returns #{r} from FOCIND command"
		getgalilstatus()
		lp = lp + 1
		sleep 1.0
		if ( (alvdtsteps.abs  <= tol) and (blvdtsteps.abs <= tol) and (clvdtsteps.abs <= tol) )
			notthere = false
		end
		if (lp > 10)
			puts "quitting after 10 iterations"
			return
		end
	end
end

#set reference button
$setrefbut = Gtk::Button.new("Set\nReference")
$setrefbut.signal_connect("clicked") do
	setref
end
#
#Restore State update
$restorestatebut = Gtk::Button.new("Restore\nState")
$restorestatebut.signal_connect("clicked") do
	restorestate
end

def toggleupdate
	$noupdate = $noupdate?false:true
	if $noupdate
		puts "gui update off"
	else
		puts "gui update on"
	end
end

def restorestate
	readfwguistatefile
	puts "read state:"
	$fwguistatehash.each_key do |k|
		puts "#{k} #{$fwguistatehash[k]}"
	end
	ky = "initialized"
	puts "INIT #{$fwguistatehash[ky]}"
	if /yes/.match($fwguistatehash[ky])

		puts "IF"
		$initialized = true
		$initstat.update("Wheel Initialized?", "Yes", "#FFFF00")
	end
	ky = "filterlist"
	if ky.length >5
		kya = ky.split("-")
		(0..5).each do |e|
			$filters[e] = kya[e]
		end
		setfilterbuttons
	end
		
end

$quitbut = Gtk::Button.new("QUIT GUI")
$quitbut.signal_connect("clicked") do
	quitgui
end
def quitgui
	$gc.send("CLIENTDONE")
	sleep 1
	exit 0
end
	
def setref
	$aref = $alvdt
	$bref = $blvdt
	$cref = $clvdt
end
#pack set lvdt part of gui
$setlvdtbox.pack_start($setlvdtl, true, true, 0)
$setlvdtbox.pack_start($hsep12, true, true, 0)
$setlvdtbox.pack_start($alvdtlab, true, true, 0)
$setlvdtbox.pack_start($alvdtsav, true, true, 0)
$setlvdtbox.pack_start($blvdtlab, true, true, 0)
$setlvdtbox.pack_start($blvdtsav, true, true, 0)
$setlvdtbox.pack_start($clvdtlab, true, true, 0)
$setlvdtbox.pack_start($clvdtsav, true, true, 0)
$setlvdtbox.pack_start($tollvdtlab, true, true, 0)
$setlvdtbox.pack_start($tollvdtsav, true, true, 0)
$setlvdtbox.pack_start($golvdtbut, true, true, 0)

#pack focus part of gui
$focusbox.pack_start($fl, true, true, 0)
$focusbox.pack_start($alab, true, true, 0)
$focusbox.pack_start($afocusav, true, true, 0)
$focusbox.pack_start($blab, true, true, 0)
$focusbox.pack_start($bfocusav, true, true, 0)
$focusbox.pack_start($clab, true, true, 0)
$focusbox.pack_start($cfocusav, true, true, 0)
$focusbox.pack_start($lockbut, true, true, 0)
$focusbox.pack_start(fbut, true, true, 0)
$focusbox.pack_start($npbut, true, true, 0)
$focusbox.pack_start($rfbut, true, true, 0)
$focusbox.pack_start($setrefbut, true, true, 0)
$focusbox.pack_start($restorestatebut, true, true, 0)
$focusbox.pack_start($quitbut, true, true, 0)
#
#Status part of GUI
#
#Label for Status part of gui
#
$sl = Gtk::Label.new
$sl.set_markup("<b>FILTERWHEEL STATUS</b>")
#Filter_in image
#
$filtinstatpb = Gdk::Pixbuf.new("/home/primefocus/filterin.png", 84, 54)
$filtoutstatpb = Gdk::Pixbuf.new("/home/primefocus/filterout.png", 84, 54)
$wheelmovingstatpb = Gdk::Pixbuf.new("/home/primefocus/wheelmoving.png", 84, 54)
#
$nofiltinstatpb = Gdk::Pixbuf.new("/home/primefocus/nofilterin.png", 84, 54)
$nofiltoutstatpb = Gdk::Pixbuf.new("/home/primefocus/nofilterout.png", 84, 54)
$wheelnotmovingstatpb = Gdk::Pixbuf.new("/home/primefocus/wheelnotmoving.png", 84, 54)
#
$filtinstat = Gtk::Image.new($nofiltinstatpb)
$filtoutstat = Gtk::Image.new($filtoutstatpb)
$wheelmovingstat = Gtk::Image.new($wheelnotmovingstatpb)
#
#initialize status box
$initstat = StatusBox.new("Wheel initialized?", "No")
$jamstat = StatusBox.new("Current Filter Status", "?")
$jamstat.update("Current Filter Status", "?", "red")
$selstat = StatusBox.new("SELECTED FILTER", "?")
$selstat.update("SELECTED FILTER", "?", "red")
$notifystat = StatusBox.new("Warnings", "None")
$notifystat.update("Warnings", "None", "grey")
$initstat.update("Wheel initialized?", "NO", "red")
#Pack status widgets
$statusbox.pack_start($sl, true, true, 0)
$statusbox.pack_start($filtinstat, true, true, 0)
$statusbox.pack_start($filtoutstat, true, true, 0)
$statusbox.pack_start($wheelmovingstat, true, true, 0)
$statusbox.pack_start($initstat, true, true, 0)
$statusbox.pack_start($jamstat, true, true, 0)
$statusbox.pack_start($selstat, true, true, 0)
$statusbox.pack_start($notifystat, true, true, 0)
#
#encoder and a/d widgets
$eadl = Gtk::Label.new
$eadl.set_markup("<b>ENCODERS and LVDTS</b>")
$heabox = Gtk::HBox.new
$hebbox = Gtk::HBox.new
$hecbox = Gtk::HBox.new
#
$lvdta = StatusBox.new("A", "0")
$lvdtaref = StatusBox.new("A ref", "0")
$lvdtalim = StatusBox.new("A limit", " ")
$enca = StatusBox.new("A encoder", "0")
$lvdtb = StatusBox.new("B", "0")
$lvdtbref = StatusBox.new("B ref", "0")
$lvdtblim = StatusBox.new("A limit", " ")
$encb = StatusBox.new("B encoder", "0")
$lvdtc = StatusBox.new("C", "0")
$lvdtcref = StatusBox.new("C ref", "0")
$lvdtclim = StatusBox.new("C limit", " ")
$encc = StatusBox.new("C encoder", "0")
#
$heabox.pack_start($lvdta, true, true, 0)
$heabox.pack_start($lvdtaref, true, true, 0)
$heabox.pack_start($lvdtalim, true, true, 0)
$heabox.pack_start($enca, true, true, 0)
$hebbox.pack_start($lvdtb, true, true, 0)
$hebbox.pack_start($lvdtbref, true, true, 0)
$hebbox.pack_start($lvdtblim, true, true, 0)
$hebbox.pack_start($encb, true, true, 0)
$hecbox.pack_start($lvdtc, true, true, 0)
$hecbox.pack_start($lvdtcref, true, true, 0)
$hecbox.pack_start($lvdtclim, true, true, 0)
$hecbox.pack_start($encc, true, true, 0)
#pack encoder and A/D widgets
$encdabox.pack_start($eadl, true, true, 0)
$encdabox.pack_start($heabox, true, true, 0)
$encdabox.pack_start($hebbox, true, true, 0)
$encdabox.pack_start($hecbox, true, true, 0)
#pack various parts of GUI
$allbox.pack_start($focusbox, false, false, 0)
$allbox.pack_start($vsep, false, false, 0)
$allbox.pack_start($vsep1, false, false, 0)
$allbox.pack_start($vb, false, false, 0)
$allbox.pack_start($vsep2, false, false, 0)
$allbox.pack_start($vsep3, false, false, 0)
$allbox.pack_start($statusbox, true, true, 0)
$allbox.pack_start($vsep4, false, false, 0)
$allbox.pack_start($vsep5, false, false, 0)
$allbox.pack_start($encdabox, true, true, 0)
$allbox.pack_start($vsep6, true, true, 0)
$allbox.pack_start($setlvdtbox, false, false, 0)
$w.add($allbox)
$w.show_all

$warnstr = "none"
$warncolor = "grey"
#close socket after initialization before entering loop
timeout_id = Gtk.timeout_add(3000) do
	update
end

def update
	if($first == 1)
		puts "about to do first update"
		$first = 0;
	end
#	$warnstr = "none"
#	$warncolor = "grey"
	if($noupdate)
		return true
	end
	getgalilstatus()
        puts "got status"
	getlimitswitches()
	setlimitdisplay()
	getmips()
	getenc()
	getref()
	getjam()
	getsel()
	oktoload()
	getad()
	getsw()
	setstat()
	notify()
 	#setas()
	#setav()
	setwarn()
        puts "done update"
	return true
end

def setstat
	if $moves["f"] == 1
		puts "moves f"
		$wheelmovingstat.set_pixbuf($wheelmovingstatpb)
		$ibut.set_sensitive(false)
		$filtrdbut.set_sensitive(false)
	else
		$wheelmovingstat.set_pixbuf($wheelnotmovingstatpb)
		$ibut.set_sensitive(true)
		$filtrdbut.set_sensitive(true)
	end
	r = $gc.send("SHOW FILTISIN")
	puts "doing show filtisin"
	rr = r.split(/:/, 2)
	puts "rr rr[0] rr[1] #{rr} #{rr[0]} #{rr[1]}"
	if rr[1].to_i == 0
		puts "got a 0 here #{rr[1]}"
		$filtinstat.set_pixbuf($nofiltinstatpb)
		$filtoutstat.set_pixbuf($filtoutstatpb)
		$ibut.set_sensitive(true)
		$filtrdbut.set_sensitive(true)
		#$lbut.set_sensitive(true)
		$ulbut.set_sensitive(false)
	else
		puts "got a non-zero here #{rr[1]}"
		$filtinstat.set_pixbuf($filtinstatpb)
		$filtoutstat.set_pixbuf($nofiltoutstatpb)
		$ibut.set_sensitive(false)
		$filtrdbut.set_sensitive(false)
		$lbut.set_sensitive(false)
		$ulbut.set_sensitive(true)
	end
	if($initialized == true)
		$initstat.update("Wheel initialized?", "Yes", "green")
	else
		$initstat.update("Wheel initialized?", "No", "red")
	end
end

def getgalilstatus
	$gstat = $gc.sendreadn("STATUS", 272)
	gl = $gstat.length
	puts "gl #{gl}"
end

 def getlimitswitches
	$limswbytes.each do |k, v|
		lsb = $gstat[v, 1]
		lsba = lsb.unpack("b8")
		lsba0 = lsba[0]
		$rlimits[k] = lsba0[2,1].to_i
		$flimits[k] = lsba0[3,1].to_i
	end
end

def setlimitdisplay
	okcolor = "#00ff00"
        if $flimits["a"] == 0
                $lvdtalim.update("A lim", "Forward", "red")
		$warnstr  = "Actuator Limit"
		$warncolor = "yellow"
        elsif $rlimits["a"] == 0
                $lvdtalim.update("A lim", "Reverse", "red")
		$warnstr = "Actuator Limit"
		$warncolor = "yellow"
        else
                $lvdtalim.update("A lim", "none", okcolor )
		$warnstr = "none"
		$warncolor = okcolor
	end
        if $flimits["b"] == 0
                $lvdtblim.update("B lim", "Forward", "red")
		$warnstr = "Actuator Limit"
		$warncolor = "yellow"
        elsif $rlimits["b"] == 0
                $lvdtblim.update("B lim", "Reverse", "red")
		$warnstr = "Actuator Limit"
		$warncolor = "yellow"
        else
                $lvdtblim.update("B lim", "none", okcolor )
		$warnstr = "none"
		$warncolor = okcolor
	end
        if $flimits["c"] == 0
                $lvdtclim.update("C lim", "Forward", "red")
		$warnstr = "Actuator Limit"
		$warncolor = "yellow"
        elsif $rlimits["c"] == 0
                $lvdtclim.update("C lim", "Reverse", "red")
		$warnstr = "Actuator Limit"
		$warncolor = "yellow"
        else
                $lvdtclim.update("C lim", "none", okcolor )
		$warnstr = "none"
		$warncolor = okcolor
	end
end


def getmips
	$mipbytes.each do |k, v|
		mb = $gstat[v, 1]
		mba = mb.unpack("b8")
		mbb = mb.unpack("C")
		mbbb = ((mbb[0] & $mip) != 0)?1:0
		$moves[k] = mbbb
		puts "moves: #{k} #{mba[0]} #{mbbb}"
	end
end

def getenc
		okcolor = "#00ff00"
		v = $encbytes["a"]
		encbb = $gstat[v, 4]
		encv = encbb.unpack("i")
		encav = encv[0]
		$enca.update("A encoder", encav.to_i , okcolor)
		v = $encbytes["b"]
		encbb = $gstat[v, 4]
		encv = encbb.unpack("i")
		encav = encv[0]
		$encb.update("B encoder", encav.to_i, okcolor)
		v = $encbytes["c"]
		encbb = $gstat[v, 4]
		encv = encbb.unpack("i")
		encav = encv[0]
		$encc.update("C encoder", encav.to_i, okcolor)
end

def getref
	okcolor = "#00ff00"
	$lvdtaref.update("A Ref", $aref, okcolor)
	$lvdtbref.update("B Ref", $bref, okcolor)
	$lvdtcref.update("C Ref", $cref, okcolor)
end

def getjam
	okcolor = "#00ff00"
	ftsc = -1
	$fwguistatehash["loadedfilter"] = "none"
	r = $gc.send("SHOW FILTTSC")
	if /OK/.match(r) == nil
                puts "Error reading filter translation stop code"
		puts "r length #{r.length} r #{r}"
        else
                d = r.split(/:/, 2)
                ftsc = d[1].to_i
        end

	puts "jam stat #{ftsc}"
	if ftsc == 2
		r = $gc.send("SHOWLOADEDFILTER")
		if /OK:/.match(r) == nil
			puts "Error getting loaded filtername"
		else
			d = r.split(/:/, 2)
			$lfname = d[1]
		        $jamstat.update("CURRENT FILTER STATUS", "#{$lfname} IN OK", "yellow")
			$fwguistatehash["loadedfilter"] = $lfname
		end
	elsif ftsc == 3
		r = $gc.send("SHOW FILTVAL")
		if /OK:/.match(r) == nil
			puts "Error getting filtval"
		else
			d = r.split(/:/, 2)
			puts "d: #{d[1]}"
			fn = $filterinfo[d[1].chomp.to_i].filtshortname
			$jamstat.update("CURRENT FILTER STATUS", "#{fn} OUT OK", okcolor)
			$rfilt = fn
		end
	elsif ftsc == 1
		 r = $gc.send("SHOW FILTVAL")
                if /OK:/.match(r) == nil
                        puts "Error getting filtval"
                else
                        d = r.split(/:/, 2)
                        fn = $filterinfo[d[1].chomp.to_i].filtshortname
                        $jamstat.update("CURRENT FILTER STATUS", "#{fn} JAMMED", "red")
			$fwguistatehash["loadedfiler"] = "jammed"
                end
	elsif ftsc == -1
		$jamstat.update("CURRENT FILTER STATUS", "UNKNOWN", "grey")
		$fwguistatehash["loadedfilter"] = "unknown"
	end
	writefwguistatefile
end

def getsel
	r = $gc.send("SHOW REQFILT")
	if /OK:/.match(r) == nil
		puts "Error getting reqfilt"
	else
		d = r.split(/:/, 2)
		s = d[1].chomp.to_i
		if s == 256
			$selstat.update("SELECTED FILTER", "None", "orange")
		else
			puts "s is #{s}"
			sf = $filterinfo[s].filtshortname
			$selstat.update("SELECTED FILTER", "#{sf}", "green")
			$sfilt = sf
		end
	end
end

def getad
		okcolor = "#00ff00"
		v = $adbytes["a"]
		adb = $gstat[v, 2]
		adba = adb.unpack("s")
		adbv = adba[0] * $adconv
		$alvdt = adbv.to_s[0,5]
		if $moves["a"] == 1
			acolor = "orange"
		else
			acolor = okcolor
		end
		$lvdta.update("A", adbv.to_s[0,5], acolor)
		v = $adbytes["b"]
		adb = $gstat[v, 2]
		adba = adb.unpack("s")
		adbv = adba[0] * $adconv
		$blvdt = adbv.to_s[0,5]
		if $moves["b"] == 1
			bcolor = "orange"
		else
			bcolor = okcolor
		end
		$lvdtb.update("B", adbv.to_s[0,5], bcolor)
		v = $adbytes["c"]
		adb = $gstat[v, 2]
		adba = adb.unpack("s")
		adbv = adba[0] * $adconv
		$clvdt = adbv.to_s[0,5]
		if $moves["c"] == 1
			ccolor = "orange"
		else
			ccolor = okcolor
		end
		$lvdtc.update("C", adbv.to_s[0,5], ccolor)
end
def getsw
	swb = $gstat[7, 1]
	swba = swb.unpack("b8")
	puts "sw #{swba[0]}"
end

def setas
	r = $gc.send("SHOW DISTA")
	puts "r #{r}"
	if /OK/.match(r) == nil
		puts "Error reading actuator A step setting"
	else
		d = r.split(/:/, 2)
		puts "a #{d[1]}"
		$afocusav.value= d[1].to_i
	end
	r = $gc.send("SHOW DISTB")
	puts "r #{r}"
	if /OK/.match(r) == nil
		puts "Error reading actuator B step setting"
	else
		d = r.split(/:/, 2)
		puts "b #{d[1]}"
		$bfocusav.value= d[1].to_i
	end
	r = $gc.send("SHOW DISTC")
	puts "r #{r}"
	if /OK/.match(r) == nil
		puts "Error reading actuator C step setting"
	else
		d = r.split(/:/, 2)
		puts "c #{d[1]}"
		$cfocusav.value= d[1].to_i
	end
	$rfflag = false
end

def setnotify(c, m)
	$ncount = c
	$nmessage = m
end

def notify()
	if $ncount != 0
		$jamstat.update("ALERT", $nmessage, "red")
		$ncount = $ncount - 1
	end
end

def setwarn()
	$notifystat.update("Warnings", $warnstr, $warncolor)
end

def initfw
	r = $gc.send("INITFW")	
	if /OK:/.match(r) == nil
		setnotify(10, "Error initializing filterwheel")
	else
		setnotify(10, "Filterwheel initializing")
		$initialized = true
	end
end

def filtrd
	r = $gc.send("FILTRD")
	 if /OK:/.match(r) == nil
                setnotify(10, "Error Reading Filters")
        else
                setnotify(10, "Reading Filters")
        end
	setfilterbuttons
end

def populate
	r = $gc.send("POPULATE")
	if /OK:/.match(r) == nil
		setnotify(10, "Error setting populate mode")
	end
end

def popdone
	r = $gc.send("POPDONE")
	if /OK:/.match(r) == nil
		setnotify(10, "Error ending populate mode")
	end
end

def oktoload
	puts "checking if OK to enable 'load button"
	if($sfilt == $rfilt)
		$lbut.set_sensitive(true)
	else
		$lbut.set_sensitive(false)
	end
end

Gtk.main
