#include "cc253x.h"

/*
#define MOTOR1_PIN1   P1_0
#define MOTOR1_PIN2   P1_1
#define MOTOR2_PIN1   P1_4
#define MOTOR2_PIN2   P2_0

#define MOTOR1_PIN1_MASK  0x01
#define MOTOR1_PIN2_MASK  0x02
#define MOTOR2_PIN1_MASK  0x10
#define MOTOR2_PIN2_MASK  0x01
*/
#define MOTOR1_PIN1   P0_0
#define MOTOR1_PIN2   P0_4
#define MOTOR2_PIN1   P0_1
#define MOTOR2_PIN2   P0_5

#define MOTOR1_PIN1_MASK  0x01
#define MOTOR1_PIN2_MASK  0x10
#define MOTOR2_PIN1_MASK  0x02
#define MOTOR2_PIN2_MASK  0x20

static char motors_status = 0;

void motor_init(void)
{
/*
  P1SEL &= ~(MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK);
  P1DIR |=  (MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK);
  P2SEL &= ~MOTOR2_PIN2_MASK;
  P2DIR |=  MOTOR2_PIN2_MASK;
*/
  P0SEL &= ~(MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK | MOTOR2_PIN2_MASK);
  P0DIR |=  (MOTOR1_PIN1_MASK | MOTOR1_PIN2_MASK | MOTOR2_PIN1_MASK | MOTOR2_PIN2_MASK);
  MOTOR1_PIN1 = 0;
  MOTOR1_PIN2 = 0;
  MOTOR2_PIN1 = 0;
  MOTOR2_PIN2 = 0;
  motors_status = 0;
}

void motor_set(char motors)
{
  MOTOR1_PIN1 = motors & 0x01;
  MOTOR1_PIN2 = (motors & 0x02) >> 1;
  MOTOR2_PIN1 = (motors & 0x04) >> 2;
  MOTOR2_PIN2 = (motors & 0x08) >> 3;
  motors_status = motors;
}

char motor_get(void)
{
  return motors_status;
}
