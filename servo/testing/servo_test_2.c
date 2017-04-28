#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
if(wiringPiSetupGpio() == -1){
 int errsv = errno;
 fprintf(stderr, "Failed to initialize GPIO, error%d\n",errsv);
 exit(EXIT_FAILURE);
}

pinMode(17, INPUT);
pinMode(18, PWM_OUTPUT);

// set pwm mode to mark space
pwmSetMode(PWM_MODE_MS);
// set overflow CCR level
pwmSetRange(2000);
// set PWM clock divider
pwmSetClock(192);
// write servo to center
pwmWrite(18, 120);
int i = 0;
while(1){
 for(i=100;i<200;i++){
  pwmWrite(18,i);
  delay(10);
 }
 for(i=200;i>100;i--){
  pwmWrite(18,i);
  delay(10);
 }
}
}
