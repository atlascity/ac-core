package us.wallet;
import java.security.KeyPair;
import java.io.OutputStream;
import us.gov.cash.tx.sigcode_t;

public class daemon_rpc_api implements daemon_api {

        wallet_rpc_api w;
        pairing_rpc_api p;
        rpc_api endpoint;

        public daemon_rpc_api(KeyPair k, String whost, short wport) {
            try {
            endpoint=new rpc_api(k, whost, wport);
            w=new wallet_rpc_api(endpoint);
            p=new pairing_rpc_api(endpoint);
            } catch (Exception e) {
                System.out.println("Daemon_rpc_api" + e);
                e.printStackTrace();
            }
        }

//---------------------------------------------------------------------generated by make, do not edit
//content of file: ../../api/apitool_generated__functions_wallet-daemon_java_impl
//------------------generated by apitool- do not edit
// wallet - master file: us/apitool/data/wallet
  @Override public void balance(boolean a0, OutputStream a1) { w.balance(a0, a1); }
  @Override public void list(boolean a0, OutputStream a1) { w.list(a0, a1); }
  @Override public void new_address(OutputStream a0) { w.new_address(a0); }
  @Override public void add_address(String a0, OutputStream a1) { w.add_address(a0, a1); }
  @Override public void transfer(String a0, long a1, OutputStream a2) { w.transfer(a0, a1, a2); }
  @Override public void tx_make_p2pkh(tx_make_p2pkh_input a0, OutputStream a1) { w.tx_make_p2pkh(a0, a1); }
  @Override public void tx_sign(String a0, sigcode_t a1, sigcode_t a2, OutputStream a3) { w.tx_sign(a0, a1, a2, a3); }
  @Override public void tx_send(String a0, OutputStream a1) { w.tx_send(a0, a1); }
  @Override public void tx_decode(String a0, OutputStream a1) { w.tx_decode(a0, a1); }
  @Override public void tx_check(String a0, OutputStream a1) { w.tx_check(a0, a1); }
  @Override public void ping(OutputStream a0) { w.ping(a0); }

// pairing - master file: us/apitool/data/pairing
  @Override public void pair(String a0, String a1, OutputStream a2) { p.pair(a0, a1, a2); }
  @Override public void unpair(String a0, OutputStream a1) { p.unpair(a0, a1); }
  @Override public void list_devices(OutputStream a0) { p.list_devices(a0); }
//-/----------------generated by apitool- do not edit

//-/-------------------------------------------------------------------generated by make, do not edit
}

