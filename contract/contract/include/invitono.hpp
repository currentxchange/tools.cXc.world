#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <eosio/permission.hpp> 

using namespace eosio;
using std::string;

// === Invitono Contract === //
// --- Multi-level referral management system --- //

CONTRACT invitono : public contract {
public:
  using contract::contract;

  // === User Actions === //
  // --- Core user interactions --- //

  // - Register new user with invite code
  ACTION registeruser(name user, name inviter);

  // - Claim rewards based on score
  ACTION claimreward(name user);

  // - Admin configuration management
  ACTION setconfig(
      name admin, 
      uint32_t min_age_days, 
      uint32_t rate_seconds, 
      bool enabled,
      uint16_t max_depth,
      uint16_t multiplier,
      name token_contract,
      symbol reward_symbol,
      uint32_t reward_rate
  );

  // - Development utility action
  ACTION deleteuser(name user);

  // === Adopter Table === //
  // --- Tracks registered users and referral statistics --- //

  /*/
  Tracks each registered user and their referral stats
  /*/
  TABLE adopter {
    name        account;          // - WAX account name
    name        invitedby;        // - Referrer account
    uint32_t    lastupdated;      // - Last score update timestamp
    uint32_t    score = 0;        // - Current referral score
    bool        claimed = false;  // - Reward claim status

    uint64_t primary_key() const { return account.value; }
    uint64_t by_score() const { return static_cast<uint64_t>(UINT32_MAX - score); } // - Sort descending
  };

  using adopters_table = multi_index<"adopters"_n, adopter,
    indexed_by<"byscore"_n, const_mem_fun<adopter, uint64_t, &adopter::by_score>>
  >;

  // === Config Singleton === //
  // --- Contract configuration values --- //

  /*/
  Stores contract-wide configuration parameters
  /*/
  TABLE config {
    uint32_t min_account_age_days = 30; // - Minimum registration age (days)
    uint32_t invite_rate_seconds = 3600; // - Score update cooldown (seconds)
    bool     enabled = true;             // - Contract operational status
    name     admin;                      // - Admin account
    uint16_t max_referral_depth = 5;     // - Maximum referral levels
    uint16_t multiplier = 100;           // - Score multiplier (100 = 1.0x)
    name     token_contract;             // - Token contract account
    symbol   reward_symbol;              // - Reward token symbol
    uint32_t reward_rate = 100;          // - Tokens per point (100 = 1.00 token)
  };

  using config_table = singleton<"config"_n, config>;

  // === Stats Singleton === //
  // --- Global contract statistics --- //

  /*/
  Tracks global referral and user statistics
  /*/
  TABLE stats {
    uint64_t total_referrals = 0; // - Total referral connections
    uint64_t total_users = 0;     // - Total registered users
    name     last_registered;     // - Most recent registration
  };

  using stats_table = singleton<"stats"_n, stats>;

private:
  // === Internal Functions === //
  // --- Core business logic --- //

  // - Updates scores for inviter and their upline
  void update_scores(name direct_inviter);

  // === Constants === //
  // --- Tetrahedral series values --- //

  // - Pre-calculated tetrahedral series values
  const std::vector<uint32_t> TETRAHEDRAL = {1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364, 455, 560, 680, 816, 969, 1140, 1330, 1540, 1771, 2024, 2300, 2600, 999999999};

  // - Calculates position in tetrahedral series
  uint32_t calculate_tetrahedral_position(uint32_t score) {
    // - Find largest n where T(n) <= score
    for (size_t i = 0; i < TETRAHEDRAL.size(); i++) {
      if (TETRAHEDRAL[i] > score) {
        return i; // - Return index where score exceeded
      }
    }
    return TETRAHEDRAL.size() - 1; // - Return last position for large scores
  }//END calculate_tetrahedral_position()
};