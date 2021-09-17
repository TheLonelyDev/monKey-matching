struct IncGenerator
{
  int current_;
  IncGenerator(int start) : current_(start) {}
  int operator()() { return current_++; }
};

std::vector<uint16_t> matchamonkey::generate_set_with_mints()
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

void matchamonkey::maintenace_check()
{
  auto config = get_config().get();
  eosio::check(!config.maintenance, "Contract is in maintenance");
}

bool matchamonkey::is_frozen(eosio::time_point time, int64_t freeze_time)
{
  return eosio::current_time_point() >= (time + eosio::milliseconds(freeze_time));
}
