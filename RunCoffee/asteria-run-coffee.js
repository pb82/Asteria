#!/usr/bin/env asteria-run

var coffee_compiler = require('coffee-script');
var fs = require('fs');

function run() {
	// The path to the coffe script is stored at position two of the
	// arguments array.
	if(typeof argv[2] == 'undefined' || argv[2].length == 0) {
		return;
	}
	
	var script = new fs.file(argv[2], 'r');
	var lines = script.lines();
	script.close();
	
	// Remove the interpreter invokation
	lines.splice(0,1);
	var str = "";
	for(var index=0; index < lines.length; index++) {
		var line = lines[index];
		if(typeof line != 'undefined' && line.length > 0) {
			str += lines[index];
			str += '\n';
		}
	}
	
	//var js = coffee_compiler.compiler.compile(str);
	//print(js);
	coffee_compiler.compiler.run(str);
}

run();
