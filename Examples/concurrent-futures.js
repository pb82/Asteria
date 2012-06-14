#!/usr/bin/env asteria-run

function applicator() {
	while(1) {
		// Receive the next message sent to this process.
		var message = concurrent.receive();

		var fun = message.value[0];
		var arg = message.value[1];

		// Assume that argument 1 is a function to run and argument 2 is an array
		// containing it's arguments.
		try {
			concurrent.send(message.sender, fun.apply(this, arg));
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
}

// Launch the applicator function in a new process
concurrent.spawn(applicator, 'applicator-service');

// Send a function and some arguments to the applicator. By using the 'sendForProxy'
// function, a result is received immidiately and the program continues execution.
// The result however is a proxy that will fetch you the actual result when you need it
// and does not block.
//
// Note that you can send almost every type of data to another process,
// including: Strings, Numbers, (nested) Arrays, Functions.
// Sending objects is not possible atm, but could be included in the
// future. However an object could be sent by JSONifying it and send it
// as a string.
var result = concurrent.sendForProxy('applicator-service', [
	// The function that we run in parallel. Just a multiplication.
	function(a, b) {
		return a * b;
	}, 

	// The arguments.
	[128,64]
]);

// result.value will block if the calculation has not been finished at this point.
// Note that if an error occurs and the applicator does not return a result, then
// the following line is doomed to wait forever. 
// 
// Future versions  will therefore have a timeout (maybe as a property of the proxy object).
try {
	print('The result is: ', result.value, '\n');
} catch(ex) {
	print(ex, '\n');
}
