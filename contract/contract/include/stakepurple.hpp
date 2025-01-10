#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <vector>


using namespace std;
using namespace eosio;

const std::vector<uint64_t> TETRAHEDRAL = {1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364, 455, 560, 680, 816, 969, 1140, 1330, 1540, 1771, 2024, 2300, 2600, 2925, 3276, 3654, 4060, 4505, 4990, 5516, 6084, 6725, 7440, 8230, 9096, 99999999999};
const std::vector<uint64_t> TRIANGULAR = {1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190, 210, 231, 253, 276, 300, 325, 351, 378, 406 };

CONTRACT stakepurple : public contract {
public:
    using contract::contract;

    // === Admin Actions === //

    // --- Set staking parameters --- //
    /**
     * @title Set Staking Parameters
     * @abi action setparams
     * @details Sets or updates the staking parameters for a specific token.
     * 
     * **Ricardian Contract:**
     * This action allows the contract owner to set or update staking parameters for a specific token. Only the contract owner can execute this action. Parameters include token symbols, contract accounts, unstake periods, and reward rates.
     * 
     * **Parameters:**
     * - `user`: The account setting the parameters.
     * - `token_symbol`: The token symbol for staking.
     * - `token_contract`: The token's contract account.
     * - `reward_token_symbol`: The reward token symbol.
     * - `reward_token_contract`: The reward token's contract.
     * - `unstake_period`: Time required before unstaking (in seconds).
     * - `reward_rate`: Daily reward rate (multiplied by 100).
     */
    ACTION setparams(const name& user, const symbol& token_symbol, const name& token_contract, const symbol& reward_token_symbol, const name& reward_token_contract, const uint32_t& unstake_period, const uint32_t& reward_rate);

    // === User Actions === //

    // --- Unstake tokens --- //
    ACTION unstake(const name& user, const asset& quantity);

    // --- Claim rewards --- //
    ACTION claim(const name& user);

    // === Notify Handlers === //

    // --- Handle incoming token transfers to automatically stake tokens --- //
    [[eosio::on_notify("*::transfer")]]
    void on_transfer(name from, name to, asset quantity, std::string memo);

    // === Emergency Pause === //
    ACTION pause(bool should_pause, const name token_contract = name(), const symbol token_symbol = symbol());

private:
    // --- Staking Parameters --- //
    TABLE config {
        name creator;
        name token_contract;
        symbol token_symbol;
        name reward_token_contract;
        symbol reward_token_symbol;
        uint32_t unstake_period;
        uint32_t reward_rate; // Reward rate percentage
        bool is_paused = false;

        uint64_t primary_key() const { return token_symbol.code().raw(); }
        uint64_t by_reward_symbol() const { return reward_token_symbol.code().raw(); }
    };

    // --- User Stakes --- //
    TABLE stake_s {
        asset staked_amount;
        time_point_sec last_claim;

        uint64_t primary_key() const { return staked_amount.symbol.code().raw(); }
    };

    typedef multi_index<"config"_n, config,
        indexed_by<"byreward"_n, const_mem_fun<config, uint64_t, &config::by_reward_symbol>>
    > config_t;
    typedef multi_index<"stakes"_n, stake_s> stake_t;

    // === Utility Functions === //
    /**
     * @brief Processes the claim for a user.
     * 
     * @param user The account claiming rewards.
     * @param reset_unstake_period Whether to reset the unstake period after claiming.
     */
    void process_claim(const name& user, bool reset_unstake_period = true);

};