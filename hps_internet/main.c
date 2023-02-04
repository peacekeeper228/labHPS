#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "linux/input.h"
#include <time.h>
#include <stdlib.h>

int  print_current_date(int switch_print) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int res = 0;
    switch (switch_print) {
        case 0:
            res = t->tm_mday;
            break;
        case 1:
            res = t->tm_mon + 1;
            break;
        case 2:
            res = t->tm_year - 120;
            break;
    }
    return res; 
}

int main(void) {
	int event_dev_fd;
	const char *path = "/dev/input/by-path/platform-soc:base-fpga-region:keys-event";
	struct input_event the_event;
	int result;
	int number_of_counts;
	char path_LED[PATH_MAX];

	int i;
	int led_file;

	const char *leds_array[] = {
		"fpga_led0",
		"fpga_led1",
		"fpga_led2",
		"fpga_led3",
		"fpga_led4",
		"fpga_led5",
		"fpga_led6",
		"fpga_led7"
	};
	
	// open the event device node
	event_dev_fd = open(path, O_RDONLY | O_SYNC);
	if(event_dev_fd < 0)
		error(1, errno, "could not open file '%s'", path);
	
	while(1) {
		// read the next event
		result = read(event_dev_fd, &the_event, 
				sizeof(struct input_event));
		if(result < 0)
			error(1, errno, "reading %d from '%s'", 
					sizeof(struct input_event), path);
		if(result != sizeof(struct input_event))
			error(1, 0, "did not read %d bytes from '%s'", 
					sizeof(struct input_event), path);
		
		if(the_event.type == EV_KEY) {
			number_of_counts = 0;
			switch(the_event.code) {
			case(KEY_F6) :
				if (the_event.value){
					number_of_counts = print_current_date(0);
				}
				break;
			case(KEY_F7) :
				if (the_event.value){
					number_of_counts = print_current_date(1);
				}
				break;
			case(KEY_F8) :
				if (the_event.value){
					number_of_counts = print_current_date(2);
				}
				break;
			case(KEY_F9) :
				if (the_event.value){
					system("sh /home/root/getTime.sh");
				}
				break;
			default :
				break;
			}
			if (number_of_counts>7){
				printf("cannot display the number %d on leds \n", number_of_counts);
			} else {
				for (i = 0 ; i < 8 ; i++ ) {
					// to change the state of led it is needed to be 
					// written in the file named brightness a value 1 or 0
					snprintf(path_LED, PATH_MAX,
						"/sys/class/leds/%s/brightness", leds_array[i]);
					led_file = open(path_LED, O_RDWR | O_SYNC);
					if (i<number_of_counts)
						result = write(led_file, "1", 1);
					else 
						result = write(led_file, "0", 1);
					result = close(led_file);
					result = usleep(125000);
				}
			}
		}
	}
}

