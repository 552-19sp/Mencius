# include <map>
# include <string>
# include <stdlib.h>
# include <iostream>

using namespace std;

template <class S, class T>
class KVStore {
  private:
    map<S, T> _store;


  public:
    void insert(S key, T value) {
      if (_store.find(key) == _store.end()) {
	_store[key] = value;
      } else {
        _store[key] = value + _store[key];  
      }
    }

    T get(S key) {
      return _store[key];
    }
};

// Currently used only to add values
class AMOCommand {
  private:
    int _seqNum;
    int _key;
    int _value;
    

  public:
    AMOCommand() {}

    AMOCommand(int seqNum, int key, int value) {
      _seqNum = seqNum;
      _key = key;
      _value = value;
    }

    int getSeqNum() {
      return _seqNum;
    }

    int getKey() {
      return _key;
    }

    int getValue() {
      return _value;
    }
};

template <class T>
class AMOResponse {
  private:
    AMOCommand _command;
    T _value;
    
      
  public:
    AMOResponse() {}

    AMOResponse(AMOCommand command, T value) {
      _command = command;
      _value = value;
    }

    AMOCommand getCommand() {
      return _command;
    }

    T getValue() {
      return _value;
    }
};

template <class S, class T>
class AMOStore {
  private:
    KVStore<S, T> kvStore;
    map<AMOCommand, AMOResponse<T>> prev;

  public:
    AMOStore() {
      kvStore = KVStore<S, T>();
    }
	  
    AMOResponse<T> execute(AMOCommand command) {
      if (!alreadyExecuted(command)) {
        kvStore.insert(command.getKey(), command.getValue());
	prev[command] = AMOResponse<T> (command, command.getValue());
      }

      return prev[command];
    }

    bool alreadyExecuted(AMOCommand command) {
      return prev.find(command) == prev.end();
    }
};

int main() {
  AMOStore<int, int> app = AMOStore<int, int>();

  AMOCommand c1 = AMOCommand(0, 10, 1);
  AMOCommand c2 = AMOCommand(0, 20, 10);
  AMOCommand c3 = AMOCommand(0, 10, 100);
  AMOCommand c4 = AMOCommand(0, 20, 1000);
  AMOCommand c5 = AMOCommand(0, 30, 10000);
  
  return 0;
}
