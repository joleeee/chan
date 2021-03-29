// For todays date;
Date.prototype.today = function () { 
    return this.getFullYear() + "-" +(((this.getMonth()+1) < 10)?"0":"") + (this.getMonth()+1) + "-" + ((this.getDate() < 10)?"0":"") + this.getDate();
}

// For the time now
Date.prototype.timeNow = function () {
     return ((this.getHours() < 10)?"0":"") + this.getHours() +":"+ ((this.getMinutes() < 10)?"0":"") + this.getMinutes() +":"+ ((this.getSeconds() < 10)?"0":"") + this.getSeconds();
}


function localize(){
	console.log("localize");

	var all = document.getElementsByClassName("posttime")

	for (var i=0, max=all.length; i < max; i++) {
	  var o = all[i].innerHTML.replace(/-/g, "/")
	  var t = new Date(o);
	  //var zone = Intl.DateTimeFormat().resolvedOptions().timeZone;
	  var d = new Date()
	  var zone = d.toLocaleString('en', {timeZoneName: 'short'}).split(' ').pop();
	  all[i].innerHTML=t.today() + " " + t.timeNow() + " <small>(" + zone + ")</small>";
	}
}
