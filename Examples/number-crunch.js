#!/usr/bin/env asteria-run

// Import the concurrent utils and add the exported functions (like map and reduce) 
// to the concurrent namespace.
require('concurrent_utils').install(concurrent);
var map = concurrent.map;
var reduce = concurrent.reduce;

// A list-generator
function seq (to) {
	var from = 0;
	var result = [];
	while(from < (to + 1)) {
		result[from] = from;
		from++;
	}
	return result;
}

// A fast fibonacci implementation
function fastfib(n) {
	var i;
	var fibs = new Array();

	fibs.push(0);
	fibs.push(1);

	fibs.push(fibs[0] + fibs[1]);
	fibs.shift();

	for(i=0; i<n; i++){
		fibs.push(fibs[0] + fibs[1]);
		fibs.shift();
	}  
	return fibs[0];
}
	
// Calculate the sum of the first 100 fibonacci numbers using all
// available cores. Actually the calculation is so fast that you should
// not see a 100% cpu usage. The program will spend the most time with
// socket send and receive calls.

var sum = reduce(map(seq(10000), fastfib), 0, function (a,b) {return a+b;});

print("Sum of the first 10000 fibonacci numbers: ", sum, '\n');
