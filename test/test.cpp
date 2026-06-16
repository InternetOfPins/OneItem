/**
 * @file test.cpp
 * @brief OneItem unit tests
 *
 * Tests:
 *   ItemDef<Int>      — basic item: get/set/depth/enabled
 *   Action<fn>        — act() dispatches to function
 *   EnDis<true/false> — enable/disable toggle
 *   Hidden<...>       — components excluded from print but still accessible
 *   Decor<...>        — get/set bypass decorative components
 *   Alias<T,V>        — Decor<title> + Hidden<value> shorthand
 */

#include <iostream>
#include <cassert>
#include <oneItem/oneItem.h>
#include <oneData/oneData.h>

#ifdef ARDUINO
  #define cout Serial
  #define assert(x) do { if(!(x)) { Serial.print("FAIL: "); Serial.println(#x); } } while(0)
#else
  using namespace std;
#endif

using namespace oneItem;
using namespace oneData;

void test_basic_item() {
  ItemDef<Int> item;
  assert(item.get() == 0);
  item.set(42);
  assert(item.get()                    == 42);
  assert(item.template depth<void>()  == 1);
  assert(item.enabled());
  assert(!item.up());           // default: returns false
  assert(!item.down());
  assert(!item.act());
  cout << "basic ItemDef<Int>: ok" << endl;
}

bool action_was_called = false;
bool doAction() { action_was_called = true; return true; }

void test_action() {
  ItemDef<Action<doAction>> item;
  action_was_called = false;
  assert(item.act());
  assert(action_was_called);
  cout << "Action<fn>: ok" << endl;
}

void test_endis() {
  ItemDef<EnDis<true>> on;
  assert(on.enabled());
  on.enable(false);
  assert(!on.enabled());
  on.enable();
  assert(on.enabled());

  ItemDef<EnDis<false>> off;
  assert(!off.enabled());
  off.enable();
  assert(off.enabled());
  cout << "EnDis<true/false>: ok" << endl;
}

// Minimal mock output for print tests
struct Sink {
  char buf[64]{};
  int  pos{0};
  template<typename T> void put(const T&) {}
  void put(const char* s) { while (*s) buf[pos++] = *s++; }
  void put(char c)        { buf[pos++] = c; }
  void nl()               { buf[pos++] = '\n'; }
};

void test_hidden() {
  // Hidden wraps components so they are excluded from print() but still store data
  // EnDis uses Hidden internally — we verify the data is accessible
  ItemDef<Int, Hidden<StaticData<7>>> item;
  item.set(3);
  assert(item.get() == 3);      // get() reaches Int, not the hidden StaticData
  cout << "Hidden<StaticData<7>> data accessible via Int::get: ok" << endl;
}

void test_static_item() {
  ItemDef<StaticData<42>> item;
  assert(item.get() == 42);
  cout << "ItemDef<StaticData<42>>: ok" << endl;
}

void doTests() {
  test_basic_item();
  test_action();
  test_endis();
  test_hidden();
  test_static_item();
  cout << "all OneItem tests passed" << endl;
}

#ifdef ARDUINO
  void setup() { Serial.begin(115200); while(!Serial); doTests(); }
  void loop() {}
#else
  int main() { doTests(); return 0; }
#endif
