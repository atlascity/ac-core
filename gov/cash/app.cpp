#include "app.h"
#include "protocol.h"
#include <thread>
#include <chrono>
#include <us/gov/crypto/base58.h>
#include <us/gov/likely.h>
#include "tx.h"

typedef us::gov::cash::app c;
using namespace us::gov;
using namespace us::gov::cash;
using namespace std;

constexpr const char* c::name;
constexpr array<const char*,app::policies_traits::num_params> app::policies_traits::paramstr;

namespace std {

  template <>
  struct hash<us::gov::cash::app::local_delta> {
	size_t operator()(const us::gov::cash::app::local_delta&) const;
  };

}

size_t std::hash<us::gov::cash::app::local_delta>::operator() (const us::gov::cash::app::local_delta&g) const {
	return *reinterpret_cast<const size_t*>(&g.get_hash()[0]); //LE
}

void c::local_delta::accounts_t::add_input(tx& t, const hash_t& addr, const cash_t& amount) {
//	box_t prev_balance=0; //TODO
	t.add_input(addr/*,prev_balance*/, amount);
}
void c::local_delta::accounts_t::add_output(tx& t, const hash_t& addr, const box_t& amount, const hash_t& locking_program) {
	t.add_output(addr,amount,locking_program);
}

int c::local_delta::app_id() const {
	return app::id();
}

c::app() {
	pool=new c::local_delta();
	policies_local=policies;
}

c::~app() {
	delete pool;
}

/*
		//relay rules: **might be outdated**  TODO transfer to doc
		//objective: 1 transaction must appear in only a few guts in the block, if every tx appears in every gut of each miner the block would have size O(N*T) where N is number of miners and T number of incoming transactions for this height.
		//		we want block size O(N), each miner shall provide only a small % of transactions.
		//For doing so, propagation of each tx shall be limited.
		//The following rule encourages a miner to both relay the tx and stop relaying it after reached a maximum number of hops is reached.
		//merchants start a transaction using only 3 random nodes, no more, no less, otherwise the tx might not progress to the blockchain.
		//at this point the tx enters the ecosystem, 3 different propagation threads has been initiated, nodes will assign hop number 0.
		//propagation threads consist on 4 hops.
		//according to the rule a total of 12 nodes see the same tx.
		//block validation will check how many guts from different miners contain the same transaction
		//the tx will be discarded if this number is below 6 (some miner refused to relay, trying to maximize chances of getting its fee)
		//the tx will be discarded if this number is above 12 (some miner relayed in excess, increasing system congestion)
		//Particularity: on reception the node will check if it has seen the tx before.
		//if seen before, it relays it without incrementing the hop number, so every propagation thread can provide 4 different guts containing the tx.
		//in total a maximum of 12 guts is expected when making the closure block
		//this miner has 1/16 chances of getting paid the fee for this tx.
		bool first_time=buffer.add(t);
		uint8_t hop=tx::get_hop(*d);
*/

void c::add_policies() {
	lock_guard<mutex> lock(mx_pool);
	lock_guard<mutex> lock2(mx_policies);
	for (int i=0; i<policies_traits::num_params; ++i)
		(*pool)[i] = policies_local[i];
}

engine::app::local_delta* c::create_local_delta() {
	add_policies();
	lock_guard<mutex> lock(mx_pool);
	auto full=pool;
	pool=new c::local_delta();
	return full;
}

bool c::process_query(peer_t *c, datagram*d) {
	switch(d->service) {
		case protocol::cash_query: {
			cash_query(c,d);
			return true;
		} break;
	}
	return false;
}

bool c::basic_check(const tx& t) const {

    cash_t min_fee;
    {
    lock_guard<mutex> lock(mx_policies);
    min_fee=policies[policies_traits::minimum_fee];
    }

    if (min_fee<0) {
        cerr << "TX REJECTED 2" << endl;
        return false;
        }

    auto fee=t.check();

    //min_fee=1; //TODO remove

    if (fee<min_fee) {
        cerr << "TX 3 " << fee << " " << min_fee << endl;
        cout << "RBF removed min fee check" << endl;
        return false;
    }

    return true;
}

engine::evidence* c::parse_evidence(uint16_t service, const string& datagram_payload) const {
	switch(service) {
		case protocol::cash_tx: {
			//string payload=d.parse_string();
			auto t=tx::from_b58(datagram_payload);
//            cout << "calendar producer - size: " << t.second->inputs.size() << endl;
            if (unlikely(!t.first.empty())) {
               return 0;
            }
            if(!basic_check(*t.second))
                return 0;
//      cout << "cash: scheduling evidence " << t.second->ts << " " << t.second->inputs.size() << endl;
           return t.second.release();
//      cout << "cash: scheduling evidence " << ptx->ts << " " << ptx->inputs.size() << endl;
		} break;
	}
	return 0;
}

datagram* c::query_accounts_t::get_datagram() const {
                if (empty()) return 0;
                ostringstream os;
                os << size() << ' ';
                for (auto&i:*this) {
                    os << i << ' ';
                }
                return new socket::datagram(protocol::cash_query,os.str());
}

c::query_accounts_t c::query_accounts_t::from_string(const string&s) {
	int n;
	istringstream is(s);
	is >> n;
	query_accounts_t r;
	r.reserve(n);
	for (int i=0; i<n; ++i) {
		hash_t v;
		is >> v;
		r.emplace_back(move(v));
	}
	return move(r);
}

c::query_accounts_t c::query_accounts_t::from_datagram(datagram*d) {
	string query=d->parse_string();
	delete d;
	return from_string(query);
}
#include <us/gov/engine/daemon.h>

void c::cash_query(peer_t *c, datagram*d) {
//cout << "CASH_QUERY" << endl;
	query_accounts_t q=query_accounts_t::from_datagram(d);

	ostringstream os;
	os << "0 "; //ret code ok
	{
	lock_guard<mutex> lock(db.mx);
	for (auto&addr:q) {
		auto a=db.accounts->find(addr);
		if (likely(a!=db.accounts->end())) {
			a->second.to_stream(os);
		}
		else {
			account_t a;
			a.locking_program=0;
//			a.balance=0;
			a.to_stream(os);
		}
	}
	}

	os << ' ' << chaininfo.get_tip(); //last db version
//cout << "CASH_RESPONSE " << os.str() << endl;
	c->send(new datagram(protocol::cash_response,os.str()));
}

void c::dbg_output() const {
	while(!program::_this.terminated) {
		db.dump(cout);
		thread_::_this.sleep_for(chrono::seconds(15));
	}
}


void c::run() {
    //thread tdbg(&c::dbg_output,this);

    dbg_output();
}

void c::local_delta::account_t::dump(ostream& os) const {
    os << "locking_program " << locking_program << "; balance " << box;
}

void c::local_delta::accounts_t::dump(ostream& os) const {
	cout << size() << " p2sh accounts:" << endl;
	for (auto& i:*this) {
		cout << ' ' << i.first << ' ';
		i.second.dump(os);
		os << endl;
	}
}

//struct tokens_t:unordered_map<token_t,cash_t> {
void c::local_delta::tokens_t::dump(ostream& os) const {
}

void c::local_delta::tokens_t::to_stream(ostream& os) const {
}

c::local_delta::tokens_t c::local_delta::tokens_t::from_stream(istream& is) {
    return tokens_t();
}

void c::local_delta::safe_deposit_box::dump(ostream& os) const {
}

void c::local_delta::safe_deposit_box::to_stream(ostream& os) const {
}

c::local_delta::safe_deposit_box c::local_delta::safe_deposit_box::from_stream(istream& is) {
    return safe_deposit_box();
}

void c::db_t::dump(ostream& os) const {
	cout << "cash app db dump" << endl;
	lock_guard<mutex> lock(mx);
	accounts->dump(os);
	cout << "supply_left " << supply_left << " block_reward " << block_reward << endl;
}

void c::db_t::to_stream(ostream&os) const {
	accounts->to_stream(os);
	os << ' ';
	os << supply_left << ' ' << block_reward;
}

c::db_t c::db_t::from_stream(istream& is) {
	db_t db;
	db.accounts=accounts_t::from_stream(is);
	is >> db.supply_left;
	is >> db.block_reward;
	return move(db);
}

bool c::local_delta::accounts_t::pay(const hash_t& address, const box_t& amount) { //caller has to get the lock
	if (amount<=0) return false;
	auto i=find(address);
	if (i==end()) {
		emplace(make_pair(address,account_t{1,amount}));
	}
	else {
		i->second.box+=amount;
	}
	return true;
}

bool c::local_delta::accounts_t::withdraw(const hash_t& address, const box_t& amount) { //caller has to get the lock
	auto i=find(address);
	if (i==end()) {
		return false;
	}
	else if (i->second.box<amount) {
		return false;
	}

	//locate the output;
	i->second.box-=amount;
	if (unlikely(i->second.box==0)) {
		erase(i);
	}
	return true;
}

bool c::db_t::add_(const hash_t& k, const box_t& amount) { //caller has to get the lock
	return accounts->pay(k,amount);
}

bool c::db_t::withdraw_(const hash_t& k, const box_t& amount) { //caller has to get the lock
	return accounts->withdraw(k,amount);
}

cash_t c::db_t::get_newcash() { //db lock must be acquired
	if (likely(supply_left>=block_reward)) {
		supply_left-=block_reward;
	}
	else {
		block_reward=supply_left;
		supply_left=0;
	}
	return block_reward;
}

void c::import(const engine::app::delta& gg, const engine::pow_t& w) {

	const delta& g=static_cast<const delta&>(gg);
	{
	lock_guard<mutex> lock(mx_policies);
	for (int i=0; i<policies_traits::num_params; ++i) policies[i]=g[i];
	}

	{
	const c::local_delta::accounts_t& a=g.g.accounts;
	lock_guard<mutex> lock(db.mx);
	for (auto& i:a) {
		auto d=db.accounts->find(i.first);
		if (d==db.accounts->end()) {
			if (likely(i.second.box!=0)) {
				db.accounts->emplace(i.first,account_t(i.second.locking_program,i.second.box));
			}
		}
		else {
			if (i.second.box==0) {
				db.accounts->erase(d);
			}
			else {
				d->second.locking_program=i.second.locking_program;
				d->second.box=i.second.box;
			}
		}
	}

	}

//spread fees
	if (likely(!w.empty())) {
	///spread newcash+fees weighted by work
		lock_guard<mutex> lock(db.mx);
		cash_t newcash=db.get_newcash();
		cash_t total_cash_int=newcash+g.g.fees;
		double total_cash=(double)total_cash_int;
		uint64_t total_work=w.sum();
		cash_t paid=0;
		for (auto&i:w) {
			double rel_work;
			if (likely(total_work>0)) {
				rel_work=(double)i.second/(double)total_work;
			}
			else {
				rel_work=1.0/(double)w.size();
			}
			cash_t amount=total_cash*rel_work;
			db.add_(i.first,amount);
			paid+=amount;
		}
		cash_t remainder=total_cash_int-paid;
		if (remainder>0) {
			default_random_engine generator(get_seed());
			uniform_int_distribution<size_t> distribution(0,w.size()-1);
			auto i=w.begin();
			advance(i,distribution(generator));
			db.add_(i->first,remainder);
		}
	}
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "locking_programs/p2pkh.h"

bool c::unlock(const hash_t& address, const size_t& this_index, const hash_t& locking_program, const string& locking_program_input, const tx& t) {
	if (likely(locking_program<min_locking_program)) {
		if (unlikely(locking_program==0)) {
			return true;
		}
		else if (locking_program==p2pkh::locking_program_hash) {
			return p2pkh::check_input(address, t,this_index, locking_program_input);
		}
	}

	//check https://en.wikipedia.org/wiki/AppArmor
	ostringstream prog;
	prog << "--private /home/mmayorga/.bitcoin++/locking/" << locking_program << "/main";
//	prog << "firejail --private ";
	pid_t pid = fork();
	switch (pid) {
		case -1: /* Error */
		    cerr << "Uh-Oh! fork() failed.\n";
		    exit(1);
		case 0: /* Child process */
		    cout << "firejail " << prog.str().c_str() << "\n";
		    execl("firejail",prog.str().c_str()); /* Execute the program */
		    cerr << "Uh-Oh! execl() failed!"; /* execl doesn't return unless there's an error */
		    exit(1);
		default: /* Parent process */
		    cout << "Process created with pid " << pid << "\n";
		    int status;

		    while (!WIFEXITED(status)) {
		        waitpid(pid, &status, 0); /* Wait for the process to complete */
		    }
		    std::cout << "Process exited with " << WEXITSTATUS(status) << "\n";
		    return status==0;
	}
}

bool c::account_state(const local_delta::batch_t& batch, const hash_t& address, account_t& acc) const {
		auto  b=batch.find(address);
		if (likely(b==batch.end())) { //this input has already been consumed at least once
			auto p=pool->accounts.find(address);
			if (likely(p==pool->accounts.end())) { //ref in db
				lock_guard<mutex> lock(db.mx);
				auto o=db.accounts->find(address);
				if (o==db.accounts->end()) {
					return false;
				}
				acc.locking_program=o->second.locking_program;
				acc.box=o->second.box;
			}
			else { //ref in pool
				acc.locking_program=p->second.locking_program;
				acc.box=p->second.box;
			}
		}
		else { //ref in batch
			acc.locking_program=b->second.locking_program;
			acc.box=b->second.box;
		}
		return true;
}

void c::process(const evidence& e) {

    auto t = static_cast<const tx&>(e);

	//tx verification
	local_delta::batch_t batch;

	account_t state;

	for (size_t j=0; j<t.inputs.size(); ++j) {
		auto& i=t.inputs[j];
		if (!account_state(batch,i.address,state)) {
            cerr << "TX REJECTED " << endl;
			return;
		}

		if (!unlock(i.address, j,state.locking_program, i.locking_program_input, t)) {
            cerr << "TX REJECTED 4" << endl;
			return;
		}

		state.box-=i.amount;
		batch.add(i.address,state);
	}

	for (size_t j=0; j<t.outputs.size(); ++j) {
		auto& i=t.outputs[j];
		if (account_state(batch,i.address,state)) {
			if (unlikely(state.box!=0 && state.locking_program!=i.locking_program)) { //locking program can only be replaced when balance is 0
                cerr << "TX REJECTED 5" << endl;
				return;
			}
			state.box+=i.amount;
		}
		else { //new output
			state.locking_program=i.locking_program;
			state.box=i.amount;
		}


		batch.add(i.address,state);
	}
    auto fee=t.check();
	pool->fees+=fee;

	pool->accounts.add(batch);
/*
//#ifdef DEBUG
cout << "TX added to mempool:" << endl;
{
    	lock_guard<mutex> lock(mx_pool);
        pool->accounts.dump(cout);
        cout << "fees: " << pool->fees << endl;
}
*/
//#endif
}

void c::local_delta::to_stream(ostream& os) const {
	os << accounts.size() << " ";
	for (auto& i:accounts) {
		os << i.first << ' ';
		i.second.to_stream(os);
	}
	os << fees << ' ';
	b::to_stream(os);
}

void c::local_delta::from_stream(istream& is) {
	int n;
	is >> n;
	for (int i=0; i<n; ++i) {
		hash_t pubk;
		is >> pubk;

		batch_item bi=batch_item::from_stream(is);
		accounts.emplace(move(pubk),move(bi));
	}
	is >> fees;
	b::from_stream(is);
}

void c::delta::to_stream(ostream& os) const {
	g.to_stream(os);
	b::b1::to_stream(os);
}

c::delta* c::delta::from_stream(istream& is) {
	delta* g=new delta();
	g->g.from_stream(is);

	static_cast<b*>(g)->from_stream(is);
	return g;
}

string c::shell_command(const string& cmdline) {
	ostringstream os;
	istringstream is(cmdline);
	string cmd;
	is >> cmd;
	if (cmd=="hello") {
		os << "cash shell. type h for help." << endl;
	}
	else if (cmd=="h" || cmd=="help") {
		os << "Cash shell." << endl;
		os << "h|help              Shows this help." << endl;
		os << "p|policies [id vote]          ." << endl;
		os << "mempool           Dumps the mempool." << endl;
		os << "exit                Exits this app and returns to parent shell." << endl;
		os << "" << endl;
	}
	else if (cmd=="p" || cmd=="policies") {
		int n=-1;
		double value;
		is >> n;
		is >> value;
		if (n>=0 && n<policies_traits::num_params) {
			lock_guard<mutex> lock(mx_policies);
			policies_local[n]=value;
		}
		else {
			os << "parameter " << n << " not found" << endl;
		}
		dump_policies(os);
	}
	else if (cmd=="mempool") {
    	lock_guard<mutex> lock(mx_pool);
        pool->accounts.dump(os);
        os << "fees: " << pool->fees << endl;

    }
	else if (cmd=="exit") {
	}
	else {
		os << "Unrecognized command" << endl;
	}
	return os.str();
}

void c::db_t::clear() {
	lock_guard<mutex> lock(mx);
    delete accounts;
	accounts=new accounts_t();
    supply_left=2100000000000000; //21.000.000e8  21e14
    block_reward=500000000; //5e8
}

void c::dbhash(hasher_t&) const {
assert(false);
}

void c::clear() {
    {
	lock_guard<mutex> lock(mx_policies);
    policies.clear();
    }
    db.clear();
}

void c::dump_policies(ostream& os) const {
	lock_guard<mutex> lock(mx_policies);
	os << policies_traits::num_params << " consensus variables:" << endl;
	for (int i=0; i<policies_traits::num_params; ++i) {
		os << "  " << i << ". " << policies_traits::paramstr[i] << " [avg] consensus value: " << policies[i] << " local value:" << policies_local[i] << endl;
	}
}

c::local_delta& c::local_delta::operator =(int zero) {
	*this=local_delta();
	return *this;
}

c::local_delta::local_delta() {
}

c::local_delta::~local_delta() {
//               cout << "cash::app::~local_delta" << endl;
}

c::local_delta::account_t::account_t(): box(0) {
}

c::local_delta::account_t::account_t(const hash_t& locking_program, const box_t& box): locking_program(locking_program), box(box) {
}

void c::local_delta::account_t::to_stream(ostream& os) const {
	os << locking_program << ' ' << box << ' ';
}

c::local_delta::account_t c::local_delta::account_t::from_stream(istream& is) {
	account_t i;
	is >> i.locking_program;
	is >> i.box;
	return move(i);
}

void c::local_delta::batch_t::add(const hash_t& address, const batch_item& bi) {
	auto i=find(address);
	if (likely(i==end())) {
		emplace(address,bi);
	}
	else {
		i->second=bi;
	}
}

void c::local_delta::accounts_t::add(const batch_t& batch) {
	for (auto& b:batch) {
		auto i=find(b.first);
		if (likely(i==end())) {
			emplace(b);
		}
		else {
			i->second=b.second;
		}
	}
}

c::local_delta::box_t c::local_delta::accounts_t::get_balance() const {
	box_t b=0;
	for (auto&i:*this) {
		b+=i.second.box;
	}
	return move(b);
}

void c::local_delta::accounts_t::to_stream(ostream& os) const {
	os << size() << ' ';
	for (auto& i:*this) {
		os << i.first << ' ';
		i.second.to_stream(os);
	}
}
c::local_delta::accounts_t* c::local_delta::accounts_t::from_stream(istream& is) {
	accounts_t* r=new accounts_t();
	size_t n;
	is >> n;
	for (size_t i=0; i<n; ++i) {
		hash_t h;
		is >> h;
		r->emplace(h,move(account_t::from_stream(is)));
	}
	return r;
}

const hash_t& c::local_delta::get_hash() const {
	if (hash==0) hash=compute_hash();
	return hash;
}

hash_t c::local_delta::compute_hash() const { //include only the elements controlled by majority_consensus
	hasher_t h;
	for (auto&i:accounts) {
		h.write(i.first);
		h.write(i.second.box);
		h.write(i.second.locking_program);
	}
	h.write(fees);
	hash_t v;
	h.finalize(v);
	return move(v);
}

uint64_t c::delta::merge(engine::app::local_delta* other0) {
	local_delta* other=static_cast<local_delta*>(other0);
	m->merge(*other,*other);

	b::merge(other0);

//cout << "MERGE: other.fees=" << other->fees << endl;
	return 0; //other->fees;
}

void c::delta::end_merge() {
	m->end_merge(g, 0);
    b::end_merge();
//cout << "END MERGE: g.fees=" << g.fees << endl;

//			double m=multiplicity;
//			for (int i=0; i<policies_traits::num_params; ++i) policies[i]/=m;
}

c::delta::delta():m(new engine::majority_merger<local_delta>) {
}

c::delta::~delta() {
	delete m;
}

c::db_t::db_t() {
	clear();
}
c::db_t::db_t(db_t&& other):supply_left(other.supply_left), block_reward(other.block_reward) {
	accounts=other.accounts;
	other.accounts=0;
}
c::db_t::~db_t() {
	delete accounts;
}

