//------------------file generated by apitool- do not edit
#ifndef US_GOV_WALLET_DAEMON_API_H
#define US_GOV_WALLET_DAEMON_API_H

#include "wallet_api.h"
#include "pairing_api.h"

namespace us{ namespace wallet {
using namespace std;

struct daemon_api: wallet_api, pairing_api  {
  virtual ~daemon_api() {}
  using wallet_api::priv_t;
  using wallet_api::pub_t;

#include <us/api/apitool_generated__functions_wallet_cpp_purevir>  //APITOOL
#include <us/api/apitool_generated__functions_pairing_cpp_purevir>  //APITOOL

};
}}
#endif
