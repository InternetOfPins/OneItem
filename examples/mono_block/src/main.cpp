// OneItem mono_block tests
//
// Verifies OneItem components under the mono_block HAPI chain:
//   1. Action<fn>      — act() dispatches to the function
//   2. EnDis<bool>     — enable/disable, Hidden<Default<Bool,ens>>
//   3. Hidden<II...>   — components in II are excluded from print
//   4. Decor<II...>    — get/set bypass the decoration components
//   5. find<Q>         — locate tagged component in ItemDef
//   6. query<TagIs<Q>> — tag detection through ItemDef::Types

#include <cassert>
#include <sstream>
#include <iostream>
using namespace std;

#include <hapi/hapi.h>
#include <oneData/oneData.h>
#include <oneOutput/oneOutput.h>
#include <oneItem/oneItem.h>
using namespace hapi;
using namespace oneData;
using namespace oneOutput;
using namespace oneItem;

// Capture ConsoleOut into a string buffer
template<typename F>
static string capture(F fn) {
  ostringstream buf;
  streambuf* old = cout.rdbuf(buf.rdbuf());
  OutDef<ConsoleOut> out;
  fn(out);
  cout.rdbuf(old);
  return buf.str();
}

// ─── Test 1: Action<fn> ──────────────────────────────────────────────────────

static bool g_called = false;
static bool test_action_fn() { g_called = true; return true; }

void test_action() {
  ItemDef<Action<test_action_fn>> item;
  assert(!g_called);
  bool r = item.act();
  assert(r && g_called);
  cout << "PASS test_action\n";
}

// ─── Test 2: EnDis<bool> ─────────────────────────────────────────────────────

void test_endis() {
  ItemDef<EnDis<true>> en;
  assert(en.enabled());
  en.enable(false);
  assert(!en.enabled());
  en.enable(true);
  assert(en.enabled());

  ItemDef<EnDis<false>> dis;
  assert(!dis.enabled());
  cout << "PASS test_endis\n";
}

static constexpr const char* kHiddenLbl = "lbl:";
static constexpr const char* kDecorLbl  = "x:";

// ─── Test 3: Hidden<II...> excludes from print ───────────────────────────────

void test_hidden() {
  // print should skip the StaticText label
  ItemDef<Hidden<StaticText<&kHiddenLbl>>, Default<Int, 7>> item;
  string out = capture([&](auto& o) { item.print(o); });
  assert(out == "7");  // label is hidden, only the int prints
  cout << "PASS test_hidden\n";
}

// ─── Test 4: Decor<II...> bypasses decoration for get/set ────────────────────

void test_decor() {
  // Decor wraps StaticText; get()/set() go directly to the underlying Int
  using DecorItem = ItemDef<Decor<StaticText<&kDecorLbl>>, Default<Int, 3>>;
  DecorItem item;
  assert(item.get() == 3);
  item.set(9);
  assert(item.get() == 9);
  cout << "PASS test_decor\n";
}

// ─── Test 5: find<TagIs<Q>> on ItemDef ───────────────────────────────────────

struct CountTag {};
struct Counter : CountTag {
  template<typename O>
  struct Part : O {
    using Base = O; using Base::Base;
    int count = 0;
    bool act() { ++count; return Base::act(); }
  };
};

void test_find_tagged() {
  ItemDef<Counter> item;
  auto& c = hapi::find<TagIs<CountTag>>(item);
  assert(c.count == 0);
  item.act();
  item.act();
  assert(c.count == 2);
  cout << "PASS test_find_tagged\n";
}

// ─── Test 6: query tag detection ─────────────────────────────────────────────

void test_query_tag() {
  static_assert(query<TagIs<CountTag>, ItemDef<Counter>::Types>,
    "CountTag should be detectable in ItemDef types");
  static_assert(!query<TagIs<CountTag>, ItemDef<Default<Int,0>>::Types>,
    "CountTag should not appear without Counter");
  cout << "PASS test_query_tag\n";
}

// ─────────────────────────────────────────────────────────────────────────────

#ifdef ARDUINO
  void setup() {
    Serial.begin(115200);
    while (!Serial);
    test_action();
    test_endis();
    test_hidden();
    test_decor();
    test_find_tagged();
    test_query_tag();
  }
  void loop() {}
#else
  int main() {
    test_action();
    test_endis();
    test_hidden();
    test_decor();
    test_find_tagged();
    test_query_tag();
    cout << "\nAll tests passed.\n";
    return 0;
  }
#endif
