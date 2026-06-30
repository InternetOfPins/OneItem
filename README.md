# OneItem

**HAPI Compatibility:** Works with new Check/Apply/ApplyPack API (2026-Q2)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

HAPI-based item composition library. Defines the `ItemDef` building block and a set of composable components for constructing interactive data items with zero runtime overhead.

Part of the [InternetOfPins](https://github.com/InternetOfPins) ecosystem.

---

## Dependencies

- [hapi](https://github.com/InternetOfPins/hapi) — open chain derivation core
- [oneData](https://github.com/InternetOfPins/oneData) — data component primitives
- [oneOutput](https://github.com/InternetOfPins/oneOutput) — output channel abstraction

---

## Concepts

### Item

An item is a composable, self-contained unit combining a data value, a display representation, and an interaction contract. Items are built by declaring a chain of components over `ItemDef`:

```cpp
using NL = StaticChar<'\n'>;
constexpr const char* label{"year:"};

ItemDef<
  Alias<StaticText<&label>, StaticInt<2026>>,
  Action<year_act>,
  NL
> year{};
```

`ItemDef` closes the chain over `ItemAPI`, which provides fallback implementations for the full item interface: `get`/`set`, `print`, `act`, `enabled`/`enable`, `up`/`down`, `depth`, and `sync`.

### API dimensions

Each component in the chain participates selectively in the item's API surface. The key dimensions are:

| Dimension | API calls | Default fallback |
|-----------|-----------|-----------------|
| Data | `get()` / `set()` | compile error if absent |
| Display | `print(out)` | empty body, DCE |
| Interaction | `act()` | `false` |
| Navigation | `up()` / `down()` | `false` |
| Lifecycle | `enabled()` / `sync()` | `true` / empty body, DCE |
| Tree | `depth()` | `1` |

---

## Components

### `Alias<Title, Value>`

Presents a value with a separate display representation. `Title` participates in `print` but not in `get`/`set`. `Value` owns the data but is invisible to output. The caller decides how and whether to surface the value.

```cpp
using Yes = Alias<Text, StaticBool<true>>;
using No  = Alias<Text, StaticBool<false>>;

ItemDef<Yes> yes{"yes"};
ItemDef<No>  no{"no"};

yes.print(out);          // prints "yes"
out.put(yes.get());      // puts true
```

`Alias` is defined as:

```cpp
template<typename Title, typename Value>
using Alias = Chain<Decor<Title>, Hidden<Value>>;
```

---

### `Action<func>`

Associates a compile-time function reference with `act()`. The function is a zero-storage template parameter — no pointer stored in the object.

```cpp
bool year_act() { cout << "activated" << endl; return false; }

ItemDef<Action<year_act>> item{};
item.act(); // calls year_act()
```

`act()` return value convention: `false` = event not consumed, `true` = consumed. Interpreted by the enclosing navigation context.

---

### `Hidden<Components...>`

Includes a group of components in the chain — contributing their storage and data API — while excluding them from `print`. The hidden group participates in `get`/`set` but is invisible to output.

---

### `Decor<Components...>`

Includes a group of components in the chain — contributing their `print` output — while excluding them from `get`/`set`. The decorated group is visible but not the item's data value. `get`/`set` pass through to the chain tail.

`Decor` and `Hidden` are orthogonal axes of participation — decoration is visible but not data, hidden components are data but not visible. `Alias` composes both.

---

### `EnDis<bool>`

Attaches an enabled/disabled state to an item, with the boolean stored as a hidden `Bool` component. Initial state is the template parameter.

```cpp
ItemDef<EnDis<true>, StaticInt<0>> item{};
```

`enable()`/`enabled()` API is under active development.

---

### `Id<int>`

Attaches a compile-time integer identifier to an item without contributing storage or behaviour. Used for static discrimination between items of the same type.

```cpp
ItemDef<Id<3>, StaticInt<0>> item{};
```

---

## Stream output

`ItemDef` supports `operator<<` for any output type satisfying the `oneOutput` interface:

```cpp
OutDef<ConsoleOut> out;
year.print(out);
out << year;  // equivalent
```

---

## Design notes

### Selective API occlusion

`Hidden` and `Decor` work by method shadowing in the linear inheritance chain, not virtual dispatch. A `Hidden<X>` layer inherits `X`'s storage and construction, then re-exposes `print` as a pass-through to the tail, making `X`'s own `print` unreachable by construction. No runtime cost.

### Zero-overhead guarantee

`ItemDef` produces a single flat object. The chain is collapsed at compile time into a contiguous block of exactly the storage declared by its components. No vtable, no heap allocation, no internal pointers. Unused fallback methods have empty bodies and are eliminated by DCE; storage elimination for unused subobjects is toolchain-dependent.

### Virtual interface

`IItem` (forthcoming) will provide a thin virtual facade over `ItemDef` for contexts requiring runtime polymorphism — heterogeneous item containers, navigation trees. The concrete `ItemDef` object behind an `IItem*` remains flat and densely packed regardless.

---

## Example

```cpp
#include <hapi/hapi.h>
using namespace hapi;

#include <oneData/oneData.h>
using namespace oneData;

#include <oneOutput/oneOutput.h>
using namespace oneOutput;

#include <oneItem/oneItem.h>
using namespace oneItem;

#include <iostream>
using namespace std;

bool year_act() {
  cout << "year_act function called()" << endl;
  return false;
}

using NL = StaticChar<'\n'>;
constexpr const char* label{"year:"};

ItemDef<
  Alias<StaticText<&label>, StaticInt<2026>>,
  Action<year_act>,
  NL
> year{};

OutDef<ConsoleOut> out;

int main() {
  cout << boolalpha;
  year.act();                  // triggers action function
  year.print(out);             // prints label and newline — value not printed
  cout << year.get() << endl;  // surfaces the value explicitly
}
```

Output:
```
year_act function called()
year:
2026
```

`print` outputs the title and `NL` — the value is not printed. It is surfaced explicitly via `get()`, giving the caller full control over how and whether it is displayed.

---

## Roadmap

- `EnDis` enable/disable API completion
- `IItem` virtual facade for heterogeneous containers
- Navigation integration (`INav`, `up`/`down` semantics)
- `depth()` for tree items and submenus