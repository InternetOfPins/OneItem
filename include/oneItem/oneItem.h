/**
 * @file oneItem.h
 * @author Rui Azevedo (neu-rah) (ruihfazevedo@gmail.com)
 * @brief HAPI based Item API
*/

#include <hapi/hapi.h>
#include <oneData/oneData.h>

namespace oneItem {

  using Depth=unsigned int;

  // item API+Def ------------------------------------------------------------------

  /// @brief hapi item, extending OneData
  /// @tparam Cfg : optional...
  template<typename Cfg=hapi::Nil>
  struct ItemAPI:oneData::DataAPI<Cfg>{
    //DataAPI is also providing some fallback
    using Base=oneData::DataAPI<Cfg>;
    template<typename Out>
    static constexpr Depth depth() {return 1;}
    static constexpr bool enabled() {return true;}
    static constexpr void enable(bool=true) {}
    using Base::sync;
    template<typename Out> static constexpr void sync(Out&) {}
    static constexpr bool up() {return false;}
    static constexpr bool down() {return false;}
    static constexpr bool act() {return false;}
  };

  /// @brief define composition over ItemAPI
  /// @tparam ...OO 
  template<typename... OO>
  struct ItemDef:APIOf<ItemAPI<>,OO...>{
    using Base=APIOf<ItemAPI<>,OO...>;
    using Base::Base;
    using Types=Chain<OO...>;
    static constexpr const size_t size{sizeof...(OO)};
  };

  //stream output for items --
  template<typename Out,typename... OO>
  Out& operator<<(Out& out,const ItemDef<OO...>& o) {o.print(out);return out;}

};//namespace oneItem

//---------------------------------------------------------------------------------------------
template<int id> struct Id {template<typename O> using Part=O;};

using ActionFunc=bool(&)();

//associate an action function
// api: act() will call it
template<ActionFunc action>
struct Action {
  template<typename I>
  struct Part:I {
    using Base=I;
    using Base::Base;
    static constexpr bool act() {return action();}
  };
};

//hide a group of components by excluding them from the print api chain
template<typename... II>
struct Hidden {
  template<typename I>
  struct Part:Chain<II...>::template Part<I> {
    using Base=typename Chain<II...>::template Part<I>;
    using Base::Base;
    template<typename Out>
    void print(Out& out) const noexcept {I::print(out);}
  };
};

//mark a  group of components as Decor
//by excluding them of the get/set api calls
template<typename... II>
struct Decor {
  template<typename I>
  struct Part:Chain<II...>::template Part<I> {
    using Base=typename Chain<II...>::template Part<I>;
    using Type=typename I::Type;
    using Base::Base;
    static Type get() noexcept {return I::get();}
    static void set(typename I::Type v) noexcept {I::set(v);}
  };
};

//
template<bool ens>
struct EnDis {
  template<typename I>
  struct Part:Chain<Default<bool,ens>,Hidden<Bool>>::template Part<I> {
    using Base=typename Chain<Default<bool,ens>,Hidden<Bool>>::template Part<I>;
    using Base::Base;
    void enable(bool e=true) {Base::set(e);}
    bool enabled() {return Base::get();}
  };
};

/// @brief alternative representation for a value
/// @tparam Title title type
/// @tparam Value value type
template<typename Title,typename Value>
using Alias=Chain<Decor<Title>,Hidden<Value>>;


