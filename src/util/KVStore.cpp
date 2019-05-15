# include <map>
# include <string>
# include <stdlib.h>
# include <iostream>

using namespace std;

enum Action { PUT, APPEND, GET };

// Supports an integer Key and a String value
class KVStore {
  private:
    map<int, string> _store;


  public:
    string insert(int key, string value) {
      if (_store.find(key) != _store.end()) {
        exit(EXIT_FAILURE);
      }
      _store[key] = value;
      
      return _store[key];
    }

    string append(int key, string value) {
      if (_store.find(key) == _store.end()) {
        exit(EXIT_FAILURE);
      }
      _store[key] = _store[key] + value;  
      return _store[key];
    }

    string get(int key) {
      return _store[key];
    }
};


class AMOCommand {
  private:
    int _seqNum;
    int _key;
    string _value;
    Action _action;
    

  public:
    AMOCommand() {}

    AMOCommand(int seqNum, int key, string value, Action action) {
      _seqNum = seqNum;
      _key = key;
      _value = value;
      _action = action;
    }

    bool operator< (const AMOCommand& r) const {
      return this->_seqNum < r._seqNum;
    }

    int getSeqNum() {
      return _seqNum;
    }

    int getKey() {
      return _key;
    }

    string getValue() {
      return _value;
    }

    Action getAction() {
      return _action;
    }
};


class AMOResponse {
  private:
    AMOCommand _command;
    string _value; 
      
  public:
    AMOResponse() {}

    AMOResponse(AMOCommand command, string value) {
      _command = command;
      _value = value; 
    }

    bool operator< (const AMOResponse& r) const {
      return _command < r._command;
    }

    AMOCommand getCommand() {
      return _command;
    }

    string getValue() {
      return _value;
    }
};


class AMOStore {
  private:
    KVStore kvStore;
    map<AMOCommand, AMOResponse> prev;

  public:
    AMOStore() {
      kvStore = KVStore();
    }
	  
    AMOResponse execute(AMOCommand command) {
      string s;
      if (!alreadyExecuted(command)) {
	cout << "New" << endl;
	switch(command.getAction()) {
	  case PUT:
	    cout << "PUT" << endl;
            s = kvStore.insert(command.getKey(), command.getValue());
	    break;
	  case APPEND:
	    cout << "APPEND" << endl;
            s = kvStore.append(command.getKey(), command.getValue());
	    break;
	  case GET:
	    cout << "GET" << endl;
            s = kvStore.get(command.getKey());
	    break;
	  default:  
	    exit(EXIT_FAILURE);
	}

	prev[command] = AMOResponse(command, s);
      }
      cout << "``````````Returned from Execute````````````````" << endl;
      cout << prev[command].getValue() << endl;
      cout << "```````````````````````````````````````````````" << endl;
      return prev[command];
    }

    bool alreadyExecuted(AMOCommand command) {
      return prev.find(command) != prev.end();
    }
};


int main() {
  AMOStore app = AMOStore();

  AMOCommand c1 = AMOCommand(0, 10, "1", PUT);
  AMOCommand c1g = AMOCommand(1, 10, "1", GET);
  AMOCommand c2 = AMOCommand(2, 20, "10", PUT);
  AMOCommand c1a = AMOCommand(3, 10, "100", APPEND);
  AMOCommand c1a2 = AMOCommand(4, 10, "100", APPEND);
  AMOCommand c1g2 = AMOCommand(5, 10, "100", GET);
  AMOCommand c2a = AMOCommand(6, 20, "1000", APPEND);
  AMOCommand c3 = AMOCommand(6, 30, "10000", PUT);

  app.execute(c1);
  app.execute(c1g);
  app.execute(c2);
  app.execute(c1a);
  app.execute(c1a2);
  app.execute(c1g2);
  app.execute(c2a);
  app.execute(c3);
  
  return 0;
}
