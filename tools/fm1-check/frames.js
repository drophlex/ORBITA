// writes the installer's requests as frames files (cmd, length lo, length hi, args) for the host check
const fs = require('fs'), path = require('path');
global.atob = s => Buffer.from(s, 'base64').toString('binary');
const o = require(path.resolve(process.argv[2])), out = process.argv[3]; fs.mkdirSync(out, { recursive: true });
const dump = (reqs, file) => { const b = []; for (const [c, a] of reqs) b.push(c, a.length & 127, a.length >> 7, ...a); fs.writeFileSync(path.join(out, file), Buffer.from(b)); };
const bank = o.orbitaBank(); if (!o.orbitaCheck(bank)) { console.error('Orbita-32 bank: bad'); process.exit(1); }
dump(o.orbitaFeluccaRequests(bank), 'orbita_frames.bin'); dump(o.emptyBankRequests(), 'empty_frames.bin');
console.log('frames written:', o.orbitaNames(bank).join(' '));
