# ifndef _AMOCOMMAND_H_
# define _AMOCOMMAND_H_

# include <string>

# include "./Action.h"

namespace AMOCommand {
using namespace std;

class AMOCommand {
  public:
    AMOCommand() {}
    AMOCommand(const int &seqNum, const int &key, const string &value, const Action::Action &action);

    bool operator< (const AMOCommand &r) const;

    int getSeqNum() const;
    int getKey() const;
    string getValue() const;
    Action::Action getAction() const;

  private:
    int seqNum_;
    int key_;
    string value_;
    Action::Action action_;
};
} //namespace AMOCommand

# endif // _AMOCOMMAND_H_
