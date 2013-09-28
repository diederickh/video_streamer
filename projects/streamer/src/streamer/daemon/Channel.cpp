#include <iostream>
#include <iterator>
#include <algorithm>
#include <streamer/daemon/Channel.h>

// ----------------------------------------------

void channel_thread(void* user) {
  Channel* c = static_cast<Channel*>(user);
  uint32_t stream_id = c->stream_id;
  uint32_t stream_id_be = ToBE32(stream_id);
  uint32_t size_be = 0;
  uv_mutex_t& mutex = c->mutex;
  uv_cond_t& cv = c->cv;

  std::vector<ChannelPacket*> todo;
  int rc = 0;

  while(true) {

    // get some work to process 
    uv_mutex_lock(&mutex);
    {
      while(c->work.size() == 0) {
        uv_cond_wait(&cv, &mutex);
      }
      std::copy(c->work.begin(), c->work.end(), std::back_inserter(todo));
      c->work.clear();
    }
    uv_mutex_unlock(&mutex);

    //notify our the daemon
    for(std::vector<ChannelPacket*>::iterator it = todo.begin(); it != todo.end(); ++it) {
      ChannelPacket& p = **it;
      c->send(p.bs.getPtr(), p.bs.size());

      // @todo - nanomsg sends/receives full packets so we don't need to handle it ourself (remove code below!)
      /*
      switch(p.type) {
        case CP_TYPE_AVPACKET: {
          size_be = (uint32_t)p.av_packet->data.size();
          size_be = ToBE32(size_be);

          c->send((uint8_t*)&stream_id_be, sizeof(stream_id_be));
          c->send((uint8_t*)&p.type, 1);
          c->send((uint8_t*)&size_be, sizeof(size_be));
          c->send((uint8_t*)&p.av_packet->data.front(), p.av_packet->data.size());

          delete p.av_packet;
          p.av_packet = NULL;

          break;
        }
      
        }
      */
      delete *it;
    }
    
    todo.clear();
  }
}

int Channel::send(uint8_t* data, size_t nbytes) {
  int rc = nn_send(sock, (char*)data, nbytes, 0);
  if(rc < 0) {
    printf("error: couldn't send: %s\n", nn_strerror(errno));
    return 0;
  }
  return rc;
}

// ----------------------------------------------

ChannelPacket::ChannelPacket() {
}

// ----------------------------------------------

Channel::Channel() 
  :stream_id(0)
  ,thread(NULL)
  ,sock(0)
{
  uv_mutex_init(&mutex);
  uv_cond_init(&cv);
}

Channel::~Channel() {
  printf("error: need to check if the thread is still running and if so, make sure it stops.\n");
}

bool Channel::setup(uint32_t id, std::string address) {

  if(!address.size()) {
    printf("error: invalid address: %s\n", address.c_str());
    return false;
  }

  stream_id = id;
  stream_address = address;

  sock = nn_socket(AF_SP, NN_PUSH);
  if(sock == -1) {
    printf("error: cannot create channel sock: %s\n", nn_strerror(errno));
    return false;
  }

  int rc = 0;
  rc = nn_connect(sock, stream_address.c_str());
  if(rc < 0) {
    printf("error: cannot connect the sock: %s\n", nn_strerror(errno));
    return false;
  }
  
  return true;
}

bool Channel::initialize() {

  if(!stream_address.size()) {
    printf("error: no address set.\n");
    return false;
  }

  uv_thread_create(&thread, channel_thread, this);

  return true;
}

void Channel::addPacket(ChannelPacket* p) {
  uv_mutex_lock(&mutex);
  {
    work.push_back(p);
  }  
  uv_cond_signal(&cv);
  uv_mutex_unlock(&mutex);  
}


