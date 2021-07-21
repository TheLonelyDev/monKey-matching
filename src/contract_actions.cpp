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

    auto iterator = rewards.find(completions);

    if (iterator == rewards.end())
    {
        rewards.emplace(get_self(), [&](auto &row)
                      {
                            row.completions = completions;
                            row.contract = contract;
                            row.amount = amount;
                      });
    }
    else
    {
        rewards.modify(iterator, get_self(), [&](auto &row)
                     {
                            row.completions = completions;
                            row.contract = contract;
                            row.amount = amount;
                     });
    }
}

[[eosio::action]] void monkeygame::rmmint(uint64_t index)
{
    require_auth(get_self());

    auto mints = get_mints();
    auto entity = mints.require_find(index, "Entity does not exist");

    mints.erase(entity);
}

[[eosio::action]] void monkeygame::addmint(uint64_t index, uint64_t template_id, std::vector<MINT> new_mints)
{
    require_auth(get_self());
    auto mints = get_mints();

    auto iterator = mints.find(index);

    if (iterator == mints.end())
    {
        mints.emplace(get_self(), [&](auto &row)
                      {
                          row.index = index;
                          row.template_id = template_id;
                          row.mints.assign(new_mints.begin(), new_mints.end());
                      });
    }
    else
    {
        mints.modify(iterator, get_self(), [&](auto &row)
                     {
                         row.index = index;
                         row.template_id = template_id;
                         row.mints.assign(new_mints.begin(), new_mints.end());
                     });
    }
}