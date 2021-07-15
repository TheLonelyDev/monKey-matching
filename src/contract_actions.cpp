/*
    Initialize the contract configuration singleton, removes the previous configuration first.

    @auth self
*/
[[eosio::action]] void monkeygame::init()
{
    require_auth(get_self());
    get_config().remove();

    get_config().set(_config_entity{}, get_self());
}

/*
    Delete the contract configuration singleton.

    @auth self
*/
[[eosio::action]] void monkeygame::destruct()
{
    require_auth(get_self());
    get_config().remove();
}

/*
    Set the contract in maintenance mode.

    Maintenance mode prevents users from doing any actions.

    @param {bool} maintenance - maintenance toggle

    @auth self
*/
[[eosio::action]] void monkeygame::maintenance(bool maintenance)
{
    require_auth(get_self());

    auto config = get_config();
    auto new_config = config.get();

    new_config.maintenance = maintenance;

    config.set(new_config, get_self());
}

/*
    Set the contract params. Will overwrite the previous params.

    @param {cfg_params} params - the params object to be set

    @auth self
*/
[[eosio::action]] void monkeygame::setparams(cfg_params params)
{
    require_auth(get_self());

    auto config = get_config();
    auto new_config = config.get();

    new_config.params = params;

    config.set(new_config, get_self());
}

/*
    Set the contract salt. It is important to use a secure salt.

    This salt is used for the PRNG within the contract.

    @param {string} salt - secure salt for PRNG

    @auth self
*/
[[eosio::action]] void monkeygame::setsalt(std::string salt)
{
    require_auth(get_self());

    auto config = get_config();
    auto new_config = config.get();

    new_config.salt = salt;

    config.set(new_config, get_self());
}

[[eosio::action]] void monkeygame::rmwhitelist(uint64_t key)
{
    require_auth(get_self());

    auto whitelist = get_whitelist();
    auto entity = whitelist.require_find(key, "Entity does not exist");

    whitelist.erase(entity);
}

[[eosio::action]] void monkeygame::addwhitelist(eosio::name collection_name, eosio::name schema_name, uint64_t template_id)
{
    require_auth(get_self());

    auto whitelist = get_whitelist();

    whitelist.emplace(get_self(), [&](auto &row)
                      {
                          //row.collection = collection;
                          //row.schema = schema;
                          row.template_id = template_id;
                      });
}

[[eosio::action]] void monkeygame::rmreward(uint64_t completions)
{
    require_auth(get_self());

    auto rewards = get_rewards();
    auto entity = rewards.require_find(completions, "Entity does not exist");

    rewards.erase(entity);
}

[[eosio::action]] void monkeygame::addreward(uint64_t completions, eosio::name contract, eosio::asset amount)
{
    require_auth(get_self());

    auto rewards = get_rewards();

    rewards.emplace(get_self(), [&](auto &row)
                    {
                        row.completions = completions;
                        row.contract = contract;
                        row.amount = amount;
                    });
}
