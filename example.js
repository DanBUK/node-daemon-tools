try {
  var daemon-tools = require('build/default/daemon-tools');
} catch (e) {
  try {
    var daemon-tools = require('daemon-tools');
  } catch (e) {
    console.log("Have you actually built the module yet?");
    console.log("eg. node-waf configure build install");
  }
}
var fs = require('fs');
var http = require('http');
var sys = require('sys');

var config = {
	lockFile: '/tmp/testd.pid'	//Location of lockFile
};

var args = process.argv;
var dPID;

// Handle start stop commands
switch(args[2]) {
	case "stop":
		var exit_val = 0;
		try {
			process.kill(parseInt(fs.readFileSync(config.lockFile)));
		} catch (e) {
			if (e.message == 'No such process') {
				sys.puts("Error: Process is not running or wrong PID value.");
				exit_val = 1;
			} else {
				sys.puts("Error unknown: " + sys.inspect(e));
				exit_val = 2;
			}
		}
		process.exit(exit_val);
		break;
		
	case "start":
		dPID = daemon-tools.start(false);
		daemon-tools.lock(config.lockFile);
		daemon-tools.closeIO(fs.openSync('/dev/null', 'w'));
		break;
		
	default:
		sys.puts('Usage: [start|stop]');
		process.exit(0);
}

// Start HTTP Server
http.createServer(function(req, res) {
	res.writeHead(200, {'Content-Type': 'text/html'});
	res.write('<h1>Hello, World!</h1>');
	res.close();
}).listen(8000);

