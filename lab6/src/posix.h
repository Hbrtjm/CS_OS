#define SERVER_QUEUE	"/server_queue"
#define MAX_CLIENTS	10
#define MSG_SIZE	1024
#define MAX_MSG		10

struct client {
	int	active;
	mqd_t	mqd;
	char	name[64];
};

struct mq_attr attr = { 
	.mq_flags   = 0,
	.mq_maxmsg  = MAX_MSG, // Haven't tested it yet, I might run a bash script and test it
	.mq_msgsize = MSG_SIZE,
	.mq_curmsgs = 0 
};
