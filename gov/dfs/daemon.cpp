#include "daemon.h"
#include "protocol.h"

#include <filesystem>
#include <fstream>

using namespace std::chrono;
namespace fs = std::filesystem;

using namespace us::gov;
using namespace us::gov::dfs;
typedef us::gov::dfs::daemon c;

bool c::process_work(socket::peer_t *c, datagram*d) {
	if (b::process_work(c,d)) return true;
	peer_t* p=static_cast<peer_t*>(c);
	switch(d->service) {
		    case protocol::file_request: request(p,d); break;
		    case protocol::file_response: response(p,d); break;
		    default: return false;
    }
	return true;
}

void c::request(peer_t *c, datagram*d) {
    auto  hash_b58=d->parse_string();
    delete d;
    cout << "received request for file " << hash_b58 << endl;

    ifstream is(get_path_from(hash_b58));
    if (!is.good()) {
        cout << "block not found in HD, ignoring." << endl;
        return;
    }

    cout << "sending  file " << hash_b58 << endl;
    string content=string((istreambuf_iterator<char>(is)), (istreambuf_iterator<char>()));
    c->send(new datagram(protocol::file_response,content));
}

void c::response(peer_t *c, datagram*d) {
    auto hash_b58 = d->compute_payload_hash().to_b58();

    string filename=get_path_from(hash_b58,true);
    if (unlikely(!fs::exists(filename))) {
        {
            ofstream os(filename);
            auto content=d->parse_string();
            os << content;
        }

        file_cv.notify_and_erase(hash_b58);
    }

    delete d;
    file_cv.purge();
}

void c::file_cv_t::notify_and_erase(const string& hash_b58) {
    lock_guard<mutex> lock(mx);
    auto it = this->find(hash_b58);

    if(unlikely(it != this->end())) {
        assert(it->second.pcv != 0);
        it->second.file_arrived=true;
        it->second.pcv->notify_all();
        delete it->second.pcv;
        this->erase(it);
    }
}

void c::file_cv_t::add(const string& hash_b58, condition_variable * pcv, bool file_arrived) {
    lock_guard<mutex> lock(mx);
    assert(pcv!=0);
    auto now = system_clock::now();
    this->emplace(hash_b58, c::params({pcv,now,file_arrived}));
}

void c::file_cv_t::purge() {
    lock_guard<mutex> lock(mx);
    auto it = this->begin();
    auto now = system_clock::now();

    while(it != this->end()) {
        auto elapsed = duration_cast<seconds>(now-it->second.time);
        if(elapsed>60s) {
            delete it->second.pcv;
            it = this->erase(it);
        } else {
            it++;
        }
    }
}

bool c::file_cv_t::exists(const string& hash_b58) {
    lock_guard<mutex> lock(mx);
    return this->find(hash_b58) != this->end();
}

void c::file_cv_t::wait_for(const string& hash_b58) {
    unique_lock<mutex> lock(mx);
    auto it = find(hash_b58);
    if(it !=  this->end()) {
        it->second.pcv->wait_for(lock, 3s, [&]{ return it->second.file_arrived || program::_this.terminated; });
        it->second.file_arrived = false;
    }

}

string c::load(const string& hash_b58, condition_variable * pcv, bool file_arrived) {
    auto filename=homedir +"/"+resolve_filename(hash_b58);
    if(fs::exists(filename)) {
        return filename;
    } else {
        file_cv.add(hash_b58,pcv,file_arrived);
        auto n=this->get_random_edge();
        if (unlikely(n==0)) return "";
        cout << "DFS: querying file for " << hash_b58 << endl;
        auto r=n->send(new datagram(protocol::file_request,hash_b58));
        return "";
    }
}

string c::load(const string& hash_b58) {
    string filename{""};
    if(!file_cv.exists(hash_b58)) {
        condition_variable * pcv = new condition_variable;
        filename=load(hash_b58, pcv, false);
    }

    if(filename.empty()) file_cv.wait_for(hash_b58);

    return filename;
}

void c::save(const string& hash, const vector<uint8_t>& data, int propagate) { //-1 nopes, 0=all peers; n num of peers
assert(false);
/*
    cout << "dfs: save file " << hash << " propagate=" << propagate << endl;

    if (propagate==-1) return;

    //if file already exists return
    //save with tmpname
    //rename atomically
*/
}

string c::resolve_filename(const string& filename) {
	
	string res;
	
	int max_length = filename.size()/2 +filename.size(); //final string length with slashes
	res.reserve(max_length);
    for(int i=0; i < filename.size(); i++){// slash"/" every 2 char
        if((i&1) == 0){
			res+="/";
        }
		res+=filename[i];
    }
	
	if(res[res.size()-1]=='/'){
		res.end()-1;//delete the last slash '/'
	}
	return &res[1];
}

string c::get_path_from(const string& hash_b58, bool create_dirs) const {
    auto file_path=homedir +"/"+resolve_filename(hash_b58);

    if(create_dirs) {
        fs::path dir(file_path);

        auto p=dir.parent_path();
        if(!(fs::exists(p))) {
            assert(fs::create_directories(p));
        }
    }

    return file_path;
}

void c::dump(ostream&os) const {
    os << "Hello from dfs::daemon" << endl;
    os << "Nothing to say here yet" << endl;    
}

