#include <stdio.h>

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
  int i=0; j=0;

  Event Event_static[10];
  Event Event[10];

  set_event(&Event[0],1,1,1,1);
  set_event(&Event[1],1,2,1,1);
  set_event(&Event[2],1,2,1,1);
  set_event(&Event[3],1,1,1,1);
  set_event(&Event[4],1,1,1,1);
  set_event(&Event[5],1,3,1,1);
  set_event(&Event[6],1,3,1,1);
  set_event(&Event[7],1,1,1,1);
  set_event(&Event[8],1,4,1,1);
  set_event(&Event[9],1,4,1,1);
  
  for ( j = 0; j < 10 ; j++) {
    if ( Event_static[i].category == 0){
      Event_static[i] += Event[j];
    } else {

    }
  }
}
