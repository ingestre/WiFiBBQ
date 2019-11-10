	var IntervalCaller; //Used to cancel the intermittant updates when editting a value
	
	function calcoldnewmd5 () {
	  var oldpwod = document.getElementById("oldpass").value;
	  var newpwod = document.getElementById("newpass").value;
	  var retpwod = document.getElementById("retype").value;
	  
	  if (newpwod != retpwod) {
		  alert("Passwords do not match");
	  } else {
		  document.getElementById("oldpass").value="";
          $("cpbutt").attr("disabled", true);
		  oldpwod = md5(oldpwod);
		  newpwod = md5(newpwod);
		  var jstr="{ \"oldpass\": \"" + oldpwod + "\" , \"newpass\": \"" + newpwod + "\" }";
		  $("#presult").fadeIn();
		  
	  	  $.ajax({
			url: '/api/setpass',
			type: 'post',
			dataType: 'json',
			contentType: 'application/json',
			success: function (data) { if (!data.Result==1) { $("#prestxt").html("failed"); } else { $("#prestxt").html("succeded"); } },
			data: jstr
		  });
		
	  }
	}

	function breakframes () {
	  if ( top !== self ) top.location.replace( self.location.href );
	}

	function md5 ( str ) {

		var RotateLeft = function(lValue, iShiftBits) {
				return (lValue<<iShiftBits) | (lValue>>>(32-iShiftBits));
			};

		var AddUnsigned = function(lX,lY) {
				var lX4,lY4,lX8,lY8,lResult;
				lX8 = (lX & 0x80000000);
				lY8 = (lY & 0x80000000);
				lX4 = (lX & 0x40000000);
				lY4 = (lY & 0x40000000);
				lResult = (lX & 0x3FFFFFFF)+(lY & 0x3FFFFFFF);
				if (lX4 & lY4) {
					return (lResult ^ 0x80000000 ^ lX8 ^ lY8);
				}
				if (lX4 | lY4) {
					if (lResult & 0x40000000) {
						return (lResult ^ 0xC0000000 ^ lX8 ^ lY8);
					} else {
						return (lResult ^ 0x40000000 ^ lX8 ^ lY8);
					}
				} else {
					return (lResult ^ lX8 ^ lY8);
				}
			};

		var F = function(x,y,z) { return (x & y) | ((~x) & z); };
		var G = function(x,y,z) { return (x & z) | (y & (~z)); };
		var H = function(x,y,z) { return (x ^ y ^ z); };
		var I = function(x,y,z) { return (y ^ (x | (~z))); };

		var FF = function(a,b,c,d,x,s,ac) {
				a = AddUnsigned(a, AddUnsigned(AddUnsigned(F(b, c, d), x), ac));
				return AddUnsigned(RotateLeft(a, s), b);
			};

		var GG = function(a,b,c,d,x,s,ac) {
				a = AddUnsigned(a, AddUnsigned(AddUnsigned(G(b, c, d), x), ac));
				return AddUnsigned(RotateLeft(a, s), b);
			};

		var HH = function(a,b,c,d,x,s,ac) {
				a = AddUnsigned(a, AddUnsigned(AddUnsigned(H(b, c, d), x), ac));
				return AddUnsigned(RotateLeft(a, s), b);
			};

		var II = function(a,b,c,d,x,s,ac) {
				a = AddUnsigned(a, AddUnsigned(AddUnsigned(I(b, c, d), x), ac));
				return AddUnsigned(RotateLeft(a, s), b);
			};

		var ConvertToWordArray = function(str) {
				var lWordCount;
				var lMessageLength = str.length;
				var lNumberOfWords_temp1=lMessageLength + 8;
				var lNumberOfWords_temp2=(lNumberOfWords_temp1-(lNumberOfWords_temp1 % 64))/64;
				var lNumberOfWords = (lNumberOfWords_temp2+1)*16;
				var lWordArray=Array(lNumberOfWords-1);
				var lBytePosition = 0;
				var lByteCount = 0;
				while ( lByteCount < lMessageLength ) {
					lWordCount = (lByteCount-(lByteCount % 4))/4;
					lBytePosition = (lByteCount % 4)*8;
					lWordArray[lWordCount] = (lWordArray[lWordCount] | (str.charCodeAt(lByteCount)<<lBytePosition));
					lByteCount++;
				}
				lWordCount = (lByteCount-(lByteCount % 4))/4;
				lBytePosition = (lByteCount % 4)*8;
				lWordArray[lWordCount] = lWordArray[lWordCount] | (0x80<<lBytePosition);
				lWordArray[lNumberOfWords-2] = lMessageLength<<3;
				lWordArray[lNumberOfWords-1] = lMessageLength>>>29;
				return lWordArray;
			};

		var WordToHex = function(lValue) {
				var WordToHexValue="",WordToHexValue_temp="",lByte,lCount;
				for (lCount = 0;lCount<=3;lCount++) {
					lByte = (lValue>>>(lCount*8)) & 255;
					WordToHexValue_temp = "0" + lByte.toString(16);
					WordToHexValue = WordToHexValue + WordToHexValue_temp.substr(WordToHexValue_temp.length-2,2);
				}
				return WordToHexValue;
			};

		var x=Array();
		var k,AA,BB,CC,DD,a,b,c,d;
		var S11=7, S12=12, S13=17, S14=22;
		var S21=5, S22=9 , S23=14, S24=20;
		var S31=4, S32=11, S33=16, S34=23;
		var S41=6, S42=10, S43=15, S44=21;

		x = ConvertToWordArray(str);
		a = 0x67452301; b = 0xEFCDAB89; c = 0x98BADCFE; d = 0x10325476;

		for (k=0;k<x.length;k+=16) {
			AA=a; BB=b; CC=c; DD=d;
			a=FF(a,b,c,d,x[k+0], S11,0xD76AA478);
			d=FF(d,a,b,c,x[k+1], S12,0xE8C7B756);
			c=FF(c,d,a,b,x[k+2], S13,0x242070DB);
			b=FF(b,c,d,a,x[k+3], S14,0xC1BDCEEE);
			a=FF(a,b,c,d,x[k+4], S11,0xF57C0FAF);
			d=FF(d,a,b,c,x[k+5], S12,0x4787C62A);
			c=FF(c,d,a,b,x[k+6], S13,0xA8304613);
			b=FF(b,c,d,a,x[k+7], S14,0xFD469501);
			a=FF(a,b,c,d,x[k+8], S11,0x698098D8);
			d=FF(d,a,b,c,x[k+9], S12,0x8B44F7AF);
			c=FF(c,d,a,b,x[k+10],S13,0xFFFF5BB1);
			b=FF(b,c,d,a,x[k+11],S14,0x895CD7BE);
			a=FF(a,b,c,d,x[k+12],S11,0x6B901122);
			d=FF(d,a,b,c,x[k+13],S12,0xFD987193);
			c=FF(c,d,a,b,x[k+14],S13,0xA679438E);
			b=FF(b,c,d,a,x[k+15],S14,0x49B40821);
			a=GG(a,b,c,d,x[k+1], S21,0xF61E2562);
			d=GG(d,a,b,c,x[k+6], S22,0xC040B340);
			c=GG(c,d,a,b,x[k+11],S23,0x265E5A51);
			b=GG(b,c,d,a,x[k+0], S24,0xE9B6C7AA);
			a=GG(a,b,c,d,x[k+5], S21,0xD62F105D);
			d=GG(d,a,b,c,x[k+10],S22,0x2441453);
			c=GG(c,d,a,b,x[k+15],S23,0xD8A1E681);
			b=GG(b,c,d,a,x[k+4], S24,0xE7D3FBC8);
			a=GG(a,b,c,d,x[k+9], S21,0x21E1CDE6);
			d=GG(d,a,b,c,x[k+14],S22,0xC33707D6);
			c=GG(c,d,a,b,x[k+3], S23,0xF4D50D87);
			b=GG(b,c,d,a,x[k+8], S24,0x455A14ED);
			a=GG(a,b,c,d,x[k+13],S21,0xA9E3E905);
			d=GG(d,a,b,c,x[k+2], S22,0xFCEFA3F8);
			c=GG(c,d,a,b,x[k+7], S23,0x676F02D9);
			b=GG(b,c,d,a,x[k+12],S24,0x8D2A4C8A);
			a=HH(a,b,c,d,x[k+5], S31,0xFFFA3942);
			d=HH(d,a,b,c,x[k+8], S32,0x8771F681);
			c=HH(c,d,a,b,x[k+11],S33,0x6D9D6122);
			b=HH(b,c,d,a,x[k+14],S34,0xFDE5380C);
			a=HH(a,b,c,d,x[k+1], S31,0xA4BEEA44);
			d=HH(d,a,b,c,x[k+4], S32,0x4BDECFA9);
			c=HH(c,d,a,b,x[k+7], S33,0xF6BB4B60);
			b=HH(b,c,d,a,x[k+10],S34,0xBEBFBC70);
			a=HH(a,b,c,d,x[k+13],S31,0x289B7EC6);
			d=HH(d,a,b,c,x[k+0], S32,0xEAA127FA);
			c=HH(c,d,a,b,x[k+3], S33,0xD4EF3085);
			b=HH(b,c,d,a,x[k+6], S34,0x4881D05);
			a=HH(a,b,c,d,x[k+9], S31,0xD9D4D039);
			d=HH(d,a,b,c,x[k+12],S32,0xE6DB99E5);
			c=HH(c,d,a,b,x[k+15],S33,0x1FA27CF8);
			b=HH(b,c,d,a,x[k+2], S34,0xC4AC5665);
			a=II(a,b,c,d,x[k+0], S41,0xF4292244);
			d=II(d,a,b,c,x[k+7], S42,0x432AFF97);
			c=II(c,d,a,b,x[k+14],S43,0xAB9423A7);
			b=II(b,c,d,a,x[k+5], S44,0xFC93A039);
			a=II(a,b,c,d,x[k+12],S41,0x655B59C3);
			d=II(d,a,b,c,x[k+3], S42,0x8F0CCC92);
			c=II(c,d,a,b,x[k+10],S43,0xFFEFF47D);
			b=II(b,c,d,a,x[k+1], S44,0x85845DD1);
			a=II(a,b,c,d,x[k+8], S41,0x6FA87E4F);
			d=II(d,a,b,c,x[k+15],S42,0xFE2CE6E0);
			c=II(c,d,a,b,x[k+6], S43,0xA3014314);
			b=II(b,c,d,a,x[k+13],S44,0x4E0811A1);
			a=II(a,b,c,d,x[k+4], S41,0xF7537E82);
			d=II(d,a,b,c,x[k+11],S42,0xBD3AF235);
			c=II(c,d,a,b,x[k+2], S43,0x2AD7D2BB);
			b=II(b,c,d,a,x[k+9], S44,0xEB86D391);
			a=AddUnsigned(a,AA);
			b=AddUnsigned(b,BB);
			c=AddUnsigned(c,CC);
			d=AddUnsigned(d,DD);
		}

		var temp = WordToHex(a)+WordToHex(b)+WordToHex(c)+WordToHex(d);

		return temp.toLowerCase();
	}


	function showdiv(evt, link) {
		var tablinks,divshs;
        if (link=='D4') {
			document.getElementById("oldpass").value="";
			document.getElementById("newpass").value="";
			document.getElementById("retype").value="";
			$("cpbutt").attr("disabled", false);
			$("#presult").hide(); 
			$("#prestxt").html("in progress");			
		}
		divshs = document.getElementsByClassName("divsh");
		for (var i = 0; i < divshs.length; i++) {
			divshs[i].style.display = 'none';
		}
		document.getElementById(link).style.display = 'block';
		
		tablinks = document.getElementsByClassName("tablinks");
		for (var i = 0; i < tablinks.length; i++) {
			tablinks[i].className = tablinks[i].className.replace(" active", "");
		}
		evt.currentTarget.className += " active";
	}


	
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
		
		if (jsondata.hasOwnProperty('Result')) {
			var jdresult=jsondata.Result;
			if (jdresult == -100) {  top.location.reload( true ); }
		}
		
		if (jsondata.hasOwnProperty('uptime')) {
			var jsecs=jsondata.uptime / 1000.0;
			var hrs=parseInt(jsecs / 3600);
			var ddys=parseInt(hrs / 24);
			var dhrs=parseInt(hrs % 24);
			jsecs -= (3600 * hrs);
			var dmins=parseInt(jsecs / 60 );
			var dsecs=parseInt(jsecs % 60);
			var upout="Uptime = " + (ddys).toString() + "d " + (dhrs).toString().padStart(2, '0')
            upout += ":" + (dmins).toString().padStart(2, '0') + ":" + (dsecs).toString().padStart(2, '0');
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