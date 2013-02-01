var serialport = require("serialport");
var SerialPort = serialport.SerialPort; // localize object constructor

var sp = new SerialPort("/dev/tty.usbmodem1411", { 
  parser: serialport.parsers.readline("\n") 
});

var src = 'function main() {\n\
  $uf0(1, 3);\n\
  while (true) { }\n\
}';

if (src.length > 256) {
  console.error('Source too long:', src.length);
  process.exit(1);
}

sp.on("open", function () {
  sp.on('data', function (data) {
    console.log(String(data));
    if (data == String('Initializing...\r')) {
      sp.write(src + '\0', function (err, res) { });
    }
  }); 
});