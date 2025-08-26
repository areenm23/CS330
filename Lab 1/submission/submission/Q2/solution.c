#include <stdio.h>
#include <stdlib.h>
#include "buffer_api.h"




int main(int argc, char* argv[]) {
	if(argc != 2){
		perror("usage: ./solution <testcase number>");
		return -1;
	}
	buffer_init(atoi(argv[1]));
	// int test = atoi(argv[1]);
	const struct ring_buffer *rb = buffer_get_base();

	/* ------ YOUR CODE ENDS HERE ------*/

uint64_t pos = rb->data_tail;
uint64_t end = rb->data_head;
uint64_t mask = rb->data_size - 1;
uint64_t total = 0, unknown = 0;

 while(pos < end) {


struct perf_event_header* hdr = (struct perf_event_header*)((char*) rb->data_base + (pos&mask));
if(hdr->type == PERF_RECORD_SAMPLE) {

struct sample_event *ev = (struct sample_event*) hdr;
printf("0x%lx\n", ev->addr);


}
else if(hdr->type == PERF_RECORD_LOST) {
struct lost_event *ev = (struct lost_event*) hdr;
total += (ev->lost);


}
else {

	unknown += hdr->size;

}

pos += hdr->size;


 }


	/* print formats */	// printf("0x%...."); // to print sample addresses
	 printf("number of lost records: %lu\n", total);
	 printf("unknown size: %lu\n", unknown);

	/* ------ YOUR CODE ENDS HERE ------*/
	buffer_exit();
	return 0;
}

