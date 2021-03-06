#include "auth_app.h"
#include "daemon.h"

using namespace std;
using namespace us::gov;
using namespace us::gov::engine::auth;

typedef us::gov::engine::auth::app c;

constexpr array<const char*,policies_traits::num_params> policies_traits::paramstr;

c::app(pubkey_t&pk): node_pubkey(pk) {
	pool=new app::local_delta();
	policies_local=policies;
}

c::~app() {
	delete pool;
}

us::gov::engine::peer_t::stage_t c::my_stage() const {
//	if (cache_my_stage!=peer_t::unknown) return cache_my_stage;   // TODO enable cache
	auto k=node_pubkey.hash();
//cout << k << endl;
//cout << db.nodes.size() << endl;
//cout << endl;
//cout << db.hall.size() << endl;
	if (db.nodes.find(k)!=db.nodes.end()) {
		cache_my_stage=peer_t::node;
//cout << "SET node" << endl;
	}
	else if (db.hall.find(k)!=db.hall.end()) {
		cache_my_stage=peer_t::hall;
//cout << "SET hall" << endl;
	}
	else {
		cache_my_stage=peer_t::out;
	}
	return cache_my_stage;
}

void c::run() {
	while(!program::_this.terminated) {
		db.dump(cout);
//		cout << "I am " << peer_t::stagestr[my_stage()] << endl;
		thread_::_this.sleep_for(chrono::seconds(30));
	}
}

void c::basic_auth_completed(peer_t* p) {
//	cout << "APP auth_app, basic_auth_completed for " <<  p->pubkey << endl;
	if (p->pubkey==node_pubkey) { //parent->peerd.id.pub) {  ///sysop connection, this connection requires a shell
		p->stage=peer_t::sysop;
//		cout << "SYSOP " << endl;
		return;
	}
	{
	lock_guard<mutex> lock(db.mx_nodes);
	auto i=db.nodes.find(p->pubkey.hash());
	if (i!=db.nodes.end()) {
		p->stage=peer_t::node;
//		cout << "NODE " << endl;
		return;
	}
	}
	{
	db_t::nodes_t::const_iterator i;
	{
	lock_guard<mutex> lock(db.mx_hall);
	i=db.hall.find(p->pubkey.hash());
	}
	if (i==db.hall.end()) {
		p->stage=peer_t::out;
		pool->to_hall.push_back(make_pair(p->pubkey.hash(),p->addr));
	}
	else {
		p->stage=peer_t::hall;
	}
	}
}

void c::add_policies() {
	lock_guard<mutex> lock(mx_pool);
	lock_guard<mutex> lock2(mx_policies);
	for (int i=0; i<policies_traits::num_params; ++i)
		(*pool)[i] = policies_local[i];
}

void c::add_growth_transactions(unsigned int seed) {

	double growth=policies[policies_traits::network_growth]; //percentage
	if (abs(growth)<1e-8) return; // no growth

	int min_growth=round(policies[policies_traits::network_min_growth]);
	default_random_engine generator(seed);
	db_t::nodes_t* src;
	db_t::nodes_t* dst;
	unordered_set<int> uniq;
	size_t maxr;
	int s;
	lock_guard<mutex> lock(db.mx_hall);
	lock_guard<mutex> lock2(db.mx_nodes);

	if (growth>=0) {
		size_t nh=db.hall.size();
		s=floor((double)nh*growth);
		src=&db.hall;
		dst=&db.nodes;
		if (s<min_growth) s=min_growth;
		if (s>nh) s=nh;
		maxr=nh-1;
	//cout << "grow the network with " << s << " nodes." << endl;
	}
	else {
		size_t nn=db.nodes.size();
		s=-floor((double)nn*growth);
		src=&db.nodes;
		dst=&db.hall;
		maxr=nn-1;
	//cout << "shrink the network with " << s << " nodes." << endl;
	}
	uniform_int_distribution<size_t> distribution(0,maxr);
//cout << s << " " << maxr << endl;
	for (size_t i=0; i<s; ++i) { ///move s from hall to nodes
		auto p=src->begin();
		size_t r;
		while (true) {
			r=distribution(generator);
			if (!uniq.insert(r).second) continue;
			if (r>=src->size()) continue;
			break;
		}
		advance(p,r);
		dst->emplace(*p);
		src->erase(p);
//cout << "dddooone" << endl;
	}
}

int us::gov::engine::auth::app::local_delta::app_id() const {
	return app::id();
}

void us::gov::engine::auth::app::local_delta::to_stream(ostream& os) const {
	os << to_hall.size() << " ";
	for (auto& i:to_hall) {
		os << i.first << " " << i.second << " ";
	}
	b::to_stream(os);
}

void us::gov::engine::auth::app::local_delta::from_stream(istream& is) {
	int n;
	is >> n;
	to_hall.reserve(n);
	for (int i=0; i<n; ++i) {
		pubkey_t::hash_t pkeyh;
		address addr;
		is >> pkeyh;
		is >> addr;
		to_hall.push_back(make_pair(pkeyh,addr));
	}
	b::from_stream(is);
}

void us::gov::engine::auth::app::delta::to_stream(ostream& os) const {
	os << to_hall.size() << " ";
	for (auto& i:to_hall) {
		os << i.first << " " << i.second << " ";
	}
	b::b1::to_stream(os);
}

app::delta* us::gov::engine::auth::app::delta::from_stream(istream& is) {
	delta* g=new delta();
	{
	int n;
	is >> n;
	for (int i=0; i<n; ++i) {
		pubkey_t::hash_t pkeyh;
		address addr;
		is >> pkeyh;
		is >> addr;
		g->to_hall.emplace(make_pair(pkeyh,addr));
	}
	}

	static_cast<b*>(g)->from_stream(is);
	return g;
}

void app::import(const engine::app::delta& gg, const engine::pow_t&) {
	const delta& g=static_cast<const delta&>(gg);
	{
	lock_guard<mutex> lock(db.mx_hall);
	for (auto& i:g.to_hall) {
		{
		auto k=db.hall.find(i.first);
		if (k==db.hall.end()) db.hall.emplace(i);
		}
	}
	}
	{
	lock_guard<mutex> lock(mx_policies);
	for (int i=0; i<policies_traits::num_params; ++i) policies[i]=g[i];
	}

	//auto seed=parent->get_seed();

	add_growth_transactions(get_seed());

	cache_my_stage=peer_t::unknown;
}

engine::peer_t::stage_t app::db_t::get_stage(const pubkeyh_t& key) const {
	{
	lock_guard<mutex> lock(mx_nodes);
	if (nodes.find(key)!=nodes.end()) return peer_t::node;
	}

	{
	lock_guard<mutex> lock(mx_hall);
	if (hall.find(key)!=hall.end()) return peer_t::hall;
	}
	return peer_t::out;
}



us::gov::engine::app::local_delta* c::create_local_delta() {
	//cout << "app: auth: create_local_delta " << endl;
	add_policies();
	lock_guard<mutex> lock(mx_pool);
	auto full=pool;
	pool=new app::local_delta();
	return full; //send collected transactions to the network
}

#include <random>
void c::db_t::dump(ostream& os) const {
//	cout << "Auth app db dump" << endl;
	{
	lock_guard<mutex> lock(mx_nodes);
	cout << nodes.size() << " nodes:" << endl;
	for (auto& i:nodes) {
		cout << "  " << i.first << " " << i.second << endl;
	}
	}
	{
	lock_guard<mutex> lock(mx_hall);
	cout << hall.size() << " candidates in hall:" << endl;
	for (auto& i:hall) {
		cout << "  " << i.first << " " << i.second << endl;
	}
	}
}

string c::get_random_node(mt19937_64& rng, const unordered_set<string>& exclude_addrs) const {
	lock_guard<mutex> lock(db.mx_nodes);
	if (db.nodes.empty()) return "";
	uniform_int_distribution<> d(0, db.nodes.size()-1);
	for (int j=0; j<10; ++j) {
		auto i=db.nodes.begin();
		advance(i,d(rng));
		if (i->first!=node_pubkey.hash() && exclude_addrs.find(i->second)==exclude_addrs.end()) {
			return i->second;
		}
	}
	return "";
}

string c::shell_command(const string& cmdline) {
	ostringstream os;
	istringstream is(cmdline);
	string cmd;
	is >> cmd;
	if (cmd=="hello") {
		os << "Auth app shell. type h for help." << endl;
	}
	else if (cmd=="h" || cmd=="help") {
		os << "Auth app shell." << endl;
		os << "h|help              Shows this help." << endl;
		os << "p|policies [id vote]          ." << endl;
		os << "s|server            Nodes,hall." << endl;
		os << "exit                Exits this app and returns to parent shell." << endl;
		os << "" << endl;
	}
	else if (cmd=="exit") {
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
    else if (cmd=="s" || cmd=="server") {
        db.dump(os);
        os << "I am " << peer_t::stagestr[my_stage()] << endl;
    }
	else {
		os << "Unrecognized command" << endl;
	}
	return os.str();
}

void c::dump_policies(ostream& os) const {
	lock_guard<mutex> lock(mx_policies);
	os << policies_traits::num_params << " consensus variables:" << endl;
	for (int i=0; i<policies_traits::num_params; ++i) {
		os << "  " << i << ". " << policies_traits::paramstr[i] << " [avg] consensus value: " << policies[i] << " local value:" << policies_local[i] << endl;
	}
}

void c::dbhash(hasher_t& h) const {
	db.hash(h);
	lock_guard<mutex> lock(mx_policies);
	policies.hash(h);

}

void c::db_t::hash(hasher_t& h) const {
	{
	lock_guard<mutex> lock(mx_nodes);
	for (auto& i:nodes) {
		h << i.first << i.second;
	}
	}
	{
	lock_guard<mutex> lock(mx_hall);
	for (auto& i:hall) {
		h << i.first << i.second;
	}
	}
}

void c::db_t::clear() {
	{
	lock_guard<mutex> lock(mx_nodes);
    nodes.clear();
	}
	{
	lock_guard<mutex> lock(mx_hall);
    hall.clear();
	}
}


void c::clear() {
    {
	lock_guard<mutex> lock(mx_policies);
    policies.clear();
    }
    db.clear();

}





