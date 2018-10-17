void setup() {
  Serial.begin(9600);
  byte msg = B11000101;

    // Fetch the payload, and see if this was the last one.


    String north="false";
    String east="false";
    String south="false";
    String west="false";
    String shape;
    String color;

    if (msg & B10000000 == B10000000) 
      north = "true";

    if (msg & B01000000 == B01000000) 
      east = "true";

    if (msg & B00100000 == B00100000) 
      south = "true";

    if (msg & B00010000 == B00010000) 
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
        shape = "red";
        break;
      case B00000010:
        shape = "green";
        break;
      case B00000011:
        shape = "blue";
        break;
      default:
        shape = "none";
        break;
    }

    Serial.println("0,0,north="+north+",east="+east+",south="+south+",west="+west+",tshape="+shape+",tcolor="+color);
}

void loop() {
  // put your main code here, to run repeatedly:

}
