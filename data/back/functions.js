	var IntervalCaller; //Used to cancel the intermittant updates when editting a value
	
	function togglepid(e) {
		$.ajax({url: "/api/togglepid", dataType: "json", success: function(result){ updatepage(result) }});
	}
	
	function changetscale(e,ts) {
		var nts=ts+1;
		if (nts==4) nts=1;
		var jstr="{ \"tscale\":" + nts + " }";
		
	    $.ajax({
            url: '/api/setparms',
            type: 'post',
            dataType: 'json',
            contentType: 'application/json',
            success: function (data) {
                if (!data.Result==0) { alert("Bad Value"); } else {
					updatepage(data);
				}
            },
            data: jstr
        });
		
	}
	
	function processpanelinput(e) {
	    var pval=$(e).val();
		var pname = $(e).parent().find('.serverparmname').html();
		var jstr="{ \"" + pname + "\": " + pval + " }";
		$(e).parent().unbind("click");
		$(e).hide();
		$(e).parent().find('.vis').html( pval );
		$(e).parent().find('.vis').show();
		$(e).parent().find('.uns').show();

        $.ajax({
            url: '/api/setparms',
            type: 'post',
            dataType: 'json',
            contentType: 'application/json',
            success: function (data) {
                if (!data.Result==0) { alert("Bad Value"); } else {
					updatepage(data);
				}
            },
            data: jstr
        });
		
		IntervalCaller = window.setInterval(function(){ $.ajax({url: "/api/getparms", dataType: "json", success: function(result){ updatepage(result) }}); }, 1000);
	}
	
	function addpanelinput(e) {
		window.clearInterval(IntervalCaller); // Whilst we are showing an input we want to stop automatic updates of page
		$(e).unbind("click");          // Also Stop further clicks of this div
		$(e).find('.vis').hide();
		$(e).find('.uns').hide();
		$(e).find('.inp').show();
		$(e).find('.inp').off();       // Remove any old event handlers
		$(e).find('.inp').on( "focusout" , function () {processpanelinput(this);} );  // Add new event handler that fires when input loses focus
		$(e).find('.inp').focus();
	}
	
	function divhtml(preamble,visible,units,spn,inputsize) {
	var rv="";
	rv += "<span class=\"pre\">" + preamble + "</span>";
	rv += "<span class=\"vis\">" + visible + "</span>";
	rv += "<span class=\"uns\">" + units + "</span>";
	rv += "<span class=\"serverparmname\" style='display:none;' >" + spn + "</span>";
	rv += "<input class=\"inp\" style='display:none;' type=\"text\" size=\"" + inputsize +"\" maxlength=\"" + inputsize +"\" value=\"" + visible + "\">";
	return rv;
	}

	// function divtscale(preamble,visible,units,inputsize) {
	// var rv="";
	// rv += "<span class=\"pre\">" + preamble + "</span>";
	// rv += "<span class=\"vis\">" + visible + "</span>";
	// rv += "<span class=\"uns\"></span>";
	// rv += "<span class=\"serverparmname\" style='display:none;' >tscale</span>";
	// rv += "<select style='display:none;' name="tscale"><option value=1>Kelvin</option><option value=2>Centigrade</option><option value=3>Farenheit</option></select>"
	// return rv;
	// }	
	
	function updatepage(jsondata) {
		$('#frame1', window.parent.document).height(100);  // Needed. Please keep.
		
		if (jsondata.hasOwnProperty('uptime')) {
			var jsecs=jsondata.uptime / 1000.0;
			var dhrs=parseInt(jsecs / 3600);
			jsecs -= (3600 * dhrs);
			var dmins=parseInt(jsecs / 60 );
			var dsecs=parseInt(jsecs % 60);
			var upout="Uptime = " + (dhrs).toString().padStart(2, '0') + ":" + (dmins).toString().padStart(2, '0') + ":" + (dsecs).toString().padStart(2, '0');
			$("#uptime").html(upout);
		}

		if (jsondata.hasOwnProperty('ipaddr')) {
			$("#ipaddr").html("IP Address = " + jsondata.ipaddr);
		}
		
		if (jsondata.hasOwnProperty('ssid')) {
			$("#ssid").html("SSID = " + jsondata.ssid);
		}

		if (jsondata.hasOwnProperty('dfilesize')) {
			$("#dfilesize").html("Data Size = " + ((jsondata.dfilesize)/1024).toFixed(2) + "K");
		}
		
		if (jsondata.hasOwnProperty('temp1')) {
			var tsu = "";
			var pam = "";
			if (jsondata.hasOwnProperty('tscale')) {
				if (jsondata.tscale==1) { tsu="°K"; pam="T1=";   }
				if (jsondata.tscale==2) { tsu="°C"; pam="T1 = "; }
				if (jsondata.tscale==3) { tsu="°F"; pam="T1 = "; }
			} 
			$("#temp1").html(pam + jsondata.temp1.toFixed(2) + tsu);
		}
		
		if (jsondata.hasOwnProperty('temp2')) {
			var tsu = "";
			var pam = "";
			if (jsondata.hasOwnProperty('tscale')) {
				if (jsondata.tscale==1) { tsu="°K"; pam="T2=";   }
				if (jsondata.tscale==2) { tsu="°C"; pam="T2 = "; }
				if (jsondata.tscale==3) { tsu="°F"; pam="T2 = "; }
			} 
			$("#temp2").html(pam + jsondata.temp2.toFixed(2) + tsu);
		}
		
		if (jsondata.hasOwnProperty('UTCoffset')) {
			$("#utcoffset").html(divhtml("UTC Offset = ",jsondata.UTCoffset," secs","UTCoffset",5));
			$("#utcoffset").addClass("clickable");
			$("#utcoffset").unbind("click"); $("#utcoffset").click( function () {addpanelinput(this);} );	
		}
		
		if (jsondata.hasOwnProperty('currenttime')) {
			$("#curtime").html("Time = " + jsondata.currenttime);
		}
		
		if (jsondata.hasOwnProperty('pidname') || jsondata.hasOwnProperty('pidrunning')) {
			$("#pidrunning").html(jsondata.pidname + " : " +
								 ( jsondata.pidrunning ? "<span style='color:DarkGreen;'>Running</span>" : "<span style='color:DarkRed;'>Not Running</span>" ));
			$("#pidrunning").addClass("clickable");
			$("#pidrunning").unbind("click"); $("#pidrunning").click( function () {togglepid(this);} );		
		}
		
		if (jsondata.hasOwnProperty('targettemp')) {
			$("#targettemp").html(divhtml("Target Temp = ",jsondata.targettemp," degF","targettemp",5));
			$("#targettemp").addClass("clickable");
			$("#targettemp").unbind("click"); $("#targettemp").click( function () {addpanelinput(this);} );	
		}
		
		if (jsondata.hasOwnProperty('fanpercent')) {		
			$("#fanpercent").html("Fan Percentage = " + jsondata.fanpercent.toFixed(2) + "%");
		}
		
		if (jsondata.hasOwnProperty('pidinterval')) {	
			$("#pidinterval").html(divhtml("PID Interval = ",jsondata.pidinterval ," ms","pidinterval",5));
			$("#pidinterval").addClass("clickable");
			$("#pidinterval").unbind("click"); $("#pidinterval").click( function () {addpanelinput(this);} );	
		}
		
		if (jsondata.hasOwnProperty('lastpidtemp')) {	
			$("#lastpidtemp").html("Last PID temp = " + jsondata.lastpidtemp.toFixed(2));
		}
		
		if (jsondata.hasOwnProperty('kP')) {	
			$("#kP").html(divhtml("kP = ",jsondata.kP,"","kP",5));
			$("#kP").addClass("clickable");
			$("#kP").unbind("click"); $("#kP").click( function () {addpanelinput(this);} );	
		}
		
		if (jsondata.hasOwnProperty('kI')) {	
			$("#kI").html(divhtml("kI = ",jsondata.kI,"","kI",5));
			$("#kI").addClass("clickable");
			$("#kI").unbind("click"); $("#kI").click( function () {addpanelinput(this);} );	
		}
		
		if (jsondata.hasOwnProperty('kD')) {
			$("#kD").html(divhtml("kD = ",jsondata.kD,"","kD",5));
			$("#kD").addClass("clickable");
			$("#kD").unbind("click"); $("#kD").click( function () {addpanelinput(this);} );	
		}
		if (jsondata.hasOwnProperty('tune')) {
			$("#tune").html(divhtml("Tune = ",jsondata.tune," degF","tune",5));
			$("#tune").addClass("clickable");
			$("#tune").unbind("click"); $("#tune").click( function () {addpanelinput(this);} );	
		}
		if (jsondata.hasOwnProperty('shift')) {
			$("#shift").html(divhtml("Shift = ",jsondata.shift," degF","shift",5));
			$("#shift").addClass("clickable");
			$("#shift").unbind("click"); $("#shift").click( function () {addpanelinput(this);} );	
		}
		
		if (jsondata.hasOwnProperty('tscale')) {
			var tscaletext="";
			if (jsondata.tscale==1) tscaletext="Kelvin";
			if (jsondata.tscale==2) tscaletext="Centigrade";
			if (jsondata.tscale==3) tscaletext="Farenheit";
			$("#tscale").html("Temp Scale = " + tscaletext);
			$("#tscale").addClass("clickable");
			$("#tscale").unbind("click"); $("#tscale").click( function () {changetscale(this,jsondata.tscale);} );	
		}
		
		if (jsondata.hasOwnProperty('p1sharta')) {
			$("#p1sharta").html(divhtml("S.Hart 1A = ",jsondata.p1sharta.toExponential(),"","p1sharta",14));
			$("#p1sharta").addClass("clickable");
			$("#p1sharta").unbind("click"); $("#p1sharta").click( function () {addpanelinput(this);} );	
		}
		if (jsondata.hasOwnProperty('p1shartb')) {
			$("#p1shartb").html(divhtml("S.Hart 1B = ",jsondata.p1shartb.toExponential(),"","p1shartb",14));
			$("#p1shartb").addClass("clickable");
			$("#p1shartb").unbind("click"); $("#p1shartb").click( function () {addpanelinput(this);} );	
		}
		if (jsondata.hasOwnProperty('p1shartc')) {
			$("#p1shartc").html(divhtml("S.Hart 1C = ",jsondata.p1shartc.toExponential(),"","p1shartc",14));
			$("#p1shartc").addClass("clickable");
			$("#p1shartc").unbind("click"); $("#p1shartc").click( function () {addpanelinput(this);} );	
		}
		if (jsondata.hasOwnProperty('p2sharta')) {
			$("#p2sharta").html(divhtml("S.Hart 2A = ",jsondata.p2sharta.toExponential(),"","p2sharta",14));
			$("#p2sharta").addClass("clickable");
			$("#p2sharta").unbind("click"); $("#p2sharta").click( function () {addpanelinput(this);} );	
		}
		if (jsondata.hasOwnProperty('p2shartb')) {
			$("#p2shartb").html(divhtml("S.Hart 2B = ",jsondata.p2shartb.toExponential(),"","p2shartb",14));
			$("#p2shartb").addClass("clickable");
			$("#p2shartb").unbind("click"); $("#p2shartb").click( function () {addpanelinput(this);} );	
		}
		if (jsondata.hasOwnProperty('p2shartc')) {
			$("#p2shartc").html(divhtml("S.Hart 2C = ",jsondata.p2shartc.toExponential(),"","p2shartc",14));
			$("#p2shartc").addClass("clickable");
			$("#p2shartc").unbind("click"); $("#p2shartc").click( function () {addpanelinput(this);} );	
		}

		
		$('#frame1', window.parent.document).width("330px");
		var sheight = document.getElementsByTagName("html")[0].scrollHeight.toString()+"px";
		$('#frame1', window.parent.document).height(sheight);
		
	}
	
	$(document).ready(function(){
		$.ajax({url: "/api/getparms", dataType: "json", success: function(result){ updatepage(result) }});
	});
	
	IntervalCaller=window.setInterval(function(){ $.ajax({url: "/api/getparms", dataType: "json", success: function(result){ updatepage(result) }}); }, 1000);