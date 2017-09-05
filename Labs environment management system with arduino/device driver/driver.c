/*
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>
#include <asm/gpio.h>
#include <linux/wait.h>
 */

#include <linux/time.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/slab.h>


#define IOM_DRIVER_MAJOR 264	// ioboard buzzer device major number
#define IOM_DRIVER_NAME "driver"	// ioboard buzzer device name
#define IOM_BUZZER_ADDRESS 0x04000020 // pysical address
#define IOM_FPGA_STEP_MOTOR_ADDRESS 0x0400000C // pysical address
#define IOM_FPGA_DOT_ADDRESS 0x04000210 
#define IOM_FPGA_DEMO_ADDRESS 0x04000300 
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x04000100 
#define IOM_LED_ADDRESS 0x04000016 
#define IOM_DEMO_ADDRESS 0x04000300
#define IOM_FND_ADDRESS 0x04000004

#define LED_GPBCON 0x11400040   
#define LED_GPBDAT 0x11400044

#define FND_GPL2CON 0x11000100
#define FND_GPL2DAT 0x11000104
#define FND_GPE3CON 0x11400140
#define FND_GPE3DAT 0x11400144

#define UON 0x00 // IOM
#define UOFF 0x01 // IOM
//Global variable

static struct struct_mydata {	//my structure for this project.
   struct timer_list timer;
   int count;
   int flag;
};
struct struct_mydata timer_for_fnd_print;
struct struct_mydata timer_for_check_flag;
//struct timespec current_time;
//struct timespec end_time;
//struct timespec start_time;

static unsigned char *fnd_data;
static unsigned int *fnd_ctrl;
static unsigned char *fnd_data2;
static unsigned int *fnd_ctrl2;
static unsigned char *led_data;
static unsigned int *led_ctrl;

static int driver_usage = 0;
static unsigned char *iom_fpga_buzzer_addr;
static unsigned char *iom_demo_addr;
static unsigned char *iom_fpga_step_motor_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_demo_addr_fnd;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_demo_addr_dot;
static unsigned char *iom_fpga_text_lcd_addr;
static unsigned char *iom_demo_addr_lcd;
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_demo_addr_led;


int jari=1; // index for print fnd.
static char *buffer = NULL;


//variable about write.
unsigned char buzzer_write;
unsigned char motor_write[3];
int ideal_temperature[4];
int current_temperature[4];
int reserve_start[4];
int reserve_end[4];
int current_time[4];
int ideal_temperature_int;
int real_temperature;
int mode;
int mode_flag=3;
int spin_flag = 0;
int button_fpga;
char text_lcd_message[33];
int manual_auto;
int humidity;
int current_time_int;
int reserve_start_int;
int reserve_end_int;
int initial = 0;

//dotmatrix for electric fan.
char electric_fan_left[5][10] = {{0,0,0,0,0,0,0,0,0,0,},
   {0x1C, 0x1C, 0x1C, 0x1C, 0xFF, 0xFF, 0x1C, 0x1C, 0x1C, 0x1C},
   {0x06, 0x06, 0x4C, 0x6C, 0x3C, 0x1E, 0X1B, 0x19, 0x30, 0x30},
   {0x41, 0x63, 0x36, 0x1C, 0x08, 0x08, 0x1C, 0x36, 0x63, 0x41},
   {0x30, 0x30, 0x19, 0x1B, 0x1E, 0x3C, 0x6C, 0x4C, 0x06, 0x06}
};

char electric_fan_right[5][10] = {{0,0,0,0,0,0,0,0,0,0,},
   {0x1C, 0x1C, 0x1C, 0x1C, 0xFF, 0xFF, 0x1C, 0x1C, 0x1C, 0x1C},
   {0x30, 0x30, 0x19, 0x1B, 0x1E, 0x3C, 0x6C, 0x4C, 0x06, 0x06},
   {0x41, 0x63, 0x36, 0x1C, 0x08, 0x08, 0x1C, 0x36, 0x63, 0x41},
   {0x06, 0x06, 0x4C, 0x6C, 0x3C, 0x1E, 0X1B, 0x19, 0x30, 0x30}
};
// define functions...
ssize_t iom_driver_write(struct file *inode, int *gdata, size_t length, loff_t *off_what);
ssize_t iom_driver_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_driver_open(struct inode *minode, struct file *mfile);
int iom_driver_release(struct inode *minode, struct file *mfile);
irqreturn_t inter_handler(int irq, void* dev_id, struct pt_regs* reg);
static void kernel_timer_blink1(unsigned long timeout);	//call this function on every 1/100 seconds.
static void kernel_timer_blink2(unsigned long timeout);	//call this function on every 1/100 seconds.
void gpio_fnd_write(int jari, int start_chr);	//fnd_write function.
void buzzer_driver_write(int ideal_temper, int real_temper);
void step_motor_driver_write(int ideal_temper, int real_temper);
void gpio_led_write(unsigned long led_buff);
void iom_fpga_text_lcd_write();
void set_text_auto();
void set_text_manual();
void set_text_base();
void set_text_ideal();
void set_text_start();
void set_text_end();
void set_text_current();
int reset_clock();
void iom_fpga_led_write(int led_buff);

// define file_operations structure
struct file_operations iom_buzzer_fops =
{
   .owner	= THIS_MODULE,
   .open	= iom_driver_open,
   .write	= iom_driver_write,
   .read	= iom_driver_read,
   .release	= iom_driver_release,
};
/*
irqreturn_t inter_handler1(int irq, void* dev_id, struct pt_regs* reg){
   printk("interrupt1!!! = %x\n", gpio_get_value(S5PV310_GPX2(0)));
   mode_flag -= 1;
   if(mode_flag == 0)
   	mode_flag = 3;   
   return IRQ_HANDLED;
}
*/
irqreturn_t inter_handler4(int irq, void* dev_id, struct pt_regs* reg){
   printk("interrupt4!!! = %x\n", gpio_get_value(S5PV310_GPX2(3)));
   return IRQ_HANDLED;
}

irqreturn_t inter_handler5(int irq, void* dev_id, struct pt_regs* reg){		//when push SW6
   printk("interrupt5!!! = %x\n", gpio_get_value(S5PV310_GPX2(4)));
   mode_flag+=1;
   if(mode_flag == 4)
   	mode_flag = 1;
   else if(mode_flag == 3)
        mode_flag = 6;
   else if(mode_flag == 7)
        mode_flag = 3;
   
   if(mode_flag == 1){
      set_text_base();
      set_text_start();
   }
   else if(mode_flag == 2){
      set_text_base();
      set_text_end();
   }
   else if(mode_flag == 3){
      set_text_base();
      set_text_ideal();
   }
   else if(mode_flag == 6){
      set_text_base();
      set_text_current();
   }
   
   return IRQ_HANDLED;
}

irqreturn_t inter_handler6(int irq, void* dev_id, struct pt_regs* reg){
   printk("interrupt6!!! = %x\n", gpio_get_value(S5PV310_GPX2(5)));
   return IRQ_HANDLED;
}

irqreturn_t inter_handler7(int irq, void* dev_id, struct pt_regs* reg){
   printk("interrupt7!!! = %x\n", gpio_get_value(S5PV310_GPX0(1)));
   return IRQ_HANDLED;
}


// when buzzer device open ,call this function
int iom_driver_open(struct inode *minode, struct file *mfile)
{
   int ret;
   if(driver_usage != 0) return -EBUSY;
   driver_usage = 1;

   /*
    *	SW2 : GPX2(0)
    *	SW3 : GPX2(1)
    *	SW4 : GPX2(2)
    *	SW6 : GPX2(4)
    *
    *	VOL+ : GPX2(5)
    *	POWER : GPX0(1)	
    *	VOL- : GPX0(1)
    *
    *	SW2,3,4,6 : PRESS-FALLING-0 / RELEASE-RISING-1
    *	VOL -, +  : PRESS-RISING-1 / RELEASE-FALLING-0		
    *	POWER	  : PRESS-FALLING-0 / RELEASE-RISING-1
    */
   //ret=request_irq(gpio_to_irq(S5PV310_GPX2(0)), &inter_handler1, IRQF_TRIGGER_RISING, "X2.0", NULL);//SW2
   ret=request_irq(gpio_to_irq(S5PV310_GPX2(3)), &inter_handler4, IRQF_TRIGGER_RISING,"X2.3", NULL);//VOL-
   ret=request_irq(gpio_to_irq(S5PV310_GPX2(4)), &inter_handler5, IRQF_TRIGGER_RISING, "X2.4", NULL);//SW6
   ret=request_irq(gpio_to_irq(S5PV310_GPX2(5)), &inter_handler6, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "X2.5", NULL);//VOL+
   ret=request_irq(gpio_to_irq(S5PV310_GPX0(1)), &inter_handler7, IRQF_TRIGGER_RISING, "X0.1", NULL);//POWER
   return 0;
}
// when buzzer device close ,call this function
int iom_driver_release(struct inode *minode, struct file *mfile)
{
   driver_usage = 0;
   //free_irq(gpio_to_irq(S5PV310_GPX2(0)), NULL);
   free_irq(gpio_to_irq(S5PV310_GPX2(3)), NULL);
   free_irq(gpio_to_irq(S5PV310_GPX2(4)), NULL);
   free_irq(gpio_to_irq(S5PV310_GPX2(5)), NULL);
   free_irq(gpio_to_irq(S5PV310_GPX0(1)), NULL);
   return 0;
}

void iom_fpga_text_lcd_write(){
	int i;
	for(i=0;i<32;i++)
		outb(text_lcd_message[i],(unsigned int)iom_fpga_text_lcd_addr+i);
}

void gpio_led_write(unsigned long led_buff){
   switch(led_buff){
      case 1: led_buff = 0xE0;break;
      case 2: led_buff = 0xD0;break;
      case 3: led_buff = 0xB0;break;
      case 4: led_buff = 0x70;break;
      default: led_buff = 0xFF;break;	//clear all led.
   }
   outb((char)led_buff, (unsigned int)led_data); 
}

void iom_fpga_dot_write(unsigned long option){
	int i;
	int index = (int)option;
	if(spin_flag == 1){
		for(i=0;i<10;i++)
			outb(electric_fan_right[index][i],(unsigned int)iom_fpga_dot_addr+i);
	}
	else{
		for(i=0;i<10;i++)
			outb(electric_fan_left[index][i],(unsigned int)iom_fpga_dot_addr+i);

	}

}
void buzzer_driver_write(int ideal_temper, int real_temper){
   if(manual_auto == 1){
  	 if(ideal_temper > real_temper + 600 || ideal_temper < real_temper - 600){
   	   buzzer_write = 1;
   	}      
   	else
   	   buzzer_write = 0;
   }
   else if(manual_auto == 0){
	if(ideal_temper > real_temper + 300 || ideal_temper < real_temper - 300){
   	   buzzer_write = 1;
   	}      
   	else
   	   buzzer_write = 0;
   }

   outb(buzzer_write,(unsigned int)iom_fpga_buzzer_addr);
}

void step_motor_driver_write(int ideal_temper, int real_temper){
   int check = ideal_temper - real_temper;
   int i;

   if(manual_auto == 0){   
      if(check > 150){
	 motor_write[0] = 1;
	 motor_write[1] = 1;
	 spin_flag = 1;
      }
      else if(check < -150){
	 motor_write[0] = 1;
	 motor_write[1] = 0;
	 check *= -1;
	 spin_flag = 2;
      }
      else{
	 motor_write[0] = 0;
	 spin_flag = 0;
      }
   }
   else{
      if(button_fpga == 1){
	 motor_write[0] = 1;
	 motor_write[1] = 0;
	 //check *= -1;
	 spin_flag = 2;
      }
      else if(button_fpga == 2){
	 motor_write[0] = 1;
	 motor_write[1] = 1;
	 spin_flag = 1;
      }
      else if(button_fpga == 4){
	 motor_write[0] = 0;
	 spin_flag = 0;
      }
      else if(button_fpga == 0){
	 motor_write[0] = 0;
	 spin_flag = 0;
      }	 
   }
   if(check <0 )
      check *= (-1);
   motor_write[2] = 250 - ((check*3)/10);
   if(motor_write[2] < 10)
      motor_write[2] = 10;

   for(i=0;i<3;i++)	
      outb(motor_write[i],(unsigned int)iom_fpga_step_motor_addr+i);
}


void iom_fpga_led_write(int led_buff){
	switch(led_buff){
		case 8: led_buff = 255;break;
		case 7:	led_buff = 254; break;
		case 6:	led_buff = 238; break;
		case 5:	led_buff = 236; break;
		case 4: led_buff = 204;  break;
		case 3: led_buff = 200;  break;
		case 2:	led_buff = 136;  break;
		case 1:	led_buff = 128;  break;
		default:led_buff = 0;  break;
	}
	outb((char)led_buff,(unsigned int)iom_fpga_led_addr);	    
}


void gpio_fnd_write(int jari, int start_chr){	//fnd_write function.
   char fnd_sel;
   char fnd_dat;
   fnd_sel = (char)jari;
   fnd_dat = (char)start_chr;

   switch(fnd_sel){
      case 1: fnd_sel = 0x02;break;
      case 2: fnd_sel = 0x04;break;
      case 3: fnd_sel = 0x10;break;
      case 4: fnd_sel = 0x80;break;
      case 5: fnd_sel = 0x96;break;	// to clear all fnd
   }
   if(jari == 2){
      switch(fnd_dat){
	 case 0: fnd_dat = 0x02;break;
	 case 1: fnd_dat = 0x9E;break;
	 case 2: fnd_dat = 0x24;break;
	 case 3: fnd_dat = 0x0C;break;
	 case 4: fnd_dat = 0x98;break;
	 case 5: fnd_dat = 0x48;break;
	 case 6: fnd_dat = 0xC0;break;
	 case 7: fnd_dat = 0x1E;break;
	 case 8: fnd_dat = 0x00;break;
	 case 9: fnd_dat = 0x08;break;
	 case 10:fnd_dat = 0xFF;break;	// to clear all fnd
      }
   }
   else{
      switch(fnd_dat){
	 case 0: fnd_dat = 0x03;break;
	 case 1: fnd_dat = 0x9F;break;
	 case 2: fnd_dat = 0x25;break;
	 case 3: fnd_dat = 0x0D;break;
	 case 4: fnd_dat = 0x99;break;
	 case 5: fnd_dat = 0x49;break;
	 case 6: fnd_dat = 0xC1;break;
	 case 7: fnd_dat = 0x1F;break;
	 case 8: fnd_dat = 0x01;break;
	 case 9: fnd_dat = 0x09;break;
	 case 10:fnd_dat = 0xFF;break;	// to clear all fnd
      }

   }
   outb (fnd_dat, (unsigned int)fnd_data);
   outb (fnd_sel, (unsigned int)fnd_data2);
}



// when write to buzzer device ,call this function
ssize_t iom_driver_write(struct file *inode, int *gdata, size_t length, loff_t *off_what)
{
   unsigned char value[3];
   const char *tmp = gdata;
   int i;
   int temp_temperature;
   //int test;
   if(copy_from_user(&real_temperature, gdata, sizeof(int)))	//get data from user space.
      return -EFAULT;
   temp_temperature = real_temperature;
   mode = (temp_temperature>>16)%256;
   
   real_temperature &= 0xFFFF;		// get real data part.

   //iom_fpga_dot_write(0);
   /*
   ideal_temperature[0] = 2;
   ideal_temperature[1] = 6;
   ideal_temperature[2] = 0;
   ideal_temperature[3] = 0;*/
   //for parameter test.
   // printk(KERN_WARNING"parameter : %d\n",test);
   //return 0;
   //
   if(mode == 3){
      if(initial == 1){
	set_text_base();
        set_text_ideal();
       //set_text_auto();
      }
      initial = 0;
      mode_flag = 3;
   }
   else if(mode == 2){
      mode_flag = 2;
   }
   else if(mode == 1){
      mode_flag = 1;
   }
   else if(mode == 4 && real_temperature == 0){
   	if(manual_auto == 0)
	   manual_auto = 1;
	else if(manual_auto == 1)
	   manual_auto = 0;
   }
   else if(mode == 6){
//	mode_flag = 6;			
   }
   printk("here you are?\n");

   del_timer_sync(&timer_for_fnd_print);
   //del_timer_sync(&timer_for_check_flag);
   timer_for_fnd_print.timer.expires = get_jiffies_64() + (1/100 * HZ);
   //timer_for_check_flag.timer.expires = get_jiffies_64() + (1/100 * HZ);
   timer_for_fnd_print.timer.data = (unsigned long)&timer_for_fnd_print;
   //timer_for_check_flag.timer.data = (unsigned long)&timer_for_check_flag;
   timer_for_fnd_print.timer.function = kernel_timer_blink1;
   //timer_for_check_flag.timer.function = kernel_timer_blink2;
   add_timer(&timer_for_fnd_print.timer);
   //add_timer(&timer_for_check_flag.timer);

   //int i;
   printk("mode : %d\n",mode);
   if(mode == 1){
      //set_text_base();
      //set_text_end();
      reserve_start_int = real_temperature;
      reserve_start[0] = real_temperature/1000;
      reserve_start[1] = (real_temperature/100)%10;
      reserve_start[2] = (real_temperature/10)%10;
      reserve_start[3] = real_temperature%10;

   }
   else if(mode == 2){
      //set_text_base();
      //set_text_end();
      reserve_end_int = real_temperature;
      reserve_end[0] = real_temperature/1000;
      reserve_end[1] = (real_temperature/100)%10;
      reserve_end[2] = (real_temperature/10)%10;
      reserve_end[3] = real_temperature%10;
   }
   else if(mode == 3){
      printk("ideal temperature : %d\n",real_temperature);
      if(real_temperature != 0){
      	ideal_temperature_int = real_temperature;
      			
      ideal_temperature[0] = real_temperature/1000;
      ideal_temperature[1] = (real_temperature/100)%10;
      ideal_temperature[2] = (real_temperature/10)%10;
      ideal_temperature[3] = real_temperature%10;
      }
      //set_text_base();
      //set_text_ideal();
      /*unsigned char remain_arr[4] = {0,};
	sprintf(remain_arr, "%04d", real_temperature);
	for(i=0;i<4;i++) 
	outb(remain_arr[i],(unsigned int)iom_fpga_fnd_addr+i);*/
   }
   else if(mode == 4){
	button_fpga = real_temperature;
	printk("button from fpga : %d\n",button_fpga);
   }
   else if(mode == 5){
	humidity = real_temperature;
      	humidity*=10;
	for(i=0;i<8;i++){
		humidity -= 125;
		if(humidity < 0){
		   iom_fpga_led_write(i+1);
		   break;
		}
	}
	humidity = real_temperature;
   }
   else if(mode == 6){
	current_time_int = real_temperature;	
   	current_time[0] = real_temperature/1000;
        current_time[1] = (real_temperature/100)%10;
        current_time[2] = (real_temperature/10)%10;
        current_time[3] = real_temperature%10;
   	if(manual_auto == 1){
		if(reserve_start_int == current_time_int){
			spin_flag = 1;
			button_fpga = 1;
		}
		if(reserve_end_int == current_time_int){
			spin_flag = 0;
			button_fpga = 4;
		}
	}
   }
   else if(mode == 7){
	initial = 1;
        printk("come here?\n");
      	gpio_led_write(5);
        iom_fpga_led_write(0);
	unsigned char remain_arr[4] = {0,};
  	for(i=0;i<4;i++) 
      		outb(remain_arr[i],(unsigned int)iom_fpga_fnd_addr+i);
      	manual_auto = 0;
   	set_text_base();
   	iom_fpga_text_lcd_write();
	iom_fpga_dot_write(0);
   	step_motor_driver_write(0,0);
   	gpio_fnd_write(5,10);
	buzzer_driver_write(0, 0);
	
     	printk("mission complete\n");
	return length;
    }
   else{
      /*if(mode_flag == 3){
	unsigned char remain_arr[4] = {0,};
	sprintf(remain_arr, "%04d", real_temperature);
	for(i=0;i<4;i++) 
	outb(remain_arr[i],(unsigned int)iom_fpga_fnd_addr+i);
	}*/

      unsigned char remain_arr[5] = {0,};
      sprintf(remain_arr, "%04d", real_temperature);
      for(i=0;i<4;i++) 
	 outb(remain_arr[i],(unsigned int)iom_fpga_fnd_addr+i);

      /* is about buzzer.
	 if buzzer_write == 1, then bell the buzzer.
       */
      /*buzzer_write = 0;
      outb(buzzer_write,(unsigned int)iom_fpga_buzzer_addr);*/
      
     
     
     
     
     
     
     
     
     
     
     
     
     
      buzzer_driver_write(ideal_temperature_int, real_temperature);
      step_motor_driver_write(ideal_temperature_int, real_temperature);
    
     
    if(manual_auto == 0){
	   set_text_auto();
   }
   else if(manual_auto == 1){
	   set_text_manual();
   } 
     
     
     
     
     
      /*
	 is about step_motor.
	 motor_write array's length is 3.
	 motor_write[0] is 0 or 1. 0 is stop rotating motor, 1 is start to rotate motor.
	 motor_write[1] is 0 or 1. 0 is left direction, 1 is right direction.
	 motor_write[2] is 10~250. 10 is the fastest rotating speed, 250 is the slowest rotating speed.
       */
   }

	
   //if(mode == 4){
   /*if(manual_auto == 0){
	   set_text_auto();
   }
   else if(manual_auto == 1){
	   set_text_manual();
   } */ 
   //}
   iom_fpga_text_lcd_write();
   //printk("time : %d\n",reset_clock());
    
   /*motor_write[0] = 1;
     motor_write[1] = 1;
     motor_write[2] = 10;

     for(i=0;i<3;i++)	
     outb(motor_write[i],(unsigned int)iom_fpga_step_motor_addr+i);*/
   return length;
}
// when read to buzzer device ,call this function
ssize_t iom_driver_read(struct file *inode, char *gdata, size_t length, loff_t *off_what)
{
   unsigned char value;
   char *tmp = gdata;
   value = inb((unsigned int)iom_fpga_buzzer_addr);
   if (copy_to_user(tmp, &value, 1))
      return -EFAULT;
   return length;
}

void set_text_base(void){
	int i;
	for(i=0;i<32;i++){
		text_lcd_message[i] = ' ';	
	}
	text_lcd_message[32] = '\0';
}

void set_text_auto(void){
	text_lcd_message[0] = ' ';	
	text_lcd_message[1] = ' ';	
	text_lcd_message[2] = ' ';
	text_lcd_message[3] = ' ';	
	text_lcd_message[4] = 'A';	
	text_lcd_message[5] = 'U';
	text_lcd_message[6] = 'T';
	text_lcd_message[7] = 'O';
	text_lcd_message[8] = 'M';	
	text_lcd_message[9] = 'O';	
	text_lcd_message[10] = 'D';
	text_lcd_message[11] = 'E';
	text_lcd_message[12] = ' ';
	text_lcd_message[13] = ' ';	
	text_lcd_message[14] = ' ';	
	text_lcd_message[15] = ' ';	
}

void set_text_manual(void){
	text_lcd_message[0] = ' ';	
	text_lcd_message[1] = ' ';	
	text_lcd_message[2] = ' ';
	text_lcd_message[3] = 'M';	
	text_lcd_message[4] = 'A';	
	text_lcd_message[5] = 'N';
	text_lcd_message[6] = 'U';
	text_lcd_message[7] = 'A';
	text_lcd_message[8] = 'L';	
	text_lcd_message[9] = 'M';	
	text_lcd_message[10] = 'O';
	text_lcd_message[11] = 'D';
	text_lcd_message[12] = 'E';
	text_lcd_message[13] = ' ';	
	text_lcd_message[14] = ' ';	
	text_lcd_message[15] = ' ';

}

void set_text_current(void){
	text_lcd_message[16] = ' ';	
	text_lcd_message[17] = ' ';	
	text_lcd_message[18] = 'C';
	text_lcd_message[19] = 'U';	
	text_lcd_message[20] = 'R';	
	text_lcd_message[21] = 'R';
	text_lcd_message[22] = 'E';
	text_lcd_message[23] = 'N';
	text_lcd_message[24] = 'T';	
	text_lcd_message[25] = ' ';	
	text_lcd_message[26] = 'T';
	text_lcd_message[27] = 'I';
	text_lcd_message[28] = 'M';
	text_lcd_message[29] = 'E';	
	text_lcd_message[30] = ' ';	
	text_lcd_message[31] = ' ';
}


void set_text_ideal(void){
	text_lcd_message[16] = 'H';	
	text_lcd_message[17] = 'O';	
	text_lcd_message[18] = 'P';
	text_lcd_message[19] = 'E';	
	text_lcd_message[20] = ' ';	
	text_lcd_message[21] = 'T';
	text_lcd_message[22] = 'E';
	text_lcd_message[23] = 'M';
	text_lcd_message[24] = 'P';	
	text_lcd_message[25] = 'E';	
	text_lcd_message[26] = 'R';
	text_lcd_message[27] = 'A';
	text_lcd_message[28] = 'T';
	text_lcd_message[29] = 'U';	
	text_lcd_message[30] = 'R';	
	text_lcd_message[31] = 'E';
}

void set_text_start(void){
	text_lcd_message[16] = ' ';	
	text_lcd_message[17] = 'R';	
	text_lcd_message[18] = 'E';
	text_lcd_message[19] = 'S';	
	text_lcd_message[20] = 'E';	
	text_lcd_message[21] = 'R';
	text_lcd_message[22] = 'V';
	text_lcd_message[23] = 'E';
	text_lcd_message[24] = 'D';	
	text_lcd_message[25] = ' ';	
	text_lcd_message[26] = 'S';
	text_lcd_message[27] = 'T';
	text_lcd_message[28] = 'A';
	text_lcd_message[29] = 'R';	
	text_lcd_message[30] = 'T';	
	text_lcd_message[31] = ' ';
}

void set_text_end(void){
	text_lcd_message[16] = ' ';	
	text_lcd_message[17] = ' ';	
	text_lcd_message[18] = 'R';
	text_lcd_message[19] = 'E';	
	text_lcd_message[20] = 'S';	
	text_lcd_message[21] = 'E';
	text_lcd_message[22] = 'R';
	text_lcd_message[23] = 'V';
	text_lcd_message[24] = 'E';	
	text_lcd_message[25] = 'D';	
	text_lcd_message[26] = ' ';
	text_lcd_message[27] = 'E';
	text_lcd_message[28] = 'N';
	text_lcd_message[29] = 'D';	
	text_lcd_message[30] = ' ';	
	text_lcd_message[31] = ' ';
}
/*
int reset_clock(){
   time_t *timer;
   struct tm *today;

   int hour;

   init_timer(&timer);
   today = localtime(&timer);

   hour = (today->tm_hour+9)%24;
   hour = today->tm_hour;
   return hour*100 +today->tm_min;
}
*/

static void kernel_timer_blink1(unsigned long timeout){	//call this function on every seconds. is about fnd_print.
   struct struct_mydata *p_data = (struct struct_mydata*)timeout;
   int i;
   static unsigned long option=0;
   static int count=0;
   if(initial == 0){
   if(mode_flag == 3){
      //ideal_temperature[0] = 2;
      //ideal_temperature[1] = 6;
      //ideal_temperature[2] = 0;
      //ideal_temperature[3] = 0;
      gpio_fnd_write(jari,ideal_temperature[jari-1]);
      gpio_led_write(1);
   }
   else if(mode_flag == 1){
      gpio_fnd_write(jari,reserve_start[jari-1]);
      gpio_led_write(2);
   } 
   else if(mode_flag == 2){
      gpio_fnd_write(jari,reserve_end[jari-1]);
      gpio_led_write(3);
   }
   else if(mode_flag == 6){
      gpio_fnd_write(jari,current_time[jari-1]);
      gpio_led_write(4);
   }
   /*else if(mode == 7){
      gpio_led_write(5);
      unsigned char remain_arr[4] = {0,};
  	for(i=0;i<4;i++) 
      		outb(remain_arr[i],(unsigned int)iom_fpga_fnd_addr+i);
      	manual_auto = 0;
   	set_text_base();
   	iom_fpga_text_lcd_write();
	iom_fpga_dot_write(0);
   	step_motor_driver_write(0,0);
   	gpio_fnd_write(5,10);
        return ;
   }*/
   jari+=1;
   if(jari == 5)
      jari = 1;	   
   

   count+=1;
   if(count == 25){
      	option+=1;
	if(option == 5)
	   option = 1;
   	if(spin_flag == 1)
		iom_fpga_dot_write(option);
	else if(spin_flag == 2)
		iom_fpga_dot_write(option);
	else
		iom_fpga_dot_write(1);
	count = 0;
   }
   
   p_data->count++;	
   timer_for_fnd_print.timer.expires = get_jiffies_64() + (1/100 * HZ);	//increment by time_interval.
   timer_for_fnd_print.timer.data = (unsigned long)&timer_for_fnd_print;
   timer_for_fnd_print.timer.function = kernel_timer_blink1;
   add_timer(&timer_for_fnd_print.timer);
   }
}

static void kernel_timer_blink2(unsigned long timeout){	//call this function on every seconds.
   struct struct_mydata *p_data = (struct struct_mydata*)timeout;
   int i;

   p_data->count++;	
   timer_for_check_flag.timer.expires = get_jiffies_64() + (1/100 * HZ);	//increment by time_interval.
   timer_for_check_flag.timer.data = (unsigned long)&timer_for_check_flag;
   timer_for_check_flag.timer.function = kernel_timer_blink1;
   add_timer(&timer_for_check_flag.timer);
}

int __init iom_driver_init(void)
{
   struct class *kernel_timer_dev_class=NULL;
   struct device *kernel_timer_dev=NULL;
   unsigned int get_ctrl_io=0;
   int result;
   result = register_chrdev(IOM_DRIVER_MAJOR, IOM_DRIVER_NAME, &iom_buzzer_fops);
   if(result < 0) {
      printk(KERN_WARNING"Can't get any major\n");
      return result;
   }

   kernel_timer_dev = device_create(kernel_timer_dev_class,NULL,MKDEV(IOM_DRIVER_MAJOR,0),NULL,IOM_DRIVER_NAME);
   if(kernel_timer_dev == NULL){
      printk("shit\n");
   }
   else{
      init_timer(&(timer_for_fnd_print.timer));
      init_timer(&(timer_for_check_flag.timer));
   }

   printk("seolma?\n"); 
   iom_fpga_step_motor_addr = ioremap(IOM_FPGA_STEP_MOTOR_ADDRESS, 0x4);
   iom_fpga_buzzer_addr = ioremap(IOM_BUZZER_ADDRESS, 0x1);
   iom_demo_addr = ioremap(IOM_DEMO_ADDRESS, 0x1);
   outb(UON,(unsigned int)iom_demo_addr);
   printk("init module, %s major number : %d\n", IOM_DRIVER_NAME, IOM_DRIVER_MAJOR);

   fnd_data = ioremap(FND_GPL2DAT, 0x01);
   fnd_data2 = ioremap(FND_GPE3DAT, 0x01);
   if(fnd_data==NULL)
   {
      printk("ioremap failed!\n");
      return -1;
   }
   fnd_ctrl = ioremap(FND_GPL2CON, 0x04);
   fnd_ctrl2 = ioremap(FND_GPE3CON, 0x04);
   if(fnd_ctrl==NULL)
   {
      printk("ioremap failed!\n");
      return -1;
   } else {
      outl(0x11111111,(unsigned int)fnd_ctrl);
      outl(0x10010110,(unsigned int)fnd_ctrl2);
   }
   outb(0xFF, (unsigned int)fnd_data);
   outb(0xFF, (unsigned int)fnd_data);
   iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
   iom_demo_addr_fnd = ioremap(IOM_DEMO_ADDRESS, 0x1);
   if(iom_fpga_fnd_addr == NULL){
      printk("error 1\n");
   }
   if(iom_demo_addr_fnd == NULL){
      printk("error 2\n");
   }
   outb(UON,(unsigned int)iom_demo_addr_fnd);

   ideal_temperature[0] = 2;
   ideal_temperature[1] = 6;
   ideal_temperature[2] = 0;
   ideal_temperature[3] = 0;
   ideal_temperature_int = 2600;
   led_data = ioremap(LED_GPBDAT, 0x01);  
   if(led_data==NULL) 
   {   
      printk("ioremap failed!\n");
      return -1;
   }

   led_ctrl = ioremap(LED_GPBCON, 0x04);  
   if(led_ctrl==NULL)  
   {
      printk("ioremap failed!\n");
      return -1;
   } else {
      buffer = (char*)kmalloc(1024, GFP_KERNEL);

      if(buffer != NULL)
	 memset(buffer, 0, 1024);

      get_ctrl_io|=(0x11110000); 
      outl(get_ctrl_io,(unsigned int)led_ctrl); 
   }
   outb(0xF0, (unsigned int)led_data);
   iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
   iom_demo_addr_dot = ioremap(IOM_FPGA_DEMO_ADDRESS, 0x1);
   if(iom_fpga_dot_addr==NULL){
	printk("error 7\n");
   }
   if(iom_demo_addr_dot==NULL){
 	printk("error 8\n");
   }
   outb(UON,(unsigned int)iom_demo_addr_dot);

   iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x20);
   iom_demo_addr_lcd = ioremap(IOM_FPGA_DEMO_ADDRESS, 0x1);
   if(iom_fpga_text_lcd_addr==NULL){
 	printk("error 5\n");
   }
   if(iom_demo_addr_lcd==NULL){
	printk("error 6\n");
   }
   outb(UON,(unsigned int)iom_demo_addr_lcd);
  

   
   iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
   iom_demo_addr_led = ioremap(IOM_DEMO_ADDRESS, 0x1);
   if(iom_fpga_led_addr== NULL){
	printk("error 3\n");
   }
   if(iom_demo_addr_led==NULL){
	printk("error 4\n");
   }
   outb(UON,(unsigned int)iom_demo_addr_led);
   
   
   
   
   
   reserve_start_int = -1;
   reserve_end_int = -1;
   initial = 0;
   manual_auto = 0;
   set_text_base();
   set_text_ideal();
   return 0;
}
void __exit iom_driver_exit(void)
{
   int i;
   unsigned char remain_arr[4] = {0,};
   for(i=0;i<4;i++) 
      outb(remain_arr[i],(unsigned int)iom_fpga_fnd_addr+i);
   manual_auto = 0;
   set_text_base();
   iom_fpga_dot_write(0);
   step_motor_driver_write(0,0);
   gpio_fnd_write(5,10);
   iom_fpga_text_lcd_write();
   iounmap(iom_fpga_buzzer_addr);
   iounmap(iom_fpga_step_motor_addr);
   iounmap(iom_demo_addr);
   unregister_chrdev(IOM_DRIVER_MAJOR, IOM_DRIVER_NAME);
   del_timer_sync(&timer_for_fnd_print);
   outb(0xF0, (unsigned int)led_data);
   iounmap(led_data);
   iounmap(led_ctrl);
   iounmap(iom_fpga_dot_addr);
   iounmap(iom_fpga_text_lcd_addr);
   iounmap(iom_fpga_led_addr);
   iounmap(iom_demo_addr_led);

}
module_init(iom_driver_init);
module_exit(iom_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
