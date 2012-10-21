Speaker[] speakers;
float jitter = 4;
float mx, my;

void setup() {
  size(512, 512);
  frameRate(60);
  strokeWeight(0);
  noFill();
  noSmooth();

  if (false) {
    debug = true;
    speakers = new Speaker[1];
    speakers[0] = new Speaker(width / 4, height / 4, width / 2, height / 2);
  } 
  else {
    debug = false;
    speakers = new Speaker[32];
    float speakerDistance = 100;
    for (int i = 0; i < speakers.length; i++) {
      while (true) {
        float cx = random(width), cy = random(height);
        boolean good = true;
        for (int j = 0; j < i; j++) {
          if (dist(cx, cy, speakers[j].x, speakers[j].y) < speakerDistance) {
            good = false;
            break;
          }
        }
        if (good) {
          speakers[i] = new Speaker(cx, cy, width / 2, height / 2);
          break;
        }
      }
    }
  }
}

void draw() {
  mx = mouseX;
  my = mouseY;

  background(0);
  for (int i = 0; i < speakers.length; i++) {
    speakers[i].update();
    speakers[i].draw();
  }
  ellipse(mx, my, 8, 8);
}

