#ifndef LED_H_
#define LED_H_

#define LED1 0x01
#define LED2 0x02
#define LED3 0x04
#define LED4 0x08
#define LED_ALL (LED1|LED2|LED3|LED4)

void led_init(void);
void led1_init(void);
void led_set(char leds);
char led_get(void);

#endif /* LED_H_ */