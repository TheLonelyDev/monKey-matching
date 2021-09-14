#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>

#include <random.hpp>
#include <atomicassets-interface.hpp>

CONTRACT matchamonkey : public eosio::contract
{
public:
    using eosio::contract::contract;

    struct cfg_params
    {
        uint16_t new_game_base = 2;
        uint16_t reward_cap = 6;
        uint16_t min_mint = 6;
        uint16_t max_mint = 105;
        uint16_t mint_offset = 1;
        int64_t freeze_time = 86400000;
        std::string reward_memo = "Set completion reward";
        std::vector<eosio::name> whitelist = {};
    };

    struct NFT
    {
        uint64_t asset_id;
        uint16_t index;
    };

    struct MINT
    {
        uint64_t asset_id;
        uint16_t mint;
    };

    struct [[eosio::table("mints")]] _mint_asset
    {
        uint64_t index;
        uint64_t template_id;
        std::vector<MINT> mints;

        uint64_t primary_key() const { return index; };
        uint64_t secondary_key_0() const { return template_id; };
    };
    typedef eosio::multi_index<
        eosio::name("mints"), _mint_asset,
        eosio::indexed_by<"template"_n, const_mem_fun<_mint_asset, uint64_t, &_mint_asset::secondary_key_0>>>
        _mints;

    [[eosio::action]] void newgame(eosio::name owner);
    [[eosio::action]] void verify(eosio::name owner, std::vector<NFT> owned_assets);
    [[eosio::action]] void complete(eosio::name owner);
    [[eosio::action]] void unfreeze(eosio::name owner, uint64_t asset_id);
    [[eosio::action]] void unfreezeall(eosio::name owner);

    [[eosio::action]] void init();
    [[eosio::action]] void destruct();
    [[eosio::action]] void maintenance(bool maintenance);
    [[eosio::action]] void setsalt(std::string salt);
    [[eosio::action]] void setparams(cfg_params params);

    [[eosio::action]] void rmmint(uint64_t index);
    [[eosio::action]] void addmint(uint64_t index, uint64_t template_id, std::vector<MINT> new_mints);

    [[eosio::action]] void rmreward(uint64_t completions);
    [[eosio::action]] void addreward(uint64_t completions, eosio::name contract, eosio::asset amount);

    [[eosio::action]] void resetuser(eosio::name usser);

private:
    struct [[eosio::table("config")]] _config_entity
    {
        bool maintenance = true;
        std::string salt;
        cfg_params params;
    };
    typedef eosio::singleton<eosio::name("config"), _config_entity> _config;

    struct [[eosio::table("games")]] _game_entity
    {
        eosio::name owner;
        std::vector<uint16_t> to_collect = {};
        std::vector<uint16_t> collected = {};

        uint64_t primary_key() const { return owner.value; };
    };
    typedef eosio::multi_index<eosio::name("games"), _game_entity> _games;

    struct [[eosio::table("users")]] _user_entity
    {
        eosio::name owner;
        uint64_t completed_sets = 0;

        uint64_t primary_key() const { return owner.value; };
        uint64_t secondary_key_0() const { return completed_sets; };
    };
    typedef eosio::multi_index<eosio::name("users"), _user_entity,
        eosio::indexed_by<"completed"_n, const_mem_fun<_user_entity, uint64_t, &_user_entity::secondary_key_0>>> _users;

    struct [[eosio::table("rewards")]] _reward_entity
    {
        uint64_t completions;

        eosio::name contract;
        eosio::asset amount;

        uint64_t primary_key() const { return completions; };
    };
    typedef eosio::multi_index<eosio::name("rewards"), _reward_entity> _rewards;

    struct [[eosio::table("frozen")]] _frozen_asset_entity
    {
        uint64_t asset_id;
        eosio::name owner;
        eosio::time_point time;

        uint64_t primary_key() const { return asset_id; };
        uint64_t secondary_key_0() const { return owner.value; };
    };
    typedef eosio::multi_index<eosio::name("frozen"), _frozen_asset_entity,
                               eosio::indexed_by<"owner"_n, const_mem_fun<_frozen_asset_entity, uint64_t, &_frozen_asset_entity::secondary_key_0>>>
        _frozen_assets;

    _games get_games()
    {
        return _games(get_self(), get_self().value);
    }

    _users get_users()
    {
        return _users(get_self(), get_self().value);
    }

    _config get_config()
    {
        return _config(get_self(), get_self().value);
    }

    _rewards get_rewards()
    {
        return _rewards(get_self(), get_self().value);
    }

    _mints get_mints()
    {
        return _mints(get_self(), get_self().value);
    }

    _frozen_assets get_frozen_assets()
    {
        return _frozen_assets(get_self(), get_self().value);
    }

    void maintenace_check(eosio::name user);

    std::vector<uint16_t> generate_set_with_mints();

    bool is_frozen(eosio::time_point time, int64_t freeze_time);
};

EOSIO_DISPATCH(matchamonkey,
               (newgame)(verify)(complete)(unfreeze)(unfreezeall)

                   (init)(destruct)(maintenance)(setsalt)(setparams)(resetuser)

                       (rmmint)(addmint)

                           (rmreward)(addreward));
