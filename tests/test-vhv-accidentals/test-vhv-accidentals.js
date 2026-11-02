#!/usr/bin/env node
// Loads a compiled verovio toolkit (by default the one that verovio-script
// serves to verovio.humdrum.org), renders the example to MIDI exactly as VHV
// does, and checks that both notes sound as B-flat.
//
// Usage: node test-vhv-accidentals.js [path/to/verovio-toolkit-wasm.js]

const path = require("path");

const toolkitPath = path.resolve(process.argv[2] ||
	path.join(__dirname, "../../../verovio-script/scripts/verovio-toolkit-wasm.js"));

const HUMDRUM = "**kern\n4b-\n4b-\n=\n*-\n";
const EXPECTED = [70, 70]; // B-flat4, B-flat4 (a B-natural would be 71)

// Return the pitches of all note-on events in a standard MIDI file.
function midiNoteOns(buf) {
	const notes = [];
	let p = 14; // skip MThd chunk
	while (p < buf.length) {
		const len = buf.readUInt32BE(p + 4);
		const end = p + 8 + len;
		if (buf.toString("ascii", p, p + 4) === "MTrk") {
			let i = p + 8, status = 0;
			const varlen = () => { let v = 0, b; do { b = buf[i++]; v = (v << 7) | (b & 0x7f); } while (b & 0x80); return v; };
			while (i < end) {
				varlen(); // delta time
				if (buf[i] & 0x80) status = buf[i++];
				const type = status & 0xf0;
				if (status === 0xff) { i++; i += varlen(); }
				else if (status === 0xf0 || status === 0xf7) { i += varlen(); }
				else if (type === 0xc0 || type === 0xd0) { i += 1; }
				else {
					const key = buf[i], vel = buf[i + 1];
					if (type === 0x90 && vel > 0) notes.push(key);
					i += 2;
				}
			}
		}
		p = end;
	}
	return notes;
}

const verovio = require(toolkitPath);
verovio.module.onRuntimeInitialized = () => {
	const tk = new verovio.toolkit();
	console.log(`Toolkit: ${toolkitPath}`);
	console.log(`Verovio ${tk.getVersion()}`);

	tk.setOptions({ inputFrom: "auto" });
	tk.loadData(HUMDRUM);

	const notes = [...tk.getMEI().matchAll(/<note\b[^>]*>(?:[\s\S]*?<\/note>)?/g)].map(m => m[0]);
	notes.forEach((n, k) => {
		const pname = (n.match(/pname="(\w+)"/) || [])[1];
		const oct = (n.match(/oct="(\d+)"/) || [])[1];
		const accid = (n.match(/\baccid="(\w+)"/) || [])[1] || "-";
		const ges = (n.match(/accid\.ges="(\w+)"/) || [])[1] || "-";
		console.log(`MEI note ${k + 1}: ${pname}${oct} accid=${accid} accid.ges=${ges}`);
	});

	const pitches = midiNoteOns(Buffer.from(tk.renderToMIDI(), "base64"));
	console.log(`MIDI pitches: ${pitches.join(", ")} (expected ${EXPECTED.join(", ")})`);

	if (JSON.stringify(pitches) === JSON.stringify(EXPECTED)) {
		console.log("PASS: both notes sound as B-flat");
		process.exit(0);
	}
	console.log("FAIL: second B-flat is not sounding as B-flat (non-visual accidental dropped)");
	process.exit(1);
};
