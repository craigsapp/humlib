//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu Jan 22 19:09:03 PST 2015
// Last Modified: Sun Feb 22 17:33:25 PST 2015 Avoid null of a@name hyperlinks.
// Filename:      javascripts/external-links.js
// Syntax:        JavaScript 1.8.5/ECMAScript 5.1
// vim:           ts=3 hlsearch
//
// Description:   Examine all links on a page, an if they start 
//                with ^https?:// then make then open in a new 
//                browser tab.
//

window.addEventListener('load', function () {
	externalLinks('new');
});



//////////////////////////////
//
// externalLinks -- Force external links to open in a new tab.
//

function externalLinks(tabname) {
	if (!tabname) {
		tabname = 'new';
	}
	var links = document.querySelectorAll('a');
	var i;
	for (var i=0; i<links.length; i++) {
		if (links[i].getAttribute('href')) {
			if (links[i].getAttribute('href').match(/^https?:\/\//)) {
				links[i].target = tabname;
			}
		}
	}
}



