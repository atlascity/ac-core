#ifndef USGOV_5369cd4184b5cf59017909fa4ac73e6f59eb5af656bfd6f5212953d549e6aba5
#define USGOV_5369cd4184b5cf59017909fa4ac73e6f59eb5af656bfd6f5212953d549e6aba5

#include <us/gov/dfs/daemon.h>

namespace us { namespace gov {
namespace engine {
	using namespace std;

	struct peer_t:dfs::peer_t {
		typedef dfs::peer_t b;
		enum stage_t {
			unknown=0,
			sysop,
			out,
			hall,
			 node,
            wallet,
			num_stages
		};
		constexpr static array<const char*,num_stages> stagestr={"unknown","sysop","out","hall","node","wallet"};

		peer_t(int sock):b(sock) {}
		virtual ~peer_t() {
			disconnect();
		}

		void dump(ostream& os) const;
                virtual void dump_all(ostream& os) const override {
                        dump(os);
                        b::dump_all(os);
                }

		virtual void verification_completed() override;

        virtual bool authorize(const pubkey_t& p) const override;

           virtual const keys& get_keys() const override;
/*
public:
        stage_t get_stage() const { return stage; }
        void set_stage(stage_t s) { stage=s; }
private:
*/
		stage_t stage{unknown};
	};


}
}}

#endif

