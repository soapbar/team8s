void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {

  // CELL (0,0)
  byte msg = B10110000;
  int x = 0;
  int y = 0;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,0)
  msg = B10100000;
  x = 1;
  y = 0;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,0)
  msg = B01100000;
  x = 2;
  y = 0;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,1)
  msg = B11000000;
  x = 2;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,1)
  msg = B10100000;
  x = 1;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (0,1)
  msg = B00110000;
  x = 0;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (0,2)
  msg = B00010000;
  x = 0;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,2)
  msg = B10100000;
  x = 1;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,2)
  msg = B10100000;
  x = 2;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,2)
  msg = B01000000;
  x = 3;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,1)
  msg = B01010000;
  x = 3;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,0)
  msg = B01110000;
  x = 3;
  y = 0;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,1)
  msg = B01010000;
  x = 3;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,2)
  msg = B01000000;
  x = 3;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,3)
  msg = B11010000;
  x = 3;
  y = 3;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,2)
  msg = B01000000;
  x = 3;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,2)
  msg = B10100000;
  x = 2;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,2)
  msg = B10100000;
  x = 1;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (0,2)
  msg = B00010000;
  x = 0;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (0,3)
  msg = B10010000;
  x = 0;
  y = 3;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,3)
  msg = B10100000;
  x = 1;
  y = 3;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,3)
  msg = B11100000;
  x = 2;
  y = 3;
  updateGUI(msg, x, y);
  delay(10000);
}

void updateGUI(byte msg, int x, int y) {
  String north="false";
  String east="false";
  String south="false";
  String west="false";
  String shape;
  String color;

  String xcoord = String(x);
  String ycoord = String(y);

  if ((msg & B10000000) == B10000000) 
    north = "true";

  if ((msg & B01000000) == B01000000) 
    east = "true";

  if ((msg & B00100000) == B00100000) 
    south = "true";

  if ((msg & B00010000) == B00010000) 
    west = "true";

  switch (msg & B00001100) {
    case B00000100:
      shape = "circle";
      break;
    case B00001000:
      shape = "triangle";
      break;
    case B00001100:
      shape = "square";
      break;
    default:
      shape = "none";
      break;
  }

  switch (msg & B00000011) {
    case B00000001:
      color = "red";
      break;
    case B00000010:
      color = "green";
      break;
    case B00000011:
      color = "blue";
      break;
    default:
      color = "none";
      break;
  }

  Serial.println(xcoord+","+ycoord+","+"north="+north+",east="+east+",south="+south+",west="+west+",tshape="+shape+",tcolor="+color);
}
