#include "cycle.h"

using namespace us::gov::engine;
using namespace std;

typedef us::gov::engine::cycle_t c;

void c::wait_for_stage(stage ts) {
	using namespace chrono;
		//this_thread::sleep_until();
	time_point now=steady_clock::now();
	duration tp = now.time_since_epoch();
	minutes m = duration_cast<minutes>(tp);
	tp-=m; 
	seconds s = duration_cast<seconds>(tp); //seconds in this minute
//cout << dec << s.count() << " <? " << ts << endl;
	int n=ts; //stage boundaries in seconds within this minute
	while (s.count()>n) n+=60;
cout << "Cycle: current second is " << s.count() << ". I'll sleep for " << (n-s.count()) << endl;
	thread_::_this.sleep_for(seconds(n-s.count()));
cout << "engine: starting stage: " << str(ts) << endl;
}

string c::str(stage s) const {
	switch(s) {
		case new_cycle: return "new_cycle"; break;
		case local_deltas_io: return "local_deltas_io"; break;
		case sync_db: return "sync_db"; break;
		case consensus_vote_tip_io: return "consensus_vote_tip_io"; break;
	}
	return "?";
}
c::stage c::get_stage() {
	using namespace chrono;
	time_point now=steady_clock::now();
	duration tp = now.time_since_epoch();
	minutes m = duration_cast<minutes>(tp);
	tp-=m;
	seconds s = duration_cast<seconds>(tp);
	int sec=s.count();
	if (sec<local_deltas_io) return new_cycle;
	if (sec<consensus_vote_tip_io) return local_deltas_io;
	if (sec<sync_db) return sync_db;
	return consensus_vote_tip_io;
}


