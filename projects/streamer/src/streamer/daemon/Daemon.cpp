#include <streamer/daemon/Daemon.h>

Daemon::Daemon() {
}

Daemon::~Daemon() {

  for(runner_iterator it = runners.begin(); it != runners.end(); ++it) {
    delete it->second;
  }
  runners.clear();

}

bool Daemon::setup(std::string configpath) {

  if(!config.load(configpath)) {
    return false;
  }

  for(DaemonConfig::iterator it = config.begin(); it != config.end(); ++it) {
    Runner* r = new Runner(it->second);
    if(!r->setup()) {
      return false;
    }
    runners.insert(std::pair<uint32_t, Runner*>(it->first, r));
  }

  return true;
}

bool Daemon::initialize() {

  if(!runners.size()) {
    printf("error: no runners found; did you add some entries to your config?.\n");
    return false;
  }

  for(runner_iterator it = runners.begin(); it != runners.end(); ++it) {
    if(!it->second->initialize()) {
      return false;
    }
  }

  return true;
}
