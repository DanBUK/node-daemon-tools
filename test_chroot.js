#!/usr/bin/env node

var test_folder = "./chroot";

var sys = require('sys');
var fs = require('fs');
var daemon = require('./build/default/daemon');

fs.readdir("./", function (err, files) {
  sys.puts('Current directory: ' + process.cwd());
  sys.puts('err: ' + sys.inspect(err));
  sys.puts('files: ' + sys.inspect(files));
  daemon.chroot(test_folder);
  fs.readdir("./", function (err, files) {
    sys.puts('Current directory: ' + process.cwd());
    sys.puts('err: ' + sys.inspect(err));
    sys.puts('files: ' + sys.inspect(files));
  });
});




