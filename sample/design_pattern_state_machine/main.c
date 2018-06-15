#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "timetestlog.h"
#include "state_machine.h"

enum state_t {
	STATE_START,
	STATE_RUN,
	STATE_END,
};

enum event_t {
	EVENT_DO,
};

typedef struct msg {
	int start;
	int run;
	int end;
	StateMachineInfo state_machine;
	void *handle;
	int endfd;
} msg_t;

static int state_start(void *arg) {
	msg_t *msg=(msg_t *)arg;
	timetestlog_store_printf(msg->handle, "enter %s\n", __func__);
	msg->start=1;
	state_machine_set_state(msg->state_machine,STATE_RUN);
	state_machine_call_event(msg->state_machine, EVENT_DO, msg, sizeof(msg_t), NULL);
}

static int state_run(void *arg) {
	msg_t *msg=(msg_t *)arg;
	timetestlog_store_printf(msg->handle, "enter %s\n", __func__);
	msg->run=1;
	state_machine_set_state(msg->state_machine,STATE_END);
	state_machine_call_event(msg->state_machine, EVENT_DO, msg, sizeof(msg_t), NULL);
}

static int state_end(void *arg) {
	msg_t *msg=(msg_t *)arg;
	timetestlog_store_printf(msg->handle, "enter %s\n", __func__);
	write(msg->endfd, msg, sizeof(*msg));
}

int main(int argc, char *argv[]) {

	/*write id, len*/
	int sockpair[2];
	socketpair(AF_UNIX, SOCK_DGRAM, 0, sockpair);
	msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.handle = timetestlog_init(",", 100, 100);
	msg.endfd = sockpair[0];
	EventTPoolManager etpool = event_tpool_manager_new(1, 0);
	state_info_t state_info[]={
		STATE_MNG_SET_INFO_INIT(STATE_START, state_start),
		STATE_MNG_SET_INFO_INIT(STATE_RUN, state_run),
		STATE_MNG_SET_INFO_INIT(STATE_END, state_end),
	};
	state_event_info_t event_info={EVENT_DO, sizeof(state_info)/sizeof(state_info[0]), state_info};
	msg.state_machine = state_machine_new(1, &event_info, etpool);

	state_machine_set_state(msg.state_machine,STATE_START);
	timetestlog_store_printf(msg.handle, "Start\n");
	state_machine_call_event(msg.state_machine, EVENT_DO, &msg, sizeof(msg), NULL);
	msg_t msg2;
	read(sockpair[1], &msg2, sizeof(msg2));
	timetestlog_store_printf(msg2.handle, "End\n");
	state_machine_free(msg2.state_machine);
	event_tpool_manager_free(etpool);
	close(sockpair[0]);
	close(sockpair[1]);
	//exit
	timetestlog_exit(msg2.handle);
	return 0;
}
