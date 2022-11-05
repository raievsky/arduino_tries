#include <ros.h>
#include <std_msgs/Empty.h>
#include <Arduino.h>

char buffer[255];

// void setup() {
//   Serial.begin(9600);
//   Serial1.begin(9600);
//   // put your setup code here, to run once:
// }
// 
// void loop() {
//   if (Serial1.available())
//   {
//     Serial1.readBytes(buffer, 255);
//     Serial.println(buffer);
//   }
// 
//   delay(500);
// }



ros::NodeHandle nh;

void messageCb( const std_msgs::Empty& toggle_msg){
  digitalWrite(LED_BUILTIN, HIGH-digitalRead(LED_BUILTIN));   // blink the led
}

ros::Subscriber<std_msgs::Empty> sub("toggle_led", &messageCb );

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  nh.initNode();
  nh.subscribe(sub);
}

void loop()
{
  nh.spinOnce();
  delay(1);
}



