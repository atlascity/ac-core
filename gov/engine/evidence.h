#ifndef USGOV_ce0f37979bb3f36a0a0e6be8535da7b8c6bd6728d662a0c6819e31fb2b47c495
#define USGOV_ce0f37979bb3f36a0a0e6be8535da7b8c6bd6728d662a0c6819e31fb2b47c495

#include <chrono>

namespace us{ namespace gov {
namespace engine {
using namespace std;
using namespace chrono;

struct evidence {

    evidence(): ts(duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count()) {}
    virtual ~evidence() {}
    
     uint64_t ts;
 };

}
}}

#endif



