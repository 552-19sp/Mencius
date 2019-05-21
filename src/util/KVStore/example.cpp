#include <iostream>

#include "./AMOStore.h"
#include "./AMOCommand.h"
#include "./Action.h"

using namespace Action;
using namespace std;

int main() {
  AMOStore::AMOStore app = AMOStore::AMOStore();

  AMOCommand::AMOCommand c1 = AMOCommand::AMOCommand(0, 10, "A", PUT);
  app.Execute(c1);
  cout << "Done" << endl;
  return 0;
}
