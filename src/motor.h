#ifndef MOTOR_H_
#define MOTOR_H_

#define MOTOR1_PIN1 0x01
#define MOTOR1_PIN2 0x02
#define MOTOR2_PIN1 0x04
#define MOTOR2_PIN2 0x08

void motor_init(void);
void motor_set(char motors);
char motor_get(void);

#endif /* MOTOR_H_ */