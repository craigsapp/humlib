// vim: ts=3


document.addEventListener('DOMContentLoaded', function(event) {
	insertLinks();
});


//////////////////////////////
//
// insertLinks -- Make links to class documentation, if the class::function
//   name is in an element with the class "mhcf" (minHumdrum class function).
//
// <span class="mhcf paren">HumdrumFile::printCSV</span> 
// transforms into:
// <span class="mhcf paren"><a href="/doc/class/HumdrumFile>HumdrumFile</a>
//	::<a href="/doc/class/HumdrumFile#printCSV">printCSV</a></span> 
//

function insertLinks() {
	var funcs = document.querySelectorAll('.mhcf');
	for (var i=0; i<funcs.length; i++) {
		var matches = funcs[i].innerHTML.match(/(.*)::(.*)/);
		if (!matches) {
			continue;
		}
		var output = '';
		output += '<a href="/doc/class/' + matches[1] + '">' + matches[1];
		output += '</a>::<a href="/doc/class/' + matches[1];
		output += '#' + matches[2]  + '">' + matches[2] + '</a>';
		funcs[i].innerHTML = output;
	}
}



