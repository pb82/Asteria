#!/usr/bin/env asteria-run

var sys = require('system');

function greeter() {
	while(1) {
		// Receive the next message sent to this process.
		var message = concurrent.receive();

		// Get the sender and return him a greeting.
		concurrent.send(message.sender, 'Hello, ' + message.value);
	}
}

// Spawn a new process that runs our greet function and call
// it 'greet-service'.
concurrent.spawn(greeter, 'greet-service');
var name = sys.stdin.readLine('Please enter your name: ');

// Send a message containing the user name to the greet-service.
concurrent.send('greet-service', name);

// Wait for a reply and print it.
print(concurrent.receive().value, '\n');
