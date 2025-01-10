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

    let purpleStaked = writable(0);
    let pendingReward = writable(0);
    let stakeAmount = writable("");
    let unstakeAmount = writable("");
    let alertOpen = false;
    let alertMessage = '';

    // Subscribe to session changes to fetch initial data
    session.subscribe(async session => {
        if (session) {
            await fetchStakingData();
        }
    });

    async function fetchStakingData() {
        if (!$session?.actor) return;
        
        // TODO: Replace with actual API calls
        // Placeholder data fetching logic
        purpleStaked.set(100); // Example value
        pendingReward.set(10); // Example value
    }

    async function handleStake(amount) {
        if (!$session) return;
        
        const stakeTx = {
            account: "purplepurple",
            name: "transfer",
            authorization: [$session.permissionLevel],
            data: {
                from: $session.actor,
                to: "stakepurple",
                quantity: `${amount.toFixed(8)} PURPLE`,
                memo: ""
            }
        };

        transact([stakeTx], { broadcast: true }).then(result => {
            if (result.resolved) {
                alertMessage = `Staked ${amount} PURPLE successfully! <span class="text-xs"><a href="https://waxblock.io/transaction/${result.resolved.transaction.id}">view tx</a></span>`;
                alertOpen = true;
            }
        });
    }

    async function handleUnstake(amount) {
        if (!$session) return;

        const unstakeTx = {
            account: "stakepurple",
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
            }
        });
    }

    async function handleClaim() {
        if (!$session) return;

        const claimTx = {
            account: "stakepurple",
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
            }
        });
    }

    function setMaxStake() {
        // TODO: Implement max stake logic based on wallet balance
        stakeAmount.set("100");
    }

    function setMaxUnstake() {
        // TODO: Implement max unstake logic based on staked amount
        unstakeAmount.set($purpleStaked.toString());
    }

    $: canStake = $session && Number($stakeAmount) > 0;
    $: canUnstake = $session && Number($unstakeAmount) > 0 && Number($unstakeAmount) <= $purpleStaked;
    $: canClaim = $session && $pendingReward > 0;
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
            <div class="flex justify-between items-center mb-2">
                <span class="text-white font-saira">Purple Staked</span>
                <span class="text-purple-400 font-saira font-bold">{$purpleStaked} PURPLE</span>
            </div>
            <div class="flex justify-between items-center">
                <span class="text-white font-saira">Pending Reward</span>
                <span class="text-blue-400 font-saira font-bold">{$pendingReward} BLUX</span>
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
