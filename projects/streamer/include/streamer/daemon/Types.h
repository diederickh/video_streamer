#ifndef ROXLU_DAEMON_H
#define ROXLU_DAEMON_H

/* Channel commands - are sent by client to the daemon */

#define CP_TYPE_NONE 0 /* default, unset */
#define CP_TYPE_AVPACKET 1 /* channel data contains a AVPacket, we will delete the packet when used */
#define CP_TYPE_STOP 2  /* stop command; */

#endif
