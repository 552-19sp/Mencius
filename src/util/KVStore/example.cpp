// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include <iostream>

#include "AMOStore.hpp"
#include "AMOCommand.hpp"
#include "Action.hpp"

using std::cout;
using std::endl;

int main() {
  AMOStore::AMOStore app = AMOStore::AMOStore();

  AMOCommand::AMOCommand c1 = AMOCommand::AMOCommand(0, 10, "A", Action::PUT);
  app.Execute(c1);
  cout << "Done" << endl;
  return 0;
}
