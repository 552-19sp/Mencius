#ifndef _AMOSTORE_H_
#define _AMOSTORE_H_

#include <map>
#include <string>

#include "./AMOCommand.h"
#include "./AMOResponse.h"
#include "./KVStore.h"

namespace AMOStore {
using namespace std;

class AMOStore {
  public:
    AMOStore();
	  
    AMOResponse::AMOResponse execute(const AMOCommand::AMOCommand &command);

  private:
    KVStore::KVStore kvStore_;
    map<AMOCommand::AMOCommand, AMOResponse::AMOResponse> prev_;

    bool alreadyExecuted(const AMOCommand::AMOCommand &command) const;
};

} //namespace AMOStore

#endif // _AMOSTORE_H_
