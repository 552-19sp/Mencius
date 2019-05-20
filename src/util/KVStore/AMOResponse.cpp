# include "AMOResponse.h"

namespace AMOResponse {
using namespace std;

AMOResponse::AMOResponse() {}

AMOResponse::AMOResponse(const AMOCommand::AMOCommand &command, const string &value) {
  command_ = command;
  value_ = value; 
}

bool AMOResponse::operator< (const AMOResponse &r) const {
  return command_ < r.command_;
}

AMOCommand::AMOCommand AMOResponse::getCommand() const {
  return command_;
}

string AMOResponse::getValue() const {
  return value_;
}
} // namespace AMOResponse
