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
        refreshAllData($session.actor.toString());
    }

    onMount(() => {
        // Check for Tonomy login callback parameters in URL
        const urlParams = new URLSearchParams(window.location.search);
        const error = urlParams.get('error');
        const errorDescription = urlParams.get('error_description');
        const token = urlParams.get('token');

        if (error) {
            alertMessage = `Tonomy Login Error: ${error} - ${errorDescription || 'Unknown error'}`;
            alertOpen = true;
            console.error('Tonomy Login Error:', error, errorDescription);
        } else if (token) {
            // Handle successful login (placeholder for session setup)
            alertMessage = 'Tonomy Login Successful! Session setup in progress...';
            alertOpen = true;
            console.log('Tonomy Login Token Received:', token);
            // TODO: Implement session handling with token
            // For now, just show success message
        } else {
            console.log('No Tonomy callback parameters found in URL');
        }

        return () => {};
    });

    function handleLogin() {
        // Updated redirect URL for Tonomy login with correct testnet domain and callback
        const callbackUrl = encodeURIComponent(window.location.origin + '/invitono');
        const appId = 'cxc'; // Application identifier as provided
        const redirectUrl = 'https://accounts.testnet.tonomy.io?redirect_uri=' + callbackUrl + '&app_id=' + appId + '&scope=login';
        console.log('Redirecting to Tonomy SSO with URL:', redirectUrl);
        window.location.href = redirectUrl; // Added app_id and scope for context
    }

    function handleLogout() {
        logout();
        alertMessage = 'Logged out successfully';
        alertOpen = true;
    }

    function handleRegister() {
        if (!$session?.actor) {
            alertMessage = 'Please login first';
            alertOpen = true;
            return;
        }

        isLoading = true;
        registerUser($session.actor.toString(), inviteCode)
            .then(() => {
                alertMessage = 'Registered successfully!';
                alertOpen = true;
                isLoading = false;
            })
            .catch((e) => {
                console.error(e);
                alertMessage = 'Registration failed: ' + e.message;
                alertOpen = true;
                isLoading = false;
            });
    }

    function handleClaimReward() {
        if (!$session?.actor) {
            alertMessage = 'Please login first';
            alertOpen = true;
            return;
        }

        isLoading = true;
        claimReward($session.actor.toString())
            .then(() => {
                alertMessage = 'Reward claimed!';
                alertOpen = true;
                isLoading = false;
            })
            .catch((e) => {
                console.error(e);
                alertMessage = 'Claim failed: ' + e.message;
                alertOpen = true;
                isLoading = false;
            });
    }
</script>

<section class="flex flex-col items-center justify-center px-4 py-2">
    <div class="w-full max-w-6xl flex justify-between items-center mb-2">
        <!-- Tonomy Login Button (Top Left) -->
        {#if !$session?.actor}
            <Button on:click={handleLogin} class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded shadow-lg transition duration-200 ease-in-out transform hover:scale-105">
                Login with Tonomy
            </Button>
        {:else}
            <div class="flex items-center gap-2">
                <span class="text-sm text-gray-600">Logged in as: {$session.actor.toString()}</span>
                <Button on:click={handleLogout} class="bg-red-600 hover:bg-red-700 text-white font-bold py-1 px-2 rounded shadow-lg transition duration-200 ease-in-out transform hover:scale-105">
                    Logout
                </Button>
            </div>
        {/if}
    </div>

    <div class="max-w-2xl w-full relative">
        <AlertDialog.Root open={alertOpen} onOpenChange={(open) => (alertOpen = open)}>
            <AlertDialog.Content class="sm:max-w-[425px] bg-white shadow-lg rounded-lg p-6 mx-auto my-auto fixed inset-0 flex items-center justify-center z-50 backdrop-blur-sm transition-all duration-300 ease-out transform scale-95 opacity-0 animate-in fade-in zoom-in-95 slide-in-from-bottom-5 sm:slide-in-from-bottom-0 sm:zoom-in-100 sm:fade-in-100 sm:scale-100 sm:opacity-100 data-closed:animate-out data-closed:fade-out data-closed:zoom-out-95 data-closed:slide-out-to-bottom-5 sm:data-closed:slide-out-to-bottom-0 sm:data-closed:zoom-out-100 sm:data-closed:fade-out-100 sm:data-closed:scale-95 sm:data-closed:opacity-0 max-h-[90vh] overflow-y-auto border border-gray-200">
                <AlertDialog.Header>
                    <AlertDialog.Title>Alert</AlertDialog.Title>
                    <AlertDialog.Description>{alertMessage}</AlertDialog.Description>
                </AlertDialog.Header>
                <AlertDialog.Footer>
                    <AlertDialog.Action on:click={() => (alertOpen = false)}>OK</AlertDialog.Action>
                </AlertDialog.Footer>
            </AlertDialog.Content>
        </AlertDialog.Root>

        <div class="bg-gradient-to-r from-purple-400 via-pink-500 to-red-500 p-1 rounded-xl shadow-xl">
            <div class="bg-white rounded-xl p-6 h-full flex flex-col justify-between gap-6 shadow-inner">
                <div>
                    <h1 class="text-3xl font-bold mb-2 text-center text-gray-800">Welcome to Invitono</h1>
                    <p class="text-gray-600 text-center mb-4">A revolutionary platform for invitations and rewards.</p>
                </div>

                {#if $session?.actor}
                    <div class="bg-blue-50 p-4 rounded-lg shadow-md mb-4 border border-blue-200">
                        <h2 class="text-xl font-semibold mb-2 text-blue-800">Your Stats</h2>
                        {#if $inviteData}
                            <p class="text-blue-600">Score: {$inviteData.score}</p>
                            <p class="text-blue-600">Claimed: {$inviteData.claimed ? 'Yes' : 'No'}</p>
                            <p class="text-blue-600">Last Updated: {new Date($inviteData.lastUpdated * 1000).toLocaleString()}</p>
                        {:else}
                            <p class="text-blue-600">No data available. Please register.</p>
                        {/if}
                    </div>

                    <div class="flex justify-center mb-4">
                        {#if $inviteData && !$inviteData.claimed}
                            <Button on:click={handleClaimReward} disabled={isLoading} class="bg-green-600 hover:bg-green-700 text-white font-bold py-2 px-4 rounded shadow-lg transition duration-200 ease-in-out transform hover:scale-105 disabled:opacity-50 disabled:cursor-not-allowed">
                                {isLoading ? 'Claiming...' : 'Claim Reward'}
                            </Button>
                        {:else if $inviteData && $inviteData.claimed}
                            <p class="text-green-600 font-semibold">Reward already claimed!</p>
                        {:else}
                            <p class="text-red-600 font-semibold">Register to claim rewards.</p>
                        {/if}
                    </div>
                {/if}

                <div class="bg-gray-50 p-4 rounded-lg shadow-md border border-gray-200">
                    <h2 class="text-xl font-semibold mb-2 text-gray-800">Global Stats</h2>
                    {#if $globalStats}
                        <p class="text-gray-600">Total Referrals: {$globalStats.totalReferrals}</p>
                        <p class="text-gray-600">Total Users: {$globalStats.totalUsers}</p>
                        <p class="text-gray-600">Last Registered: {$globalStats.lastRegistered}</p>
                    {:else}
                        <p class="text-gray-600">Loading stats...</p>
                    {/if}
                </div>

                {#if !$session?.actor}
                    <div class="bg-yellow-50 p-4 rounded-lg shadow-md mt-4 border border-yellow-200">
                        <p class="text-yellow-800">Please log in with Tonomy to access your personalized data and register on the platform.</p>
                    </div>
                {:else if !$inviteData?.score}
                    <div class="mt-4">
                        <h3 class="text-lg font-medium mb-2 text-gray-800">Register with Invite Code</h3>
                        <div class="flex gap-2">
                            <Input bind:value={inviteCode} placeholder="Enter your invite code" class="rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 sm:text-sm" />
                            <Button on:click={handleRegister} disabled={isLoading || !inviteCode} class="bg-indigo-600 hover:bg-indigo-700 text-white font-bold py-2 px-4 rounded shadow-lg transition duration-200 ease-in-out transform hover:scale-105 disabled:opacity-50 disabled:cursor-not-allowed">
                                {isLoading ? 'Registering...' : 'Register'}
                            </Button>
                        </div>
                    </div>
                {/if}
            </div>
        </div>
    </div>
</section> 