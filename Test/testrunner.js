#!/usr/bin/env asteria-run

var sys = require('system');
var shell = require('shell');
tests = [];

function register(path) {
	var suite = require(path);

	for(property in suite) {
		var test = suite[property];

		if(typeof test === 'function') {
			tests.push(test);
		}
	}
}

function run() {
	register('./modules/module-test');
	register('./modules/fs-test');
	register('./concurrency/concurrency-test');
	
	var result = true;

	for(index in tests) {
		var test = tests[index];
		print('Test: ', test.description, '... ');
		var value = true;
		try {
			value = test();			
		} catch (Ex) {
			print('(Exception)'); value = false;
		}
		result &= value;
		print(value ? 'Pass' : 'Fail', '\n');
	}

	return result;
}

sys.system('clear');
print('Running tests...\n');
print('----------------\n');
print('\n');
var result = run();
print('\n');
if(result) {
	print('All tests completed without errors.\n');
} else {
	print('There were errors in one or more tests.\n');
}
