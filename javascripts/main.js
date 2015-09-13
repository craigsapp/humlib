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
	var matches;
	var output;
	var i;
	matches = window.location.hash.match(/^#(.*)/);
	if (matches) {
		sessionStorage.hash = matches[1];
	}
	removeHash();

	var funcs = document.querySelectorAll('.mhcf');
	for (i=0; i<funcs.length; i++) {
		matches = funcs[i].innerHTML.match(/(.*)::(.*)/);
		if (!matches) {
			continue;
		}
		output = '';

		if (!funcs[i].className.match(/\bnoc\b/)) {
			output += '<a href="/doc/class/' + matches[1] + '">';
			output +=  matches[1];
			output += '</a>::';
		} else if (funcs[i].className.match(/\bdot\b/)) {
			output += '.';
		}
		output += '<a href="/doc/class/' + matches[1];
		output += '#' + matches[2]  + '">' + matches[2] + '</a>';
		funcs[i].innerHTML = output;
	}

	makeDocLinks("ref", "#");
	makeDocLinks("snippet", "#");
	makeDocLinks("example", "#");
	makeDocLinks("topic", "#");
	makeDocLinks("class", "");

}



//////////////////////////////
//
// makeDocLinks -- Create hyperlinks to various components of
//    the documentation.
//

function makeDocLinks(tag, prefix) {
	var list = document.querySelectorAll("span[class^='" + tag + "']");
	for (i=0; i<list.length; i++) {
		var regex = new RegExp(tag + '-(.*)');
		matches = list[i].className.match(regex);
		if (!matches) {
			continue;
		}
		output = '';
		output += '<a href="/doc/' + tag + '/' + prefix + matches[1] + '">';
		output +=  list[i].innerHTML;
		output += '</a>';
		list[i].outerHTML = output;
	}
}



//////////////////////////////
//
// removeHash -- Get rid of hash in URL.
//

function removeHash () { 
	window.location.hash = '';

/*	history.pushState(
		"", 
		document.title, 
		window.location.pathname + window.location.search
	);
*/
}



//////////////////////////////
//
// checkDetailsState -- If there is an anchor, then open only that
//     details section (for documentation pages).
//

function checkDetailsState(openlist, tag) {
	var anchor = sessionStorage.hash;
	if (anchor) {
		sessionStorage[openlist] = '["' + anchor + '"]';
	}
	if (!sessionStorage[openlist]) {
		return;
	}
	list = JSON.parse(sessionStorage[openlist]);
	var item = document.querySelector('details.' + tag + '-' + anchor);
	if (!item) {
		return;
	}
	item.open = 'open';
	item.scrollIntoViewIfNeeded();
}



//////////////////////////////
//
// closeAllDetails -- Close all details on a page.
//

function closeAllDetails() {
	var list = document.querySelectorAll('details');
	for (var i=0; i<list.length; i++) {
		var detail = list[i];
		if (!detail) {
			continue;
		}
		detail.removeAttribute('open');
	}
}



//////////////////////////////
//
// openAllDetails -- Open all details on a page.
//

function openAllDetails() {
	var list = document.querySelectorAll('details');
	for (var i=0; i<list.length; i++) {
		var detail = list[i];
		if (!detail) {
			continue;
		}
		detail.open = 'open';
	}
}



