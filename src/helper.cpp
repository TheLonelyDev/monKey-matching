struct IncGenerator
{
  int current_;
  IncGenerator(int start) : current_(start) {}
  int operator()() { return current_++; }
};

std::vector<uint16_t> monkeygame::generate_set_with_mints()
{
  auto options = get_config().get().params;
  uint16_t min = options.min_mint;
  uint16_t max = options.max_mint;
  uint64_t size = max - min + 1;

  std::vector<uint16_t> set(size);
  IncGenerator generator(min);
  std::generate(set.begin(), set.end(), generator);

  return set;
}

uint64_t monkeygame::get_set_size(uint64_t completed_sets)
{
  return pow(get_config().get().params.new_game_base, completed_sets + 1);
}

void monkeygame::init_user(eosio::name owner)
{
  auto users = get_users();

  users.emplace(owner, [&](auto &row)
                { row.owner = owner; });
}

random monkeygame::random_generator(std::string data)
{
  std::string salt = get_config().get().salt.append(data);

  return random(eosio::sha256(salt.c_str(), salt.length()));
}

void monkeygame::maintenace_check()
{
  eosio::check(!get_config().get().maintenance, "Game is in maintenance");
}

bool monkeygame::is_frozen(eosio::time_point time, uint64_t freeze_time)
{
  return eosio::current_time_point() >= time + eosio::microseconds(freeze_time);
}
