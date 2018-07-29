/*

   public static final short wallet_base = 0;
    public static final short protocol_balance_query = wallet_base+1;
    public static final short protocol_dump_query = wallet_base+2;
    public static final short protocol_new_address_query = wallet_base+3;
    public static final short protocol_add_address_query = wallet_base+4;
    public static final short protocol_tx_make_p2pkh_query = wallet_base+5;
    public static final short protocol_tx_sign_query = wallet_base+6;
    public static final short protocol_tx_send_query = wallet_base+7;
    public static final short protocol_tx_decode_query = wallet_base+8;
    public static final short protocol_tx_check_query = wallet_base+9;
    public static final short protocol_pair_query = wallet_base+10;
    public static final short protocol_unpair_query = wallet_base+11;
    public static final short protocol_list_devices_query = wallet_base+12;

    public static final short protocol_response = wallet_base+0;
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <functional>
using namespace std;


void gen_cpp(const vector<string>& v, ostream& os) {
    os << "//------------------generated by apitool- do not edit" << endl;
    int n=0;
    const string& base=v[0];
    os << "    static constexpr uint16_t " << v[n++] << "{0};" << endl;
    for (;n<v.size();++n) {
        os << "    static constexpr uint16_t " << v[n] << "{" << base << "+" << n << "};" << endl;
    }
    os << "//-/----------------generated by apitool- do not edit" << endl;
}


void gen_java(const vector<string>& v, ostream& os) {
    os << "//------------------generated by apitool- do not edit" << endl;
    int n=0;
    const string& base=v[0];
    os << "    public static final short " << v[n++] << " = 0;" << endl;
    for (;n<v.size();++n) {
        os << "    public static final short protocol_" << v[n] << " = " << base << "+" << n << ";" << endl;
    }
    os << "//-/----------------generated by apitool- do not edit" << endl;
}


void gen_gov_cpp(int mbase, const vector<string>& v, ostream& os) {
    os << "//------------------generated by apitool- do not edit" << endl;
    int n=0;
    const string& base=v[0];
    os << "    static constexpr uint16_t " << v[n++] << "{" << mbase << "};" << endl;
    for (;n<v.size();++n) {
        os << "    static constexpr uint16_t " << v[n] << "{((" << base << "+" << n-1 << ")<<2)+protocol_suffix};" << endl;
    }
    os << "//-/----------------generated by apitool- do not edit" << endl;
}
void gen_gov_java(int mbase, const vector<string>& v, ostream& os) {
    os << "//------------------generated by apitool- do not edit" << endl;
    int n=0;
    const string& base=v[0];
    os << "    public static final short gov_" << v[n++] << " = " << mbase << ";" << endl;
    for (;n<v.size();++n) {
        os << "    public static final short gov_" << v[n] << " = ((gov_" << base << "+" << n-1 << ")<<2)+protocol_suffix;" << endl;
    }
    os << "//-/----------------generated by apitool- do not edit" << endl;
}

void gen_gov(int base, const vector<string>& v, ostream& os) {
    cout << "C++" << endl;
    gen_gov_cpp(base,v,os);
    cout << "files affected:" << endl;
    cout << "  gov/id/protocol.h" << endl;
    cout << endl;

    cout << "java" << endl;

    gen_gov_java(base,v,os);
    cout << "files affected:" << endl;
    cout << "  sdk/java/src/wallet.h" << endl;
    cout << endl;

}

int protocol_main() {

vector<string> f{ "wallet_base","balance_query","list_query",
    "new_address_query","add_address_query","tx_make_p2pkh_query",
    "tx_sign_query","tx_send_query","tx_decode_query",
    "tx_check_query","pair_query","unpair_query","list_devices_query",
    "ping","response" };


vector<string> gov_id{
    "id_base",
    "id_request",
    "id_peer_challenge",
    "id_challenge_response",
    "id_peer_status"
};


    cout << "-------------wallet" << endl;
   
    cout << "C++" << endl;
    gen_cpp(f,cout);
    cout << "files affected:" << endl;
    cout << "  wallet/protocol.h" << endl;
    cout << endl;

    cout << "java" << endl;
    gen_java(f,cout);
    cout << "files affected:" << endl;
    cout << "  sdk/java/src/Wallet.java" << endl;


    cout << "-------------gov" << endl;
    cout << "" << endl;
    cout << "-------------gov::id" << endl;
    gen_gov(200,gov_id,cout);

    return 0;
}

    struct f: vector<string>  {
        typedef vector<string> args;
        string name;
        string fcgi;
        static f from_stream(istream& is) {
            f r;        
            int n;
            is >> r.name;
            is >> n;
            if (r.name.empty()) {
                return r;
            }
            string dummy;
            getline(is,dummy);    
            r.reserve(n);
            for (int i=0; i<n; ++i) {
                string line;
                getline(is,line);    
                if (!line.empty()) {
                    r.push_back(line);
                }
            }
            getline(is,r.fcgi);
            r.push_back("ostream&");
            return r;
        }
        void gen_cpp_purevir(ostream& os) const {
            os << "  virtual void " << name << "(";
            if (!empty()) {
                auto e=end();
                --e;
                for (auto i=cbegin(); i!=e; ++i) {
                    os << *i << ", ";
                }
               os << *rbegin();
            }
           os << ")=0;" << endl;
            
        }
        void gen_cpp_override(ostream& os) const {
            os << "  virtual void " << name << "(";
            if (!empty()) {
                auto e=end();
                --e;
                for (auto i=cbegin(); i!=e; ++i) {
                    os << *i << ", ";
                }
               os << *rbegin();
            }
           os << ") override;" << endl;
            
        }
        void gen_cpp_delegate(const string& typeredirect, ostream& os) const {
            os << "  inline virtual void " << name << "(";
            int j=0;
            if (!empty()) {
                auto e=end();
                --e;
                for (auto i=cbegin(); i!=e; ++i) {
                    os << *i << " a" << j++ << ", ";
                }
               os << *rbegin() << " a" << j;
            }
           os << ") override { " << typeredirect << "::" << name << "(";
            j=0;
            if (!empty()) {
                auto e=end();
                --e;
                for (auto i=cbegin(); i!=e; ++i) {
                    os << "a" << j++ << ", ";
                }
                os << "a" << j;
            }
            os << "); }" << endl;
            
        }
    };


struct api_t: vector<f> {
    string src;
    string name;

    void warn_h(ostream& os) const {
        os << "//------------------generated by apitool- do not edit" << endl;
    }
    void warn_f(ostream& os) const {
       os << "//-/----------------generated by apitool- do not edit" << endl;
    }

    static api_t from_stream(istream&is) {
        api_t api;
        while (is.good()) {
            f i=f::from_stream(is);
            if (!is.good()) {
                break;
            }
            api.push_back(i);
        }
        return api;
    }

    static api_t load(string file) {
        string nm=file;
        file=string("data/")+file;
        ifstream is(file);
        auto a=from_stream(is);
        a.name=nm;
        a.src=file;
        return move(a);
    }
 


        void gen_cpp_purevir(auto header, auto footer, ostream& os) const {
//            os << "//------------------generated by apitool- do not edit" << endl;
            header(os);
            for (auto&i:*this) {
                i.gen_cpp_purevir(os);
            }
            footer(os);
//            os << "//-/----------------generated by apitool- do not edit" << endl;
        }
    void info(ostream&os) const {
       os << "// " << name << " - master file: us/apitool/" << src <<  endl;
    }

    void gen_cpp_override(ostream&os) const {
        info(os);
            for (auto&i:*this) {
                i.gen_cpp_override(os);
            }
    }
    void gen_cpp_purevir(ostream&os) const {
        info(os);
            for (auto&i:*this) {
                i.gen_cpp_purevir(os);
            }
    }
    void gen_cpp_wallet_daemon_delegate(const string& t, ostream&os) const {
        info(os);
            for (auto&i:*this) {
                i.gen_cpp_delegate(t,os);
            }
    }

};


void include_snippet(const string& file,ostream&os) {
    ifstream f(file);
    while(f.good()) {
        string line;
        getline(f,line);
        os << line << endl;
    }
}

void gen_functions_cpp_override(const api_t&w, const api_t&p) {
    {
    string file="apitool_generated_wallet_functions_cpp_override";
    cout << "writting file " << file  << endl;
    ofstream os(file);
    w.warn_h(os);
    w.gen_cpp_override(os);
    w.warn_f(os);
    }
    {
    string file="apitool_generated_pairing_functions_cpp_override";
    cout << "writting file " << file  << endl;
    ofstream os(file);
    p.warn_h(os);
    p.gen_cpp_override(os);
    p.warn_f(os);
    }
}

void gen_functions_cpp_purevir(const api_t&w, const api_t&p) {
    {
    string file="apitool_generated_wallet_functions_cpp_purevir";
    cout << "writting file " << file  << endl;
    ofstream os(file);
    w.warn_h(os);
    w.gen_cpp_purevir(os);
    w.warn_f(os);
    }
    {
    string file="apitool_generated_pairing_functions_cpp_purevir";
    cout << "writting file " << file  << endl;
    ofstream os(file);
    p.warn_h(os);
    p.gen_cpp_purevir(os);
    p.warn_f(os);
    }

}

void gen_functions_wallet_daemon_impl(const api_t&w, const api_t&p) {
    string file="apitool_generated_wallet_daemon_functions_cpp_impl";
    cout << "writting file " << file  << endl;
    ofstream os(file);
    w.warn_h(os);
    w.gen_cpp_wallet_daemon_delegate("w",os);
    os << endl;
    p.gen_cpp_wallet_daemon_delegate("p",os);
    w.warn_f(os);
}



int main(int argc, char**argv) {
    auto w=api_t::load("wallet");
    auto p=api_t::load("pairing");

    gen_functions_cpp_purevir(w,p);
    gen_functions_cpp_override(w,p);
    gen_functions_wallet_daemon_impl(w,p);



}
