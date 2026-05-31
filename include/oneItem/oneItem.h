/**
 * @file oneItem.h
 * @author Rui Azevedo (neu-rah) (ruihfazevedo@gmail.com)
 * @brief Item API embrio for HAPI components
*/

#include <hapi/hapi.h>

namespace oneItem {
  // item API+Def ------------------------------------------------------------------

  /// @brief minimal printable item 
  /// @tparam Cfg : optional...
  template<typename Cfg=hapi::Nil>
  struct ItemAPI:Cfg{
    template<typename Out>
    static constexpr void print(Out& out) {}
  };

  /// @brief define composition over ItemAPI
  /// @tparam ...OO 
  template<typename... OO>
  struct ItemDef:APIOf<ItemAPI<>,OO...>{
    using Base=APIOf<ItemAPI<>,OO...>;
    using Base::Base;
    using Types=typename Base::Types;
    static constexpr const size_t size{sizeof...(OO)};
  };

  //stream output for items --
  template<typename Out,typename... OO>
  Out& operator<<(Out& out,const ItemDef<OO...>& o) {o.print(out);return out;}

};//namespace oneItem

//rules ItemDef query specialization --
template<typename Q,typename... OO>
constexpr const bool hapi::template query<Q,oneItem::template ItemDef<OO...>>{(hapi::template query<Q,OO>||...)};

