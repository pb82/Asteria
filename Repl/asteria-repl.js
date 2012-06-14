#!/usr/local/bin/asteria-run

var s = require('system');

var rl, running = true;
var quit = function () { running = false; return "Goodbye!"; }

function repl() {
	while(running) {
		var line = rl.readline('> ');
		if(line.length > 0) {
			rl.add_history(line);
			try {
				print(eval(line), '\n');
			} catch(ex) {
				print(ex, '\n');				
			}
		}
	}
}

s.system('clear');
print('-- Welcome to the asteria repl.', '\n');

// Check for the GNU readline module
// If not available, fall back to standard
// stdin functions.

if(checkModule('readline')) {
	rl = require('readline');
} else {
	print('-- GNU readline not available. Falling back to raw stdin.', '\n');
	rl = new Object();
	rl.readline = function(p) { 
		s.stdout.write(p);
		s.stdout.flush();
		return s.stdin.readLine();
	}
	rl.add_history = function (x) { /* dummy */ }
}

print('-- Execute quit() to leave.', '\n\n');

// -- Enter the loop
repl();
