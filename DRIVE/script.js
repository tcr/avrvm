function pinMode (pin, val) {
  $uf4(pin, val); // 0 = output, 1 = input
}

function pinOutput (pin, val) {
  $uf6(pin, val);
}

function delay (ms) {
  $uf3(ms);
}

function main () {
  pinMode(13, 0);
  while (true) {
    pinOutput(13, 1);
    delay(200);
    pinOutput(13, 0);
    delay(200);
  }
}