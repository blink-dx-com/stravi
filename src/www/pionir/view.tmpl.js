var view_lastTag  ="";
var view_lastColor="";

function th_snc( tag, id ) {

	var newColor     = "#D0E0FF";
	
	
	if (view_lastTag!="") {
		document.getElementById(view_lastTag).style.backgroundColor = view_lastColor;
	}
	if( (document.getElementById) && (document.getElementById( tag )!=null)) {
  		var myElement = document.getElementById( tag );        
		if ((myElement.style) && (myElement.style.backgroundColor!=null)) {   
			colorNow = myElement.style.backgroundColor;
			myElement.style.backgroundColor = newColor;
		}

		view_lastTag = tag;
		view_lastColor = colorNow;
	}  
	  
}
