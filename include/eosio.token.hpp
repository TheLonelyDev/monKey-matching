#pragma once

namespace eosiotoken
{
   struct account
   {
      eosio::asset balance;

      uint64_t primary_key() const { return balance.symbol.code().raw(); }
   };

   typedef eosio::multi_index<"accounts"_n, account> accounts;

   static asset get_balance(const eosio::name &token_contract_account, const eosio::name &owner, const eosio::symbol_code &sym_code)
   {
      accounts accountstable(token_contract_account, owner.value);
      const auto &ac = accountstable.get(sym_code.raw());
      return ac.balance;
   }
}
