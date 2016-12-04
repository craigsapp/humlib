//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Fri Dec  2 09:30:17 PST 2016
// Last Modified: Fri Dec  2 09:30:21 PST 2016
// Filename:      HumdrumFileBase-net.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileBase-net.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Functionality related to downloading Humdrum data
//                over the internet.
//

#include "HumdrumFileBase.h"
#include "Convert.h"

#include <sstream>
#include <fstream>
#include <stdarg.h>
#include <string.h>

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumFileBase::getUriToUrlMapping --
//

string HumdrumFileBase::getUriToUrlMapping(const string& uri) {
	auto css = uri.find("://");
	if (css == string::npos) {
		// this is not a URI, so just return input:
		return string(uri);
	}

	if (Convert::startsWith(uri, "http://")) {
		// The URI is a URL, so just copy:
		return string(uri);
	}

	string tag  = uri.substr(0, css);
	string rest = uri.substr(css+3);
	if (rest.empty()) {
		rest = "/";
	}

	// getting a repertory:
	// http://kern.humdrum.org/data?l=osu/classical/bach/inventions
	// getting a single file:
	// http://kern.humdrum.org/data?s=http://kern.humdrum.org/data?s=osu/classical/bach/inventions&file=inven15.krn
	// (Should allow repertory from &s...)
	if ((tag == "humdrum") || (tag == "hum") || (tag == "h")) {
		string testlocation;
		string testfilename;
		int repertoryQ = false;
		auto slash = rest.rfind('/');
		if (slash != string::npos) {
			testlocation = rest.substr(0, slash);
			testfilename = rest.substr(slash+1);
			if (testfilename.find('.') == string::npos) {
				repertoryQ = true;
			}
		} if (slash == string::npos) {
			// no files in root directory, but no reperoties either
			repertoryQ = true;
		}
		string output = "http://";;
		output += "kern.ccarh.org";
		output += "/data?";
		if (repertoryQ) {
			output += "l=";
		} else {
			output += "s=";
		}
		output += rest;
		// probably not needed:
		//output += "&format=kern";
		return output;
	}

	if (tag == "jrp") {
		string output = "http://";
		output += "jrp.ccarh.org";
		output += "/cgi-bin/jrp?a=humdrum";
		output += "&f=";
		output += rest;
		return output;
	}

	// not familiar with the URI, just assume that it is a URL,
	// such as "https://".
	return uri;
}


#ifdef USING_URI

//////////////////////////////
//
// HumdrumFileBase::readFromHumdrumUri -- Read a Humdrum file from an
//      humdrum:// web address
//
// Example:
//    maps: humdrum://osu/classical/haydn/london/sym099a.krn
// into:
//    http://kern.ccarh.org/cgi-bin/ksdata?file=sym099a.krn&l=/osu/classical/haydn/london&format=kern
//

void HumdrumFileBase::readFromHumdrumUri(const string& humaddress) {
	string url = HumdrumFileBase::getUriToUrlMapping(humaddress);
	readFromHttpUri(url);
}



//////////////////////////////
//
// readFromJrpUri -- Read a Humdrum file from a jrp:// web-style address
//
// Example:
// maps:
//    jrp://Jos2721-La_Bernardina
// into:
//    http://jrp.ccarh.org/cgi-bin/jrp?a=humdrum&f=Jos2721-La_Bernardina
//

void HumdrumFileBase::readFromJrpUri(const string& jrpaddress) {
	string url = HumdrumFileBase::getUriToUrlMapping(jrpaddress);
	readFromHttpUri(url);
}


//////////////////////////////
//
// HumdrumFileBase::readFromHttpUri -- download content from the web.
//

void HumdrumFileBase::readFromHttpUri(const string& webaddress) {
	stringstream inputdata;
	readStringFromHttpUri(inputdata, webaddress);
	HumdrumFileBase::readString(inputdata.str());
}



//////////////////////////////
//
// readFromHttpUri -- Read a Humdrum file from an http:// web address
//

void HumdrumFileBase::readStringFromHttpUri(stringstream& inputdata,
		const string& webaddress) {
	auto css = webaddress.find("://");
	if (css == string::npos) {
		// give up since URI was not in correct format
	}
	string rest = webaddress.substr(css+3);
	string hostname;
	string location;
	css = rest.find("/");
	if (css != string::npos) {
		hostname = rest.substr(0, css);
		location = rest.substr(css);
	} else {
		hostname = rest;
		location = "/";
	}
	if (location.empty()) {
		location = "/";
	}

	string newline({0x0d, 0x0a});

	stringstream request;
	request << "GET "   << location << " HTTP/1.1" << newline;
	request << "Host: " << hostname << newline;
	request << "User-Agent: HumdrumFile Downloader 2.0 ("
		     << __DATE__ << ")" << newline;
	request << "Connection: close" << newline;  // this line is necessary
	request << newline;

	unsigned short int port = 80;
	int socket_id = open_network_socket(hostname, port);
	if (::write(socket_id, request.str().c_str(), request.str().size()) == -1) {
		exit(-1);
	}

	#define URI_BUFFER_SIZE (10000)
	char buffer[URI_BUFFER_SIZE];
	int message_len;
	stringstream header;
	int foundcontent   = 0;
	int newlinecounter = 0;
	int i;

	// read the response header:
	while ((message_len = ::read(socket_id, buffer, 1)) != 0) {
		header << buffer[0];
		if ((buffer[0] == 0x0a) || (buffer[0] == 0x0d)) {
					newlinecounter++;
		} else {
					newlinecounter = 0;
		}
		if (newlinecounter == 4) {
			foundcontent = 1;
			break;
		}
	}
	if (foundcontent == 0) {
		cerr << "Funny error trying to read server response" << endl;
		exit(1);
	}

	// now read the size of the rest of the data which is expected
	int datalength = -1;

	// also, check for chunked transfer encoding:

	int chunked = 0;

	header << ends; // necessary?
	while (header.getline(buffer, URI_BUFFER_SIZE)) {
		int len = strlen(buffer);
		for (i=0; i<len; i++) {
			buffer[i] = std::tolower(buffer[i]);
		}
		if (strstr(buffer, "content-length") != NULL) {
			for (i=14; i<len; i++) {
				if (std::isdigit(buffer[i])) {
					sscanf(&buffer[i], "%d", &datalength);
					if (datalength == 0) {
						cerr << "Error: no data found for URI, probably invalid\n";
						cerr << "URL:   " << webaddress << endl;
						exit(1);
					}
					break;
				}
			}
		} else if ((strstr(buffer, "transfer-encoding") != NULL) &&
			(strstr(buffer, "chunked") != NULL)) {
			chunked = 1;
		}
	}

	// once the length of the remaining data is known (or not), read it:
	if (datalength > 0) {
		getFixedDataSize(socket_id, datalength, inputdata, buffer,
				URI_BUFFER_SIZE);

	} else if (chunked) {
		int chunksize;
		int totalsize = 0;
		do {
			chunksize = getChunk(socket_id, inputdata, buffer, URI_BUFFER_SIZE);
			totalsize += chunksize;
		} while (chunksize > 0);
		if (totalsize == 0) {
			cerr << "Error: no data found for URI (probably invalid)\n";
			exit(1);
		}
	} else {
		// if the size of the rest of the data cannot be found in the
		// header, then just keep reading until done (but this will
		// probably cause a 5 second delay at the last read).
		while ((message_len = ::read(socket_id, buffer, URI_BUFFER_SIZE)) != 0) {
			if (foundcontent) {
				inputdata.write(buffer, message_len);
			} else {
				for (i=0; i<message_len; i++) {
					if (foundcontent) {
						inputdata << buffer[i];
					} else {
						header << buffer[i];
						if ((buffer[i] == 0x0a) || (buffer[i] == 0x0d)) {
							newlinecounter++;
						} else {
							newlinecounter = 0;
						}
						if (newlinecounter == 4) {
							foundcontent = 1;
							continue;
						}
					}

				}
			}
		}
	}

	close(socket_id);
}



//////////////////////////////
//
//  HumdrumFileBase::getChunk --
//
// http://en.wikipedia.org/wiki/Chunked_transfer_encoding
// http://tools.ietf.org/html/rfc2616
//
// Chunk Format
//
// If a Transfer-Encoding header with a value of chunked is specified in
// an HTTP message (either a request sent by a client or the response from
// the server), the body of the message is made of an unspecified number
// of chunks ending with a last, zero-sized, chunk.
//
// Each non-empty chunk starts with the number of octets of the data it
// embeds (size written in hexadecimal) followed by a CRLF (carriage
// return and linefeed), and the data itself. The chunk is then closed
// with a CRLF. In some implementations, white space chars (0x20) are
// padded between chunk-size and the CRLF.
//
// The last chunk is a single line, simply made of the chunk-size (0),
// some optional padding white spaces and the terminating CRLF. It is not
// followed by any data, but optional trailers can be sent using the same
// syntax as the message headers.
//
// The message is finally closed by a last CRLF combination.

int HumdrumFileBase::getChunk(int socket_id, stringstream& inputdata,
		char* buffer, int bufsize) {
	int chunksize = 0;
	int message_len;
	char digit[2] = {0};
	int founddigit = 0;

	// first read the chunk size:
	while ((message_len = ::read(socket_id, buffer, 1)) != 0) {
		if (isxdigit(buffer[0])) {
			digit[0] = buffer[0];
			chunksize = (chunksize << 4) | strtol(digit, NULL, 16);
			founddigit = 1;
		} else if (founddigit) {
			break;
		} // else skipping whitespace before chunksize
	}
	if ((chunksize <= 0) || (message_len == 0)) {
		// next chunk is zero, so no more primary data0:w
		return 0;
	}

	// read the 0x0d and 0x0a characters which are expected (required)
	// after the size of chunk size:
	if (buffer[0] != 0x0d) {
		cerr << "Strange error occurred right after reading a chunk size" << endl;
		exit(1);
	}

	// now expect 0x0a:
	message_len = ::read(socket_id, buffer, 1);
	if ((message_len == 0) || (buffer[0] != 0x0a)) {
		cerr << "Strange error after reading newline at end of chunk size"<< endl;
		exit(1);
	}

	return getFixedDataSize(socket_id, chunksize, inputdata, buffer, bufsize);
}



//////////////////////////////
//
// getFixedDataSize -- read a know amount of data from a socket.
//

int HumdrumFileBase::getFixedDataSize(int socket_id, int datalength,
		stringstream& inputdata, char* buffer, int bufsize) {
	int readcount = 0;
	int readsize;
	int message_len;

	while (readcount < datalength) {
		readsize = bufsize;
		if (readcount + readsize > datalength) {
			readsize = datalength - readcount;
		}
		message_len = ::read(socket_id, buffer, readsize);
		if (message_len == 0) {
			// shouldn't happen, but who knows...
			break;
		}
		inputdata.write(buffer, message_len);
		readcount += message_len;
	}

	return readcount;
}



//////////////////////////////
//
// HumdrumFileBase::prepare_address -- Store a computer name, such as
//    www.google.com into a sockaddr_in structure for later use in
//    open_network_socket.
//

void HumdrumFileBase::prepare_address(struct sockaddr_in *address,
		const string& hostname, unsigned short int port) {

	memset(address, 0, sizeof(struct sockaddr_in));
	struct hostent *host_entry;
	host_entry = gethostbyname(hostname.c_str());

	if (host_entry == NULL) {
		cerr << "Could not find address for " << hostname << endl;
		exit(1);
	}

	// copy the address to the sockaddr_in struct.
	memcpy(&address->sin_addr.s_addr, host_entry->h_addr_list[0],
			host_entry->h_length);

	// set the family type (PF_INET)
	address->sin_family = host_entry->h_addrtype;
	address->sin_port = htons(port);
}



//////////////////////////////
//
// open_network_socket -- Open a connection to a computer on the internet.
//    Intended for downloading a Humdrum file from a website.
//

int HumdrumFileBase::open_network_socket(const string& hostname,
		unsigned short int port) {
	int inet_socket;                 // socket descriptor
	struct sockaddr_in servaddr;     // IP/port of the remote host

	prepare_address(&servaddr, hostname, port);

	// socket(domain, type, protocol)
	//    domain   = PF_INET(internet/IPv4 domain)
	//    type     = SOCK_STREAM(tcp) *
	//    protocol = 0 (only one SOCK_STREAM type in the PF_INET domain
	inet_socket = socket(PF_INET, SOCK_STREAM, 0);

	if (inet_socket < 0) {
		// socket returns -1 on error
		cerr << "Error opening socket to computer " << hostname << endl;
		exit(1);
	}
	if (connect(inet_socket, (struct sockaddr *)&servaddr,
			sizeof(struct sockaddr_in)) < 0) {
		// connect returns -1 on error
		cerr << "Error opening connection to coputer: " << hostname << endl;
		exit(1);
	}

	return inet_socket;
}

#endif


// END_MERGE

} // end namespace hum



