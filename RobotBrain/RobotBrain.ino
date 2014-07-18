#include <Wire.h>
#include <HMC5883L.h>
#include <Compass.h>
#include <Motor.h>
#include <TinyGPS.h>

#define NORTH 0
#define TERMBAUD  9600
#define GPSBAUD  4800
#define PI 3.14159265358979323846

TinyGPS gps;
Compass compass;
Motor motor;

typedef struct Point {
  float latitude;
  float longitude;
  float distance;
  bool checked;
  int sequence;
} Point;


int count = 5;
int acceleration = 40;

int use = 0;

//Declination Angle of Jaraguá do Sul is -0.31 and from Mauá is -0.35
const float declinationAngle = -0.35;

Point point[3];
Point currentPoint;


void getgps(TinyGPS &gps, float *latitude, float *longitude);

bool inited = false;

void setup(){

  Serial.begin(TERMBAUD);
  Serial1.begin(GPSBAUD);
  
  pinMode(49,OUTPUT);
  digitalWrite(49,HIGH);

  compass.init(declinationAngle);

  motor.defineRight(3,2,4);
  motor.defineLeft(5,6,7);
  motor.defineCompass(compass,20);

  point[0].latitude = -23.64667;
  point[0].longitude = -46.57254;
  point[0].sequence = 1;
  point[0].checked = false;

  point[1].latitude = -23.64660;
  point[1].longitude = -46.57268;
  point[1].sequence = 2;
  point[1].checked = false;

  point[2].latitude = -23.64650;
  point[2].longitude = -46.57261;
  point[2].sequence = 2;
  point[2].checked = false;

}




void loop(){
  
  
  while(Serial1.available())     
  {
    int c = Serial1.read();
    if(gps.encode(c))
    {

      getgps(gps, &currentPoint.latitude, &currentPoint.longitude);  
      
      //Serial.print("LAT___: ");Serial.print(currentPoint.latitude,5);
      //Serial.println("LON___: ");Serial.print(currentPoint.longitude,5);

      //float currentAngulation = compass.getCurrentAngulation();

      if(!point[use].checked) {
        while(!motor.turnToNorth());
        point[use].checked = true;
      } else {

        /*Nunca modificar essa função*/
        float dlat = point[use].latitude - currentPoint.latitude;
        float dlong = point[use].longitude - currentPoint.longitude; 
        float validationAngle = atan2(dlong,dlat) * 180 / PI;
        /*Até aqui*/

        int distlat = (point[use].latitude * 100000) - (currentPoint.latitude * 100000);
        int distlong = (point[use].longitude * 100000) - (currentPoint.longitude * 100000);

        if(distlat < 0) {
          distlat *= -1;
        }
        if(distlong < 0) {
          distlong *= -1; 
        }

        if(validationAngle < 0) {
          validationAngle *= -1;
          validationAngle = 360 - validationAngle;
        }

        //Serial.print("DIST LAT: ");Serial.print(distlat);
        //Serial.print("DIST LONG: ");Serial.println(distlong);

        if(distlat < 6 && distlong < 6) {
          acceleration = 80;
          count = 9;
        } else {
          count = 5;
        }


        if(distlat > 1 || distlong > 1) {

          while  (!motor.turnToDirection(validationAngle));

          if(count == 5 && acceleration < 200) {
            count = 0;
            acceleration += 10;
          }
          motor.front(acceleration); 
          if(count < 5) {
            count++; 
          }

        } else {
          motor.stop();
          for(int i = 0; i < 3; i++) {
            digitalWrite(49,LOW);
            delay(1000);
            digitalWrite(49,HIGH);
            delay(1000);
          }
          use++; 
        }

      }

    }
  }

    /*while(!goTo(allToGo.p1)); //BLUMENAU
    while(!goTo(allToGo.p2)); //OTACILIO
    while(!goTo(allToGo.p3)); //PALHOCA*/


}




/*
 * Retorna a latitude e longitude em relacao ao ponto atual
 */
void getgps(TinyGPS &gps, float *latitude, float *longitude) {
  float latit, longi;
  gps.f_get_position(&latit, &longi);
  *latitude = latit;
  *longitude = longi;
}
