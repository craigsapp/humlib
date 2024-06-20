//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Sep 13 13:33:28 PDT 2015
// Last Modified: Sun Sep 13 13:33:31 PDT 2015
// Filename:      scripts/main.js
// Syntax:        JavaScript 1.8.5/ECMAScript 5.1
// vim:           ts=3 hlsearch
//
// Description:   Javascript common to all pages.
//

// event keyCodes.  See: http://www.javascripter.net/faq/keycodes.htm
var TabKey       =   9;
var BackspaceKey =   8;
var ShiftKey     =  16;
var CommaKey     = 188;
var PeriodKey    = 190;
var PeriodKeyNumberPad = 110; // Number Pad Minus Key;
var AKey         =  65;
var BKey         =  66;
var CKey         =  67;
var DKey         =  68;
var EKey         =  69;
var FKey         =  70;
var GKey         =  71;
var HKey         =  72;
var IKey         =  73;
var JKey         =  74;
var KKey         =  75;
var LKey         =  76;
var MKey         =  77;
var NKey         =  78;
var OKey         =  79;
var PKey         =  80;
var QKey         =  81;
var RKey         =  82;
var SKey         =  83;
var TKey         =  84;
var UKey         =  85;
var VKey         =  86;
var WKey         =  87;
var XKey         =  88;
var PlusKey      = 187;
var MinusKey     = 189;
var MinusKey2    = 173;      // Firefox MinusKey
var MinusKeyNumberPad = 109; // Number Pad Minus Key;
var ZeroKey      =  48;
var OneKey       =  49;
var TwoKey       =  50;
var ThreeKey     =  51;
var FourKey      =  52;
var FiveKey      =  53;
var SixKey       =  54;
var SevenKey     =  55;
var EightKey     =  56;
var NineKey      =  57;
var QuestionKey  = 191;
var EscKey       =  27;


document.addEventListener('DOMContentLoaded', function(event) {
	insertLinks();
});


//////////////////////////////
//
// insertLinks -- Make links to class documentation, if the class::function
//   name is in an element with the class "mhcf" (humlib class function).
//
// <span class="mhcf paren">HumdrumFile::printCSV</span> 
// transforms into:
// <span class="mhcf paren"><a href="/doc/class/HumdrumFile>HumdrumFile</a>
//	::<a href="/doc/class/HumdrumFile#printCSV">printCSV</a></span> 
//
// Options: 
//    noc = no class
//    paren = put parenthese after function
//    dot   = put dot before function
//

function insertLinks() {
	let matches;
	let output;

	let funcs = document.querySelectorAll('.mhcf');
	for (let i=0; i<funcs.length; i++) {
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
	let list = document.querySelectorAll("span[class^='" + tag + "']");
	for (let i=0; i<list.length; i++) {
		let regex = new RegExp(tag + '-(.*)');
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
	let anchor = sessionStorage.hash;
	if (anchor) {
		sessionStorage[openlist] = '["' + anchor + '"]';
	}
	if (!sessionStorage[openlist]) {
		return;
	}
	list = JSON.parse(sessionStorage[openlist]);
	let item = document.querySelector('details.' + tag + '-' + anchor);
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
	let list = document.querySelectorAll('details');
	for (let i=0; i<list.length; i++) {
		let detail = list[i];
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
	let list = document.querySelectorAll('details');
	for (let i=0; i<list.length; i++) {
		let detail = list[i];
		if (!detail) {
			continue;
		}
		detail.open = 'open';
	}
}



//////////////////////////////
//
// documentationKeyCommands -- keyboard commands for documentation pages.
//

function documentationKeyCommands(event, tag) {
	switch (event.keyCode) {
		case OneKey:
			let list = document.querySelector('details[class^="' + tag + '-"]');
			if (list.open) {
				closeAllDetails();
			} else {
				openAllDetails();
			}
			break;

		case PlusKey:
			openAllDetails();
			break;

		case MinusKey:
		case MinusKey2:
		case MinusKeyNumberPad:
			closeAllDetails();
			break;
	}
}



//////////////////////////////
//
// checkAnchor -- If a specific details element is requested to be
//    opened when the page is loaded, then open it and remove the anchor
//    from the URL.
//

function checkAnchor(tag) {
	let anchor = '';
	matches = window.location.hash.match(/^#(.*)/);
	if (matches) {
		sessionStorage.hash = matches[1];
		anchor = matches[1];
	}
	anchor = anchor.toLowerCase();
	removeHash();
	let selector = `details.${tag}-${anchor}`;
	let element = document.querySelector(selector);
	if (!element) {
		return;
	}
	element.open = 'open';
	element.scrollIntoViewIfNeeded();
}



