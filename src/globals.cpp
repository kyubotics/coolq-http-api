#include "app.h"

using namespace std;

optional<Sdk> sdk; // will be initialized in "Initialize" event
Config config; // will be initiated in "Enable" event
shared_ptr<ctpl::thread_pool> pool; // will be initiated in "Enable" event
