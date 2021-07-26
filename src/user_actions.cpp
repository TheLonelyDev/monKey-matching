/*
    Generate a unique random set.

    @param {ranom} generator - random generator instance with seed
    @param {vector<uint16_t>} input - input vector to use as a source
    @param {int} amount - amount of elements to generate
*/
std::vector<uint16_t> random_set(
    random generator,
    std::vector<uint16_t> &input,
    int amount = 1)
{
  int size = input.size();
  int _amount = std::min(amount, size);

  std::vector<uint16_t> result = {};

  for (int i = 0; i < _amount; i++)
  {
    auto index = generator.next(size - i);
    result.push_back(input[index]);
    input.erase(input.begin() + index);
  }

  return result;
}

/*
    Creates a new game. Randomizes the set using config.salt, caller and amount of completed sets.

    RAM costs are paid by the caller.

    @throws Will throw if the contract is in maintenace
    @throws Will throw if the user has a game running

    @auth caller
*/
[[eosio::action]] void monkeygame::newgame(eosio::name owner)
{
  eosio::require_auth(owner);
  maintenace_check();

  auto games = get_games();
  auto users = get_users();
  auto game = games.find(owner.value);

  eosio::check(game == games.end(), "You already have a game running");

  // Get or create the user
  auto user = users.find(owner.value);

  if (user == users.end())
  {
    init_user(owner);
    user = users.find(owner.value);
  }

  // Init a random_generator based on config.salt, owner and completed sets
  random generator = random_generator(owner.to_string().append("-" + std::to_string(user->completed_sets)));

  // Determine the next mints

  // Generates a vector containing all the possible mints
  std::vector<uint16_t> set = generate_set_with_mints();

  // Randomize
  std::vector<uint16_t> result = random_set(
      generator,
      set,
      get_set_size(user->completed_sets));

  // Create entry in games table
  games.emplace(owner, [&](auto &row)
                {
                  row.owner = owner;
                  row.to_collect.assign(result.begin(), result.end()); // Assigns the generated set
                  row.collected = {};
                });
}

/*
    Verify the collection by fetching the atomicassets mints.

    RAM costs are paid by the caller.

    @throws Will throw if the contract is in maintenace
    @throws Will throw if the user has no game running
    @throws Will throw if the user does not exist

    @auth caller
*/
[[eosio::action]] void monkeygame::verify(eosio::name owner, std::vector<NFT> owned_assets)
{
  eosio::require_auth(owner);
  maintenace_check();

  auto games = get_games();
  auto users = get_users();
  auto mints = get_mints();
  auto config = get_config().get();
  auto game = games.require_find(owner.value, "You have no running game");
  auto user = users.require_find(owner.value, "No user found");

  std::vector<uint16_t> owned = {};
  //std::vector<uint16_t> owned = generate_set_with_mints();

  // Get the owned assets
  auto assets = atomicassets::get_assets(owner);
  std::map<uint16_t, uint64_t> ownedDict;
  for (const NFT &nft : owned_assets)
  {
    assets.require_find(nft.asset_id, "You do not own all assets");
    mints.require_find(nft.index, "Mint index not found");
    unfreeze(nft.asset_id);

    auto entry = mints.get(nft.index);

    auto iterator = std::find_if(entry.mints.begin(), entry.mints.end(), [&id = nft.asset_id](const MINT &mint) -> bool
                                 { return id == mint.asset_id; });

    eosio::check(iterator != entry.mints.end(), "Asset mint number not found");
    owned.push_back(iterator->mint);
    ownedDict[iterator->mint] = iterator->asset_id;
  }

  // TODO: improve
  // Sort the collected & to_collect vector
  std::vector<uint16_t> collected(game->collected);
  std::vector<uint16_t> to_collect(game->to_collect);
  std::sort(to_collect.begin(), to_collect.end());
  std::sort(collected.begin(), collected.end());

  // Get the set difference (to be collected mints)
  std::vector<uint16_t> remainder;
  std::set_difference(to_collect.begin(), to_collect.end(), collected.begin(), collected.end(), std::inserter(remainder, remainder.begin()));

  // Loop over the remaining mints that are required
  for (auto elem : remainder)
  {
    // If the mint is owned, add it to the collected vector
    if (std::find_if(owned.begin(), owned.end(), [&elem = elem, &mint_offset = config.params.mint_offset](const uint16_t &mint) -> bool
                     { return std::abs(int(mint - elem)) <= mint_offset; }) != owned.end())
    {
      collected.push_back(elem);

      // Freeze asset
      get_frozen_assets().emplace(owner, [&](auto &row)
                {
                      row.asset_id = ownedDict[elem];
                      row.owner = owner;
                      row.time = eosio::current_time_point();
                });
    }
  }

  // Update the collected mints
  games.modify(game, owner, [&](auto &row)
               { row.collected.assign(collected.begin(), collected.end()); });
}

/*
    Complete the set, check if the caller collected the correct mints.
    Remove the game table entry and update user.completed_sets.

    RAM costs are paid by the caller.

    @throws Will throw if the contract is in maintenace
    @throws Will throw if the user has no game running
    @throws Will throw if the user does not exist
    @throws Will throw if the set has not been completed

    @auth caller
*/
[[eosio::action]] void monkeygame::complete(eosio::name owner)
{
  eosio::require_auth(owner);
  maintenace_check();

  auto games = get_games();
  auto users = get_users();
  auto game = games.require_find(owner.value, "You have no running game");
  auto user = users.require_find(owner.value, "No user found");

  // Check if the user collected enough mints
  // Vyryn's fancy math request: https://discord.com/channels/733122024448458784/867103030516252713/867125763291086869
  int size = game->to_collect.size();
  int required = (size - ceil(log10(size)));
  eosio::check(game->collected.size() >= required, "You need to collect at least " + std::to_string(required) + " mints");

  // Update the user
  users.modify(user, owner, [&](auto &row)
               { row.completed_sets = user->completed_sets + 1; });

  auto config = get_config().get();
  auto rewards = get_rewards();
  auto reward = rewards.require_find(user->completed_sets > 8 ? config.params.reward_cap : user->completed_sets, "No reward found");

  action(
      permission_level{get_self(), name("active")},
      reward->contract,
      name("transfer"),
      make_tuple(
          get_self(),
          owner,
          reward->amount,
          config.params.reward_memo))
      .send();

  // Release the game entry
  games.erase(game);
}

/*
    Unfreeze an asset if the asset has been frozen for long enough.

    Frozen time set by config options.

    @throws Will throw if the contract is in maintenace
    @throws Will throw if asset canot be unfrozen

    @auth none
*/
[[eosio::action]] void monkeygame::unfreeze(
  uint64_t asset_id
)
{
  maintenace_check();

  auto config = get_config().get();
  auto frozen_assets = get_frozen_assets();
  auto iterator = frozen_assets.find(asset_id);

  // Unfreeze if asset is found
  if (iterator != frozen_assets.end())
  {
    eosio::check(is_frozen(iterator->time, config.params.freeze_time), "Could not unfreeze the asset");

    frozen_assets.erase(iterator);
  }
}

/*
    Unfreeze all expired assets owned by someone.

    Frozen time set by config options.

    @throws Will throw if the contract is in maintenace

    @auth none
*/
[[eosio::action]] void monkeygame::unfreezeall(
  eosio::name owner
)
{
  maintenace_check();

  auto config = get_config().get();
  auto frozen_assets = get_frozen_assets();
  auto owner_index = frozen_assets.get_index<eosio::name("owner")>();
  auto iterator = owner_index.lower_bound(owner.value);

  while (iterator->owner == owner)
  {
    if (is_frozen(iterator->time, config.params.freeze_time))
    {
      iterator = owner_index.erase(iterator);
    }
  }
}