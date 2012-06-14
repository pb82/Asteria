#!/usr/bin/env asteria-run

function applicator() {
	// Receive the next message sent to this process.
	var message = concurrent.receive();

	var fun = message.value[0];
	var arg = message.value[1];

	// Assume that argument 1 is a function to run and argument 2 is the
	// argument.
	try {
		concurrent.send(message.sender, fun(arg));
	} catch (ex) {
		// In case of an error we throw the exception back to the sender
		// of the message. 
		//
		// This functionality is very limited at the moment.
		// We can only thow strings or instances of Error.
		// The receiver of the exception will always get an Error (instance
		// if class Error).
		concurrent.throw(message.sender, ex);
	}
}

concurrent.map = function (list,lambda) {	
	var result = new Array(list.length);
	
	// Iterate over the input array and spawn a new applicator
	// for every item. To spawn a process for every item in the array
	// is actually a pretty bad thing to do. A better implementation
	// would limit the number of processes to a reasonable amount and
	// balance the items over the processes.
	for(var index = 0; index < list.length; index++) {
		var pid = concurrent.spawn(applicator);
		// Send the function and the current array item to the applicator.
		// It will execute the function on the item.
		result[index] = concurrent.sendForProxy(pid, [lambda, list[index]]);
	}
	
	// Collect the items. By using 'sendForProxy' the array contains
	// now a list of proxies for the actual results. By accessing the
	// 'value' property we query the result. This query may block if
	// the calculation has not finished yet.
	for(var index = 0; index < result.length; index++) {
		try {
			result[index] = result[index].value;
		} catch(ex) {
			result[index] = ex;
		}
	}
	
	return result;
}

// Map square-root over an array. We need to wrap Math.sqrt in
// a function, since it is not implemented in JavaScript. Only
// JavaScript functions can be sent to other processes (it is
// of course possible that these functions contain native calls).
var t = concurrent.map([32,33,34], function (x) {	
	return Math.sqrt(x);
});

print(t, '\n');
