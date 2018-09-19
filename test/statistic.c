#include <stdio.h>
#include <string.h>

typedef struct  Event{
  int category;
  int code;
  int pkt;
  int byte;
}Event;

void set_event(Event *ev, int cat, int cod, int pkt, int by) {
  ev->category = cat;
  ev->code = cod;
  ev->pkt = pkt;
  ev->byte = by;
}

int main() {
  int i=0, j=0;

  Event Event_static[10];
  Event Event[10];

  set_event(&Event[0],1,1,1,1);
  set_event(&Event[1],1,5,1,1);
  set_event(&Event[2],1,2,1,1);
  set_event(&Event[3],1,1,1,1);
  set_event(&Event[4],3,1,1,1);
  set_event(&Event[5],1,3,1,1);
  set_event(&Event[6],1,3,1,1);
  set_event(&Event[7],1,1,1,1);
  set_event(&Event[8],1,4,1,1);
  set_event(&Event[9],1,4,1,1);
 
 	memset(Event_static, 0 , sizeof(Event_static));	

	for (i = 0; i < 10; i++) {
		for ( j = 0; j < 10 ; j++) {
			if ((Event_static[j].category == 0 && Event_static[j].code == 0) ||
					(Event_static[j].category == Event[i].category && Event_static[j].code == Event[i].code))
					{
				Event_static[j].category = Event[i].category;
				Event_static[j].code = Event[i].code;
				Event_static[j].pkt += Event[i].pkt;
				Event_static[j].byte += Event[i].byte;
				break;
			}
		}
  }

	for (i = 0; i < 10; i++){
		if(Event_static[i].category == 0){
			break;
		} else {
			printf("%d. category[%d],code[%d], pkt[%d],byte[%d]\n",i, Event_static[i].category,Event_static[i].code, Event_static[i].pkt, Event_static[i].byte);
		}
	}
}
