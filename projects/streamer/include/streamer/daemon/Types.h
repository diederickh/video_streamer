/*

---------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl  
                                          www.twitter.com/roxlu
              
---------------------------------------------------------------------------------

*/

#ifndef ROXLU_DAEMON_H
#define ROXLU_DAEMON_H

/* Channel commands - are sent by client to the daemon */

#define CP_TYPE_NONE 0 /* default, unset */
#define CP_TYPE_AVPACKET 1 /* channel data contains a AVPacket, we will delete the packet when used */
#define CP_TYPE_STOP 2  /* stop command; */

#endif
