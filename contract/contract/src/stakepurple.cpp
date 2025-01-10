#include "stakepurple.hpp"
/**
 * @title Set Staking Parameters
 * @abi action setparams
 * @details Sets or updates the staking parameters for a specific token
 * 
 * @param user - The account setting the parameters
 * @param token_symbol - The token symbol for staking
 * @param token_contract - The token's contract account
 * @param reward_token_symbol - The reward token symbol
 * @param reward_token_contract - The reward token's contract
 * @param unstake_period - Time required before unstaking (in seconds)
 * @param reward_rate - Daily reward rate (multiplied by 100)
 * 
 * @pre Requires contract authority
 * @pre token_symbol must be valid
 * @pre token_contract must be valid account
 * @pre unstake_period must be > 0
 * @pre reward_rate must be > 0
 */
ACTION stakepurple::setparams(const name& user, const symbol& token_symbol, const name& token_contract ,const symbol& reward_token_symbol, const name& reward_token_contract, const uint32_t& unstake_period, const uint32_t& reward_rate = 100) {
    require_auth(get_self());

    check(unstake_period > 0, "🔯 Unstake period must be positive");
    check(reward_rate > 0, "🔯 Reward rate must be positive");
    check(token_symbol.is_valid(), "🔯 Invalid token symbol");
    check(reward_token_symbol.is_valid(), "🔯 Invalid reward token symbol");

    config_t config_tbl(get_self(), get_self().value);
    
    // Check if reward token is already configured as a stakeable token
    auto existing_token = config_tbl.find(reward_token_symbol.code().raw());
    check(existing_token == config_tbl.end(), "🔯 Reward token cannot be an existing stakeable token");

    auto config_itr = config_tbl.find(token_symbol.code().raw());

    if (config_itr == config_tbl.end()) {
        config_tbl.emplace(get_self(), [&](auto& row) {
            row.creator = user;
            row.token_contract = token_contract;
            row.token_symbol = token_symbol;
            row.reward_token_contract = reward_token_contract;
            row.reward_token_symbol = reward_token_symbol;
            row.unstake_period = unstake_period;
            row.reward_rate = reward_rate;
        });
    } else {
        config_tbl.modify(config_itr, get_self(), [&](auto& row) {
            row.token_contract = token_contract;
            row.token_symbol = token_symbol;
            row.reward_token_contract = reward_token_contract;
            row.reward_token_symbol = reward_token_symbol;
            row.unstake_period = unstake_period;
            row.reward_rate = reward_rate;
        });
    }
}

/**
 * @title Unstake Tokens
 * @abi action unstake
 * @details Allows users to unstake their tokens after the required lock period
 * 
 * @param user - The account unstaking tokens
 * @param quantity - The amount of tokens to unstake
 * 
 * @pre Requires user authority
 * @pre User must have staked tokens
 * @pre Lock period must have elapsed
 * @pre Quantity must be > 0
 * @pre Quantity must not exceed staked amount
 */
ACTION stakepurple::unstake(const name& user, const asset& quantity) {

    check(is_account(user), "🔯 Invalid account");
    check(quantity.is_valid(), "🔯 Invalid quantity");
    check(quantity.symbol.is_valid(), "🔯 Invalid symbol");

    require_auth(user);

    // Verify token configuration exists and get it
    config_t config_tbl(get_self(), get_self().value);
    auto config_itr = config_tbl.require_find(quantity.symbol.code().raw(), 
        ("🔯 Token configuration not found for symbol: " + quantity.symbol.code().to_string()).c_str());

    // Only process claims if the token is not paused
    if (!config_itr->is_paused) {
        process_claim(user, false);
    }

    // Initialize stakes table in user's scope
    stake_t stake_tbl(get_self(), user.value);
    auto stake_itr = stake_tbl.require_find(quantity.symbol.code().raw(),
        ("🔯 No staked tokens found for " + user.to_string() + " with symbol " + quantity.symbol.code().to_string()).c_str());

    // Validate staked amount
    check(stake_itr->staked_amount.symbol == quantity.symbol, "🔯 Symbol mismatch");
    check(stake_itr->staked_amount.amount >= quantity.amount, 
        ("🔯 You currently have only " + stake_itr->staked_amount.to_string() + " staked.").c_str());

    // Get current block time
    time_point_sec current = current_time_point();
    
    // Calculate time difference in seconds
    uint32_t elapsed_time = current.sec_since_epoch() - stake_itr->last_claim.sec_since_epoch();
    
    // If not enough time has passed
    if (elapsed_time < config_itr->unstake_period) {
        uint32_t remaining = config_itr->unstake_period - elapsed_time;
        uint32_t hours = remaining / 3600;
        uint32_t minutes = (remaining % 3600) / 60;
        string error_msg = "🔯 You can unstake in " + std::to_string(hours) + " hours and " + std::to_string(minutes) + " minutes";
        check(false, error_msg);
    }

    // Update or erase stake
    if ((stake_itr->staked_amount.amount - quantity.amount) == 0) {
        stake_tbl.erase(stake_itr);
    } else {
        stake_tbl.modify(stake_itr, get_self(), [&](auto& row) {
            row.staked_amount -= quantity;
            check(row.staked_amount.amount > 0, "🔯 Staked amount must be positive");
        });
    }

    // Send tokens back to user
    action(
        permission_level{get_self(), "active"_n},
        config_itr->token_contract,
        "transfer"_n,
        std::make_tuple(get_self(), user, quantity, std::string("🔯 Here's your PURPLE back 🍄"))
    ).send();
}

/**
 * @title Claim Staking Rewards
 * @abi action claim
 * @details Allows users to claim rewards from their staked tokens
 * 
 * @param user - The account claiming rewards
 * 
 * @pre Requires user authority
 * @pre User must have staked tokens
 * @pre 12 hours must have passed since last claim
 * @pre Contract must not be paused
 */
ACTION stakepurple::claim(const name& user) {
    // Allow either the user or the contract to perform the claim
    if (has_auth(user)) {
        require_auth(user);
    } else {
        require_auth(get_self());
    }

    process_claim(user);
}

/**
 * @title Token Transfer Handler
 * @details Handles incoming token transfers for staking
 * 
 * @param from - The account sending tokens
 * @param to - The receiving account (must be contract)
 * @param quantity - The amount of tokens
 * @param memo - Transfer memo
 * 
 * @pre Transfer must be to contract
 * @pre Token must be configured
 * @pre Contract must not be paused
 * @pre Quantity must be > 0
 */
[[eosio::on_notify("*::transfer")]]
void stakepurple::on_transfer(name from, name to, asset quantity, std::string memo) {
    if (from == get_self()) return;

    check(quantity.symbol.is_valid(), "Invalid symbol");

    // Handle "for:" memo to stake for another account
    if (memo.substr(0, 4) == "for:" || memo.substr(0, 5) == "for: ") {
        string account_str = memo.substr(memo.find(":") + 1);
        // Trim leading spaces
        account_str.erase(0, account_str.find_first_not_of(" "));
        // Trim trailing spaces
        account_str.erase(account_str.find_last_not_of(" ") + 1);
        
        // Verify the account name is valid
        check(account_str.length() <= 12, "🔯 Invalid account name length in memo");
        from = name(account_str);
        check(is_account(from), "🔯 Account specified in memo does not exist");
    }

    config_t config_tbl(get_self(), get_self().value);
    auto by_reward = config_tbl.get_index<"byreward"_n>();
    
    // Check if token is stakeable
    auto primary_itr = config_tbl.find(quantity.symbol.code().raw());
    bool is_stakeable = (primary_itr != config_tbl.end() && 
                        get_first_receiver() == primary_itr->token_contract);
    
    /*/ Check if token is reward token
    auto reward_itr = by_reward.find(quantity.symbol.code().raw());
    bool is_reward = (reward_itr != by_reward.end() && 
                     get_first_receiver() == reward_itr->reward_token_contract);
    
    check(is_stakeable || is_reward, "Token " + get_first_receiver().to_string() + "::" + quantity.symbol.code().to_string() + " isn't configured as a reward or stakable token");
    */

    if (!is_stakeable) {
        return;
    }

    check(quantity.amount > 1, "🔯 Must transfer more than one token.");

    stake_t stake_tbl(get_self(), from.value);
    auto stake_itr = stake_tbl.find(quantity.symbol.code().raw());

    if (stake_itr == stake_tbl.end()) {
        stake_tbl.emplace(get_self(), [&](auto& row) {
            row.staked_amount = quantity;
            row.last_claim = time_point_sec(current_time_point());
        });
    } else {
        process_claim(from, false);  // Process claim before adding new stake
        stake_tbl.modify(stake_itr, get_self(), [&](auto& row) {
            row.staked_amount += quantity;
        });
    }
}

/**
 * @title Emergency Pause
 * @abi action pause
 * @details Allows contract owner to pause/unpause contract functionality
 * 
 * @param should_pause - True to pause, false to unpause
 * 
 * @pre Requires contract authority
 */
ACTION stakepurple::pause(bool should_pause, const name token_contract, const symbol token_symbol) {
    require_auth(get_self());
    
    config_t config_tbl(get_self(), get_self().value);
    
    // If token_contract is empty name or token_symbol is empty/ALL, pause all tokens
    if (token_contract == name() || token_symbol.code().to_string() == "ALL" || token_symbol.code().raw() == 0) {
        auto config_itr = config_tbl.begin();
        while(config_itr != config_tbl.end()) {
            config_tbl.modify(config_itr, get_self(), [&](auto& row) {
                row.is_paused = should_pause;
            });
            config_itr++;
        }
        return;
    }

    // Find and update specific token configuration
    auto config_itr = config_tbl.find(token_symbol.code().raw());
    check(config_itr != config_tbl.end(), "🔯 Token configuration not found");
    check(config_itr->token_contract == token_contract, 
          "🔯 Token contract does not match configuration");

    config_tbl.modify(config_itr, get_self(), [&](auto& row) {
        row.is_paused = should_pause;
    });
}

void stakepurple::process_claim(const name& user, bool reset_unstake_period) {
    stake_t stake_tbl(get_self(), user.value);
    // We need to check all staked tokens for this user
    check(stake_tbl.begin() != stake_tbl.end(), "🔯 No staked tokens found.");

    config_t config_tbl(get_self(), get_self().value);
    
    // Check if any of user's staked tokens are paused
    for(auto stake_itr = stake_tbl.begin(); stake_itr != stake_tbl.end(); stake_itr++) {
        auto config_itr = config_tbl.find(stake_itr->staked_amount.symbol.code().raw());
        check(config_itr != config_tbl.end(), "🔯 Token configuration not found.");
        check(!config_itr->is_paused, "🔯 Claims are paused for " + stake_itr->staked_amount.symbol.code().to_string() + 
              " unstake your " + stake_itr->staked_amount.symbol.code().to_string() + " then claim.");
    }

    // Process each staked token
    for(auto stake_itr = stake_tbl.begin(); stake_itr != stake_tbl.end(); stake_itr++) {
        auto config_itr = config_tbl.find(stake_itr->staked_amount.symbol.code().raw());
        check(config_itr != config_tbl.end(), "🔯 Token configuration not found.");
        if (config_itr->is_paused) {
            continue;
        }

        // New check for minimum time between claims
        uint32_t time_since_last_claim = current_time_point().sec_since_epoch() - stake_itr->last_claim.sec_since_epoch();
        //check(time_since_last_claim >= 43200, "🔯 You must wait at least 12 hours between claims."); // FLAG CHANGE THIS BACK TO 12 HOURS
        check(time_since_last_claim >= 200, "🔯 Douglas, you must change this back."); // FLAG CHANGE THIS BACK TO 12 HOURS
        // Calculate the user's level based on the Tetrahedral series
        uint64_t staked_amount = floor(stake_itr->staked_amount.amount / pow(10, stake_itr->staked_amount.symbol.precision()));
         //check(false, to_string(staked_amount) + " .. " + to_string(pow(10, stake_itr->staked_amount.symbol.precision())) + " .. " + to_string(stake_itr->staked_amount.amount) + " 🔯.");
        //staked_amount
        size_t level = 0;
        for (size_t i = 0; i < TETRAHEDRAL.size(); ++i) {
            if (staked_amount < TETRAHEDRAL[i]) {
                level = i;
                break;
            }
        }

        // Calculate amount needed for next level
        uint64_t next_level_amount = TETRAHEDRAL[level] - staked_amount;

        // Determine the reward rate using the Triangular series
        uint32_t lvl = TRIANGULAR[level];
        uint32_t reward_rate = config_itr->reward_rate + level; // EXPLAIN reward_rate should be the # of reward tokens per day * 100

        // Calculate 1 BLUX per day reward
        uint32_t days_passed = (current_time_point().sec_since_epoch() - stake_itr->last_claim.sec_since_epoch()) / (24 * 3600);
        asset reward = asset(static_cast<int64_t>(staked_amount * (reward_rate / 100) * days_passed), config_itr->reward_token_symbol);
        // Add bonus to user level
        reward += asset(lvl, config_itr->reward_token_symbol);

        if (reset_unstake_period) {
            // Update last_claim time only if reset is desired
            stake_tbl.modify(stake_itr, get_self(), [&](auto& row) {
                row.last_claim = time_point_sec(current_time_point());
            });
        }

        uint32_t hours_passed = time_since_last_claim / 3600;
        uint32_t minutes_passed = (time_since_last_claim % 3600) / 60;
        std::string memo = "🔯 PURPLE 🔷 Rewards: ⏳ " + to_string(days_passed) + " days, " + to_string(hours_passed) + " hours, " + to_string(minutes_passed) + "m | ";
        memo += "🔒: "+to_string(staked_amount) +"🔯 @ " + to_string(reward_rate) + "% | Bonus: " + to_string(lvl) + " 🔷 | Next Level: +" + to_string(next_level_amount) + " PURPLE staked 🍄";

        // Send rewards to user

        //check(false, memo);
        action(
            permission_level{get_self(), name("active")},
            config_itr->reward_token_contract,
            name("transfer"),
            std::make_tuple(get_self(), user, reward, memo)
        ).send();
    }
}