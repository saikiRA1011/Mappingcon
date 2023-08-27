import processing.serial.*;

Serial port;

PGraphics pg;

void setup() 
{
  size(1000,1000);
  port = new Serial(this,"COM4",9600);
  background(255);
  
  pg = createGraphics(1000,1000);
  pg.beginDraw();
  pg.background(255);
  pg.endDraw();
}

class robot
{
  float x, y;
  float dx, dy;
  float oldDx,oldDy;
  float oldAx,oldAy;
  float speed;
  float arg;
  float darg;
  float oldDarg;
  
  int now,old;
  float dt;
  
  public robot()
  {
    x = width/2;
    y = height/2;
    dx = 0; dy = 0;
    oldDx = 0; oldDy = 0;
    oldAx = 0; oldAy = 0;
    arg = 0;
    darg = 0;
    oldDarg = 0;
    speed = 0;
    now=0;
    old=0;
  }
  
  public void move(float ax,float ay, float gz,float beamLength)
  {
    now=millis();
    dt=(now-old)/1000.0;
    //println("now : "+now+" old : "+old);
    old=now;
    
    add_darg(radians(gz));
    add_dx(ax);
    add_dy(ay);
    x +=100*(dt*dx*cos(arg)+dt*dy*sin(arg));
    y +=100*(dt*dy*cos(arg)+dt*dx*sin(arg));
    
    //println("ax : "+ax+" ay : "+ay+" dx : "+dx+" dy : "+dy);
    
    //println("ax : "+ax+" dx : "+dx+" x : "+x);
    
    
    fill(255);
    rect(0,0,width,height);
    image(pg,0,0);
    draw_robot();
    draw_beam(beamLength);
  }
  
  public void add_speed(float _speed)
  {
    speed = constrain(speed + _speed, - 3.0,3.0);
    dx = speed * cos(arg);
    dy = speed * sin(arg);
  }
  
  public void add_dx(float ax)
  {
    dx+=(oldAx+ax)*dt/2.0;
    oldAx=ax;
  }
  
  public void add_dy(float ay)
  {
    dy+=(oldAy+ay)*dt/2.0;
    oldAy=ay;
  }
  
  public void add_darg(float _darg)
  {
    arg+=((oldDarg+_darg)*dt/2.0);
    oldDarg=_darg;
  }
  
  public void stand(float beamLength)
  {
    stand_arg();
    stand_speed();
    move(0,0,0,beamLength);
  }
  
  public void stand_arg()
  {
    darg /= 2.0;
  }
  
  public void stand_speed()
  {
    dx/=2.0;
    dy/=2.0;
  }
  
  private void draw_robot()
  {
    stroke(0);
    fill(200,0,0);
    circle(x,y,40);
    
    // センサ方向
    fill(0,0,200);
    circle(x + 15 * cos(arg+radians(-90)),y + 15 * sin(arg+radians(-90)),5);
    
    // 前方
    fill(0);
    circle(x+15*cos(arg),y+15*sin(arg),5);
  }
  
  private void draw_beam(float beamLength)
  {
    stroke(0);
    if(beamLength>100) return;
    line(x + 15 * cos(arg+radians(-90)),y + 15 * sin(arg+radians(-90)),x + (beamLength + 15) * cos(arg+radians(-90)),y + (beamLength + 15) * sin(arg+radians(-90)));
    
    pg.beginDraw();
    pg.fill(0);
    pg.circle(x + (beamLength + 15) * cos(arg+radians(-90)),y + (beamLength + 15) * sin(arg+radians(-90)),5);
    pg.endDraw();
  }
}



boolean ok = false;
int gomiCount=0;

boolean keyMap[] = {false,false,false,false};
int time=0;
float oldTemp=15;
robot rb = new robot();
float duration=0;
float ax=0,ay=0,az=0,gx=0,gy=0,gz=0;
final float alpha=0.9;
boolean brake=false;

void draw()
{
  if (keyPressed) {
    port.write(key);
    if (key == 'o') {
      rb.old=millis();
      rb.x=width/2;
      rb.y=height/2;
      ok = true;
    }
  }
  
  if (ok) {
    
    float distance=0;
    float temp=0;
    if (port.available()>0) {
      
      String data = port.readStringUntil('\n');
      println(data);
      if(data ==null){
        sendMeirei();
        return;
      }
        
      
      String[] dataList=split(data,",");
      
      if(gomiCount<3){
        gomiCount++;
        return;
      }
      if(dataList.length<8){ 
        sendMeirei();
        return;
      }
      
      temp=float(dataList[0]);
      ax=alpha*(float(dataList[1]))/16384.0*9.80665+(1-alpha)*ax;
      ay=alpha*(float(dataList[2]))/16384.0*9.80665+(1-alpha)*ay;
      az=alpha*(float(dataList[3]))/16384.0*9.80665+(1-alpha)*az;
      gx=alpha*(float(dataList[4]))/32767.0*250.0+(1-alpha)*gx;
      gy=alpha*(float(dataList[5]))/32767.0*250.0+(1-alpha)*gy;
      gz=alpha*(float(dataList[6]))/32767.0*250.0+(1-alpha)*gz;
      
      duration=alpha*float(dataList[7])+(1-alpha)*duration;
      
      distance=(duration/2.0)*(331.5+temp*0.6)*100.0/1000000.0;
      
      if(temp==255){
        temp=oldTemp;
      }
      else{
        oldTemp=temp;
      }
      
    }
    else{
      sendMeirei();
    }
  if(brake) rb.stand(distance);
  else rb.move(5*az,5*ay,gx,distance); 
  //println("ax : "+ax+" ay :"+ay+" az : "+az);
  
  }
  
}

void sendMeirei()
{
    brake=false;
    int sousin=-1;
      
      if (!(keyMap[0] ^ keyMap[1])) {        // ブレーキ中
        sousin=0;
        brake=true;
        port.write(0);
      }
      else if(keyMap[2] && keyMap[3]){   // ゆっくり
        if(keyMap[0]){        // 前進
          sousin=1;
          port.write(1);              
        }
        else if(keyMap[1]){   // 後退
          sousin=2;
          port.write(2);
        }
      }
      else if (keyMap[0] && keyMap[2]) {     // 左前
        sousin=3;
        port.write(3);
      }
      else if (keyMap[0] && keyMap[3]) {     // 右前
        sousin=4;
        port.write(4);
      }
      else if (keyMap[1] && keyMap[2]){      // 左後
        sousin=5;
        port.write(5);
      }
      else if (keyMap[1] && keyMap[3]){      // 右後
        sousin=6;
        port.write(6);
      }
      else if (keyMap[0]) {
        sousin=7;
        port.write(7);                     // 前進
      }
      else if (keyMap[1]){
        sousin=8;
        port.write(8);
      }
}

void keyPressed()
{
  switch(key) {
    case 'w':
    keyMap[0] = true;
    break;
    case 's':
    keyMap[1] = true;
    break;
    case 'a':
    keyMap[2] = true;
    break;
    case 'd':
    keyMap[3] = true;
    break;
  }
}

void keyReleased()
{
  switch(key) {
    case 'w':
    keyMap[0] = false;
    break;
    case 's':
    keyMap[1] = false;
    break;
    case 'a':
    keyMap[2] = false;
    break;
    case 'd':
    keyMap[3] = false;
    break;
  }
}
