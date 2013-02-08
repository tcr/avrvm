var fs = require('fs');
var serialport = require("serialport"), SerialPort = serialport.SerialPort;

var sp = new SerialPort("/dev/tty.usbmodem1411", { 
  parser: serialport.parsers.readline("\n") 
});

sp.on("open", function () {
  sp.on('data', console.log);
});

function upload () {
  var src = require('fs').readFileSync(__dirname + '/../DRIVE/script.js', 'utf-8');
  if (src.length > 1024) {
    console.error('Source too long:', src.length);
    return;
  }

  var size = [(src.length >> 8) & 0xff, src.length & 0xff];
  sp.write(new Buffer(size), function () { });
  setTimeout(function () {
    sp.write(src + '\0', function () { })
  }, 500);
}

/*
fs.watch(__dirname + '/../DRIVE/script.js', function () {
  upload();
});
*/
if (process.argv.length > 2) {
  setTimeout(upload, 2000);
}