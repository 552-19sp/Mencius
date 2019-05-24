// Copyright 2019 Jack Khuu, Justin Johnson, Lukas Joswiak

#include <iostream>

#include "AMOStore.hpp"
#include "AMOCommand.hpp"
#include "Action.hpp"

using std::cout;
using std::endl;

int main() {
  KVStore::AMOStore app = KVStore::AMOStore();

  KVStore::AMOCommand c1 = KVStore::AMOCommand(0, "10", "A", KVStore::PUT);
  app.Execute(c1);
  cout << "Done" << endl;
  return 0;
}
