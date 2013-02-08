var serialport = require("serialport"), SerialPort = serialport.SerialPort;

var sp = new SerialPort("/dev/tty.usbmodem1411", { 
  parser: serialport.parsers.readline("\n") 
});

var src = require('fs').readFileSync(__dirname + '/../test.evm', 'utf-8');

if (src.length > 1024) {
  console.error('Source too long:', src.length);
  process.exit(1);
}

sp.on("open", function () {
  sp.on('data', function (data) {
    if (data == String('Initializing.\r')) {
      sp.write(src + '\0', function (err, res) { });
    }
  }); 
});