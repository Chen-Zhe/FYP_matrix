#include <iostream>
#include <mqueue.h>
#include <fcntl.h>

int main() {
	char msg[5] = "abcd";

	char recv;

	struct mq_attr rec_vad_attr;
	rec_vad_attr.mq_flags = 0;
	rec_vad_attr.mq_maxmsg = 10;
	rec_vad_attr.mq_msgsize = 1;
	rec_vad_attr.mq_curmsgs = 0;
	mqd_t toVad = mq_open("/test", O_CREAT | O_RDWR, 0644, &rec_vad_attr);

	mq_send(toVad, msg+2, 1, 0);

	mq_receive(toVad, &recv, 1, NULL);

	std::cout << recv << std::endl;
	
}