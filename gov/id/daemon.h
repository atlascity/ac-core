#ifndef USGOV_d503861e22c19bdbfae6ed39978276fcf121cce8fb47df51d65ca7fb26ff55e9
#define USGOV_d503861e22c19bdbfae6ed39978276fcf121cce8fb47df51d65ca7fb26ff55e9

#include <us/gov/socket/daemon.h>
#include <us/gov/crypto.h>
//#include <us/gov/peer.h>

namespace us { namespace gov {
namespace id {

using namespace std;

//struct daemon:peer::daemon {
struct daemon:socket::daemon {
//	typedef peer::daemon b;
	typedef socket::daemon b;
    typedef crypto::ec::keys keys;

	using datagram=socket::datagram;

	daemon();
        daemon(uint16_t port, uint16_t edges);
	virtual ~daemon();

	virtual socket::client* create_client(int sock) override;
	void dump(ostream& os) const;

	virtual bool process_work(socket::peer_t *p, datagram*d) override;

        virtual const keys& get_keys() const=0;

        bool is_duplicate(const pub_t&) const;
};


}
}}

#endif

