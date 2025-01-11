<script lang="ts">
    import { API_NODE_URL, fetchBalances, fetchLinkId, waitFor } from "$lib";
    import { writable, get } from "svelte/store";
    import { Input } from "$lib/components/ui/input";
    import { Button } from "$lib/components/ui/button";
    import { cn } from "$lib/utils";
    import * as AlertDialog from "$lib/components/ui/alert-dialog";
    import { balances, session, transact } from "$lib/store";
    import { APIClient, Asset, KeyType, PrivateKey } from "@wharfkit/session";
    import { onMount } from "svelte";

    let alertOpen = false;
    let alertMessage = '';

    // State variables using writable stores
    let purpleStaked = writable("0.00000000 PURPLE");
    let purpleBalance = writable("0.00000000 PURPLE");
    let pendingReward = writable("0 BLUX");
    let stakeAmount = writable("");
    let unstakeAmount = writable("");
    let currentLevel = writable(0);
    let currentBonus = writable(0);
    let nextLevelAmount = writable("0.00000000 PURPLE");

    // Function to fetch PURPLE balance
    async function fetchPurpleBalance() {
        if (!$session?.actor) return;
        
        const api = new APIClient({
            url: "https://wax.greymass.com"
        });

        try {
            const response = await api.v1.chain.get_table_rows({
                code: "purplepurple",
                scope: $session.actor,
                table: "accounts",
                limit: 1,
                index_position: 1,
                key_type: "i64",
                json: true
            });

            if (response.rows.length > 0) {
                purpleBalance.set(response.rows[0].balance);
            } else {
                purpleBalance.set("0.00000000 PURPLE");
            }
        } catch (e) {
            console.error("Error fetching PURPLE balance:", e);
            purpleBalance.set("0.00000000 PURPLE");
        }
    }

    // Function to fetch staked amount
    async function fetchStakedAmount() {
        if (!$session?.actor) return;

        await Promise.all([
            (async () => {
                const api = new APIClient({
                    url: "https://wax.greymass.com"
                });

                try {
                    const response = await api.v1.chain.get_table_rows({
                        code: "stake.cxc",
                        scope: $session.actor,
                        table: "stakes",
                        limit: 1,
                        index_position: 1,
                        key_type: "i64",
                        json: true
                    });

                    if (response.rows.length > 0) {
                        purpleStaked.set(response.rows[0].staked_amount);
                        calculateLevelInfo(parseFloat(response.rows[0].staked_amount));
                    } else {
                        purpleStaked.set("0.00000000 PURPLE");
                        calculateLevelInfo(0);
                    }
                } catch (e) {
                    console.error("Error fetching staked amount:", e);
                }
            })(),
            calculatePendingReward()
        ]);
    }

    // Update both balances whenever session changes or after transactions
    function updateBalances() {
        fetchPurpleBalance();
        fetchStakedAmount();
    }

    // Subscribe to session changes
    $: if ($session) {
        updateBalances();
    }

    // Set up initial load and periodic refresh
    onMount(() => {
        updateBalances();
        const interval = setInterval(updateBalances, 5000); // Refresh every 5 seconds
        return () => clearInterval(interval);
    });

    // Add function to calculate pending rewards
    async function calculatePendingReward() {
        if (!$session?.actor) return;
        
        const api = new APIClient({
            url: "https://wax.greymass.com"
        });

        try {
            // Get stake info including last_claim
            const stakeResponse = await api.v1.chain.get_table_rows({
                code: "stake.cxc",
                scope: $session.actor,
                table: "stakes",
                limit: 1,
                index_position: 1,
                key_type: "i64",
                json: true
            });

            if (stakeResponse.rows.length > 0) {
                const stake = stakeResponse.rows[0];
                const last_claim = new Date(stake.last_claim + "Z");
                const staked_amount = parseFloat(stake.staked_amount);
                
                // Calculate time passed
                const now = new Date();
                const time_since_last_claim = Math.floor((now.getTime() - last_claim.getTime()) / 1000);
                const days_passed = Math.floor(time_since_last_claim / (24 * 3600));
                
                // Calculate level based on Tetrahedral series
                const amount = Math.floor(staked_amount);
                let level = 0;
                for (let i = 0; i < TETRAHEDRAL.length; i++) {
                    if (amount < TETRAHEDRAL[i]) {
                        level = i;
                        break;
                    }
                }
                
                // Calculate reward
                const reward_rate = 100 + level; // Base rate (100) + level bonus
                const daily_reward = amount * (reward_rate / 100);
                const level_bonus = TRIANGULAR[level];
                
                // Total reward = (daily reward * days passed) + level bonus
                const total_reward = (daily_reward * days_passed) + level_bonus;
                
                pendingReward.set(`${total_reward.toFixed(0)} BLUX`);
            } else {
                pendingReward.set("0 BLUX");
            }
        } catch (e) {
            console.error("Error calculating pending reward:", e);
            pendingReward.set("0 BLUX");
        }
    }

    // Add the TETRAHEDRAL and TRIANGULAR series from the smart contract
    const TETRAHEDRAL = [1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364, 455, 560, 680, 816, 969, 1140, 1330, 1540, 1771, 2024, 2300, 2600, 2925, 3276, 3654, 4060, 4505, 4990, 5516, 6084, 6725, 7440, 8230, 9096, 99999999999];
    const TRIANGULAR = [1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190, 210, 231, 253, 276, 300, 325, 351, 378, 406];

    function calculateLevelInfo(stakedAmount: number) {
        // Use the exact staked amount
        const amount = stakedAmount;
        let level = 0;
        
        // Find current level
        for (let i = 0; i < TETRAHEDRAL.length; i++) {
            if (amount < TETRAHEDRAL[i]) {
                level = i;
                break;
            }
        }
        
        // Calculate bonus percentage (100 + level)
        const bonus = 100 + level;
        
        // Calculate amount needed for next level
        const nextRequired = level < TETRAHEDRAL.length - 1 
            ? `${(TETRAHEDRAL[level] - amount).toFixed(8)} PURPLE`
            : "MAX LEVEL";

        currentLevel.set(level);
        currentBonus.set(bonus);
        nextLevelAmount.set(nextRequired);
    }

    async function handleStake() {
        if (!$session) return;
        
        const amount = parseFloat($stakeAmount);
        if (isNaN(amount)) return;
        
        const stakeTx = {
            account: "purplepurple",
            name: "transfer",
            authorization: [$session.permissionLevel],
            data: {
                from: $session.actor,
                to: "stake.cxc",
                quantity: `${amount.toFixed(8)} PURPLE`,
                memo: ""
            }
        };

        transact([stakeTx], { broadcast: true }).then(result => {
            if (result.resolved) {
                alertMessage = `Staked ${amount} PURPLE successfully! <span class="text-xs"><a href="https://waxblock.io/transaction/${result.resolved.transaction.id}">view tx</a></span>`;
                alertOpen = true;
                updateBalances();
            }
        });
    }

    async function handleUnstake() {
        if (!$session) return;
        
        const amount = parseFloat($unstakeAmount);
        if (isNaN(amount)) return;

        const unstakeTx = {
            account: "stake.cxc",
            name: "unstake",
            authorization: [$session.permissionLevel],
            data: {
                user: $session.actor,
                quantity: `${amount.toFixed(8)} PURPLE`
            }
        };

        transact([unstakeTx], { broadcast: true }).then(result => {
            if (result.resolved) {
                alertMessage = `Unstaked ${amount} PURPLE successfully! <span class="text-xs"><a href="https://waxblock.io/transaction/${result.resolved.transaction.id}">view tx</a></span>`;
                alertOpen = true;
                updateBalances();
            }
        });
    }

    async function handleClaim() {
        if (!$session) return;

        const claimTx = {
            account: "stake.cxc",
            name: "claim",
            authorization: [$session.permissionLevel],
            data: {
                user: $session.actor
            }
        };

        transact([claimTx], { broadcast: true }).then(result => {
            if (result.resolved) {
                alertMessage = `Claimed rewards successfully! <span class="text-xs"><a href="https://waxblock.io/transaction/${result.resolved.transaction.id}">view tx</a></span>`;
                alertOpen = true;
                updateBalances();
            }
        });
    }

    function setMaxStake() {
        if (!$purpleBalance) return;
        // Extract the numerical value from the balance string (e.g., "10.00000000 PURPLE" -> 10)
        const maxAmount = parseFloat($purpleBalance);
        stakeAmount.set(maxAmount.toString());
    }

    function setMaxUnstake() {
        if (!$purpleStaked) return;
        // Extract the numerical value from the staked amount string
        const maxAmount = parseFloat($purpleStaked);
        unstakeAmount.set(maxAmount.toString());
    }

    $: canStake = $session && Number($stakeAmount) > 0;
    $: canUnstake = $session && Number($unstakeAmount) > 0 && Number($unstakeAmount) <= parseFloat($purpleStaked);
    $: canClaim = $session && parseFloat($pendingReward) > 0;
</script>

<section class="flex flex-col items-center justify-center px-4 py-2">
    <AlertDialog.Root bind:open={alertOpen}>
        <AlertDialog.Content>
            <AlertDialog.Header>
                <AlertDialog.Title>Transaction Sent</AlertDialog.Title>
                <AlertDialog.Description>
                    <div class="flex flex-col gap-2">
                        <p class="rounded bg-green-700 p-2 text-center font-saira font-semibold text-white">{@html alertMessage}</p>
                    </div>
                </AlertDialog.Description>
            </AlertDialog.Header>
            <AlertDialog.Footer>
                <AlertDialog.Action>Done</AlertDialog.Action>
            </AlertDialog.Footer>
        </AlertDialog.Content>
    </AlertDialog.Root>

    <img src="/images/cxcmusicnfts-spinning-logo.gif" alt="cXc Logo" class="h-auto max-w-full" />

    <!-- Information Display -->
    <div class="m-4 flex flex-col gap-2 w-full max-w-md">
        <div class="rounded-lg bg-gray-800 p-4">
            <div class="flex flex-col gap-4 mb-4">
                <div class="flex justify-between items-center">
                    <span class="text-white font-saira">Level Info</span>
                    <div class="text-right">
                        <span class="text-purple-400 font-saira font-bold">Level {$currentLevel}</span>
                        <span class="text-gray-400 font-saira text-sm ml-2">({$currentBonus}% APR)</span>
                    </div>
                </div>
                <div class="flex justify-between items-center">
                    <span class="text-white font-saira">Next Level In</span>
                    <span class="text-purple-400 font-saira font-bold text-right">{$nextLevelAmount}</span>
                </div>
                <div class="flex justify-between items-center">
                    <span class="text-white font-saira">Purple Staked</span>
                    <span class="text-purple-400 font-saira font-bold text-right">{$purpleStaked}</span>
                </div>
                <div class="flex justify-between items-center">
                    <span class="text-white font-saira">Wallet Balance</span>
                    <span class="text-purple-400 font-saira font-bold text-right">{$purpleBalance}</span>
                </div>
                <div class="flex justify-between items-center">
                    <span class="text-white font-saira">Pending Reward</span>
                    <span class="text-blue-400 font-saira font-bold text-right">{$pendingReward}</span>
                </div>
            </div>
        </div>
    </div>

    <!-- Staking Form -->
    <div class="m-4 flex flex-col gap-4 w-full max-w-md">
        <div class="flex flex-col gap-2">
            <div class="flex items-center gap-2">
                <Input 
                    type="text" 
                    bind:value={$stakeAmount} 
                    placeholder="Amount to stake" 
                    class={!$session ? "opacity-50" : ""}
                    disabled={!$session}
                />
                <Button 
                    variant="outline" 
                    class="text-xs"
                    on:click={setMaxStake}
                    disabled={!$session}
                >
                    Max
                </Button>
            </div>
            <Button 
                class="w-full bg-purple-600 hover:bg-purple-700" 
                on:click={handleStake}
                disabled={!canStake}
            >
                Stake PURPLE
            </Button>
        </div>

        <div class="flex flex-col gap-2">
            <div class="flex items-center gap-2">
                <Input 
                    type="text" 
                    bind:value={$unstakeAmount} 
                    placeholder="Amount to unstake" 
                    class={!$session || $purpleStaked === 0 ? "opacity-50" : ""}
                    disabled={!$session || $purpleStaked === 0}
                />
                <Button 
                    variant="outline" 
                    class="text-xs"
                    on:click={setMaxUnstake}
                    disabled={!$session || $purpleStaked === 0}
                >
                    Max
                </Button>
            </div>
            <Button 
                class="w-full bg-red-600 hover:bg-red-700" 
                on:click={handleUnstake}
                disabled={!canUnstake}
            >
                Unstake PURPLE
            </Button>
        </div>

        <Button 
            class="w-full bg-green-600 hover:bg-green-700" 
            on:click={handleClaim}
            disabled={!canClaim}
        >
            Claim Rewards
        </Button>
    </div>
</section>
<style>
    :global(input[type="text"]) {
        background-color: rgb(55, 65, 81) !important;
        color: white !important;
    }
    :global(input[type="text"]::placeholder) {
        color: rgb(156, 163, 175) !important;
    }
</style>
