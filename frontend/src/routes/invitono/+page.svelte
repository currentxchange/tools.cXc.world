<script lang="ts">
    import { onMount } from "svelte";
    import { Input } from "$lib/components/ui/input";
    import { Button } from "$lib/components/ui/button";
    import { cn } from "$lib/utils";
    import * as AlertDialog from "$lib/components/ui/alert-dialog";
    import { session, transact, tonomySession, logout } from "$lib/store";
    import { inviteData, globalStats, contractConfig, refreshAllData, registerUser, claimReward } from "$lib/contract/invitono";
    import { goto } from '$app/navigation';

    let alertOpen = false;
    let alertMessage = '';
    let inviteCode = '';
    let isLoading = false;

    // Subscribe to session changes
    $: if ($session?.actor) {
        refreshAllData($session.actor);
    }

    // Set up initial load and periodic refresh
    onMount(() => {
        if ($session?.actor) {
            refreshAllData($session.actor);
            const interval = setInterval(() => refreshAllData($session.actor), 5000);
            return () => clearInterval(interval);
        }
    });

    onMount(async () => {
        // Check for Tonomy session data from URL parameters or local storage
        const urlParams = new URLSearchParams(window.location.search);
        const tonomyToken = urlParams.get('token');
        if (tonomyToken) {
            // Store the token or session data (this is a placeholder for actual session handling)
            tonomySession.set({ token: tonomyToken, loggedIn: true });
            // Clear URL parameters for cleanliness
            window.history.replaceState({}, document.title, window.location.pathname);
        }
    });

    async function handleRegister() {
        if (!$session?.actor) return;
        if (!inviteCode) {
            alertMessage = "Please enter an invite code";
            alertOpen = true;
            return;
        }

        isLoading = true;
        try {
            const action = await registerUser($session.actor, inviteCode);
            await transact([action]);
            alertMessage = "Successfully registered!";
            alertOpen = true;
            inviteCode = '';
            refreshAllData($session.actor);
        } catch (e) {
            console.error("Registration error:", e);
            alertMessage = "Registration failed: " + (e as Error).message;
            alertOpen = true;
        } finally {
            isLoading = false;
        }
    }

    async function handleClaim() {
        if (!$session?.actor) return;
        if (!$inviteData?.score) {
            alertMessage = "No rewards to claim";
            alertOpen = true;
            return;
        }

        isLoading = true;
        try {
            const action = await claimReward($session.actor);
            await transact([action]);
            alertMessage = "Rewards claimed successfully!";
            alertOpen = true;
            refreshAllData($session.actor);
        } catch (e) {
            console.error("Claim error:", e);
            alertMessage = "Claim failed: " + (e as Error).message;
            alertOpen = true;
        } finally {
            isLoading = false;
        }
    }

    function handleLogout() {
        logout();
        goto('/');
    }
</script>

<section class="flex flex-col items-center justify-center px-4 py-2">
    <div class="max-w-2xl w-full">
        <h1 class="text-3xl font-bold mb-8 text-center">Invitono</h1>

        <!-- Global Stats -->
        {#if $globalStats}
            <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mb-8">
                <div class="bg-card p-4 rounded-lg">
                    <h3 class="text-lg font-semibold">Total Users</h3>
                    <p class="text-2xl">{$globalStats.totalUsers}</p>
                </div>
                <div class="bg-card p-4 rounded-lg">
                    <h3 class="text-lg font-semibold">Total Referrals</h3>
                    <p class="text-2xl">{$globalStats.totalReferrals}</p>
                </div>
                <div class="bg-card p-4 rounded-lg">
                    <h3 class="text-lg font-semibold">Last Registered</h3>
                    <p class="text-xl">{$globalStats.lastRegistered}</p>
                </div>
            </div>
        {/if}

        <!-- User Status -->
        {#if $inviteData}
            <div class="bg-card p-6 rounded-lg mb-8">
                <h2 class="text-2xl font-bold mb-4">Your Status</h2>
                <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
                    <div>
                        <h3 class="text-lg font-semibold">Score</h3>
                        <p class="text-2xl">{$inviteData.score}</p>
                    </div>
                    <div>
                        <h3 class="text-lg font-semibold">Claim Status</h3>
                        <p class="text-xl">{$inviteData.claimed ? 'Claimed' : 'Available'}</p>
                    </div>
                </div>
            </div>
        {/if}

        <!-- Registration Form -->
        {#if !$inviteData}
            <div class="bg-card p-6 rounded-lg mb-8">
                <h2 class="text-2xl font-bold mb-4">Register</h2>
                <div class="flex gap-4">
                    <Input
                        type="text"
                        placeholder="Enter invite code"
                        bind:value={inviteCode}
                        class="flex-1"
                    />
                    <Button
                        on:click={handleRegister}
                        disabled={isLoading}
                    >
                        {isLoading ? 'Registering...' : 'Register'}
                    </Button>
                </div>
            </div>
        {/if}

        <!-- Claim Rewards -->
        {#if $inviteData && !$inviteData.claimed}
            <div class="bg-card p-6 rounded-lg">
                <h2 class="text-2xl font-bold mb-4">Claim Rewards</h2>
                <Button
                    on:click={handleClaim}
                    disabled={isLoading}
                    class="w-full"
                >
                    {isLoading ? 'Claiming...' : 'Claim Rewards'}
                </Button>
            </div>
        {/if}

        <!-- Alert Dialog -->
        <AlertDialog.Root bind:open={alertOpen}>
            <AlertDialog.Content>
                <AlertDialog.Header>
                    <AlertDialog.Title>Notification</AlertDialog.Title>
                </AlertDialog.Header>
                <AlertDialog.Description>
                    {alertMessage}
                </AlertDialog.Description>
                <AlertDialog.Footer>
                    <AlertDialog.Action asChild>
                        <Button>OK</Button>
                    </AlertDialog.Action>
                </AlertDialog.Footer>
            </AlertDialog.Content>
        </AlertDialog.Root>
    </div>
</section> 