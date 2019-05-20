# include <map>
# include <string>
# include <stdlib.h>
# include <iostream>

using namespace std;

enum Action { PUT, APPEND, GET };

// Supports an integer Key and a String value
class KVStore {
  private:
    map<int, string> store_;


  public:
    string insert(const int &key, const string &value) {
      if (store_.find(key) != store_.end()) {
        exit(EXIT_FAILURE);
      }
      store_[key] = value;
      
      return store_[key];
    }

    string append(const int &key, const string &value) {
      if (store_.find(key) == store_.end()) {
        exit(EXIT_FAILURE);
      }
      store_[key] = store_[key] + value;  
      return store_[key];
    }

    string get(const int &key) {
      return store_[key];
    }
};


class AMOCommand {
  private:
    int seqNum_;
    int key_;
    string value_;
    Action action_;
    

  public:
    AMOCommand() {}

    AMOCommand(const int &seqNum, const int &key, const string &value, const Action &action) {
      seqNum_ = seqNum;
      key_ = key;
      value_ = value;
      action_ = action;
    }

    bool operator< (const AMOCommand &r) const {
      return this->seqNum_ < r.seqNum_;
    }

    int getSeqNum() const {
      return seqNum_;
    }

    int getKey() const {
      return key_;
    }

    string getValue() const {
      return value_;
    }

    Action getAction() const {
      return action_;
    }
};


class AMOResponse {
  private:
    AMOCommand command_;
    string value_; 
      
  public:
    AMOResponse() {}

    AMOResponse(const AMOCommand &command, const string &value) {
      command_ = command;
      value_ = value; 
    }

    bool operator< (const AMOResponse &r) const {
      return command_ < r.command_;
    }

    AMOCommand getCommand() const {
      return command_;
    }

    string getValue() const {
      return value_;
    }
};


class AMOStore {
  private:
    KVStore kvStore_;
    map<AMOCommand, AMOResponse> prev_;

    bool alreadyExecuted(const AMOCommand &command) const {
      return prev_.find(command) != prev_.end();
    }

  public:
    AMOStore() {
      kvStore_ = KVStore();
    }
	  
    AMOResponse execute(const AMOCommand &command) {
      string s;
      if (!alreadyExecuted(command)) {
	cout << "New" << endl;
	switch(command.getAction()) {
	  case PUT:
	    cout << "PUT" << endl;
            s = kvStore_.insert(command.getKey(), command.getValue());
	    break;
	  case APPEND:
	    cout << "APPEND" << endl;
            s = kvStore_.append(command.getKey(), command.getValue());
	    break;
	  case GET:
	    cout << "GET" << endl;
            s = kvStore_.get(command.getKey());
	    break;
	  default:  
	    exit(EXIT_FAILURE);
	}

	prev_[command] = AMOResponse(command, s);
      }
      cout << "Returned from Execute: ";
      cout << prev_[command].getValue() << endl;
      return prev_[command];
    }
};


int main() {
  AMOStore app = AMOStore();

  AMOCommand c1 = AMOCommand(0, 10, "A", PUT);
  AMOCommand c1g = AMOCommand(1, 10, "---", GET);
  AMOCommand c2 = AMOCommand(2, 20, "B", PUT);
  AMOCommand c1a = AMOCommand(3, 10, "A2", APPEND);
  AMOCommand c1a2 = AMOCommand(4, 10, "A3", APPEND);
  AMOCommand c1g2 = AMOCommand(5, 10, "---", GET);
  AMOCommand c2a = AMOCommand(6, 20, "B2", APPEND);
  AMOCommand c3 = AMOCommand(7, 30, "C", PUT);
  AMOCommand c2g = AMOCommand(8, 20, "---", GET);
  AMOCommand c3g = AMOCommand(9, 30, "---", GET);

  /*
  // Put Test
  cout << "Put Test" << endl << endl;
  app.execute(c1);
  app.execute(c2);
  app.execute(c3);
  app.execute(c1g);
  app.execute(c2g);
  app.execute(c3g);
  */

  // Append Test
  cout << "Append Test" << endl << endl;
  app.execute(c1);
  app.execute(c1g);
  app.execute(c1a);
  app.execute(c1g);
  app.execute(c1g2);
  

  return 0;
}
