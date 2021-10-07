/*
    Initialize the contract configuration singleton, removes the previous configuration first.

    @auth self
*/
[[eosio::action]] void matchamonkey::init()
{
    require_auth(get_self());
    get_config().remove();

    get_config().set(_config_entity{}, get_self());
}

/*
    Delete the contract configuration singleton.

    @auth self
*/
[[eosio::action]] void matchamonkey::destruct()
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
[[eosio::action]] void matchamonkey::maintenance(bool maintenance)
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
[[eosio::action]] void matchamonkey::setparams(cfg_params params)
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
[[eosio::action]] void matchamonkey::setsalt(std::string salt)
{
    require_auth(get_self());

    auto config = get_config();
    auto new_config = config.get();

    new_config.salt = salt;

    config.set(new_config, get_self());
}

/*
    Remove the row for a sepcific amount of completions.

    @auth self
*/
[[eosio::action]] void matchamonkey::rmreward(uint64_t completions)
{
    require_auth(get_self());

    auto rewards = get_rewards();
    auto entity = rewards.require_find(completions, "Entity does not exist");

    rewards.erase(entity);
}

/*
    Add a reward for a specific amount of completions.

    Updates the values if the completions already exist.

    @auth self
*/
[[eosio::action]] void matchamonkey::addreward(uint64_t completions, eosio::name contract, eosio::asset amount)
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

/*
    Removes a mint index.

    @auth self
*/
[[eosio::action]] void matchamonkey::rmmint(uint64_t index)
{
    require_auth(get_self());

    auto mints = get_mints();
    auto entity = mints.require_find(index, "Entity does not exist");

    mints.erase(entity);
}

/*
    Creates or updates a mint index.

    @auth self
*/
[[eosio::action]] void matchamonkey::addmint(uint64_t index, uint64_t template_id, std::vector<MINT> new_mints)
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

/*
    Resets the account of an user, both user account and current game.

    @auth self
*/
[[eosio::action]] void matchamonkey::resetuser(eosio::name user)
{
    require_auth(get_self());

    auto games = get_games();
    auto game_itr = games.find(user.value);

    if (game_itr != games.end())
    {
        games.erase(game_itr);
    }

    auto users = get_users();
    auto user_itr = users.find(user.value);

    if (user_itr != users.end())
    {
        users.erase(user_itr);
    }
}