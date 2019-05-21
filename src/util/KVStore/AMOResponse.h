#ifndef _AMORESPONSE_H_
#define _AMORESPONSE_H_

#include <string>

#include "./AMOCommand.h"

namespace AMOResponse {
using namespace std;

class AMOResponse {
  public:
    AMOResponse();
    AMOResponse(const AMOCommand::AMOCommand &command, const string &value);

    bool operator< (const AMOResponse &r) const;

    AMOCommand::AMOCommand GetCommand() const;
    string GetValue() const;

  private:
    AMOCommand::AMOCommand command_;
    string value_;   
};
} //namespace AMOResponse

#endif // _AMORESPONSE_H_
