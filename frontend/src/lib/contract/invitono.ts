import { APIClient, Name } from "@wharfkit/session";
import { writable } from "svelte/store";

const CONTRACT_ACCOUNT = "tono.cxc";
const API_URL = "https://wax.greymass.com";

// Store for user's invite data
export const inviteData = writable<{
    score: number;
    claimed: boolean;
    lastUpdated: number;
} | null>(null);

// Store for global stats
export const globalStats = writable<{
    totalReferrals: number;
    totalUsers: number;
    lastRegistered: string;
} | null>(null);

// Store for contract config
export const contractConfig = writable<{
    minAccountAgeDays: number;
    inviteRateSeconds: number;
    enabled: boolean;
    maxReferralDepth: number;
    multiplier: number;
    tokenContract: string;
    rewardSymbol: string;
    rewardRate: number;
} | null>(null);

// Initialize API client
const api = new APIClient({ url: API_URL });

// Fetch user's invite data
export async function fetchInviteData(account: string) {
    try {
        const response = await api.v1.chain.get_table_rows({
            code: CONTRACT_ACCOUNT,
            scope: CONTRACT_ACCOUNT,
            table: "adopters",
            lower_bound: Name.from(account),
            upper_bound: Name.from(account),
            limit: 1,
            json: true
        });

        if (response.rows.length > 0) {
            inviteData.set({
                score: response.rows[0].score,
                claimed: response.rows[0].claimed,
                lastUpdated: response.rows[0].lastupdated
            });
        } else {
            inviteData.set(null);
        }
    } catch (e) {
        console.error("Error fetching invite data:", e);
        inviteData.set(null);
    }
}

// Fetch global stats
export async function fetchGlobalStats() {
    try {
        const response = await api.v1.chain.get_table_rows({
            code: CONTRACT_ACCOUNT,
            scope: CONTRACT_ACCOUNT,
            table: "stats",
            limit: 1,
            json: true
        });

        if (response.rows.length > 0) {
            globalStats.set({
                totalReferrals: response.rows[0].total_referrals,
                totalUsers: response.rows[0].total_users,
                lastRegistered: response.rows[0].last_registered
            });
        }
    } catch (e) {
        console.error("Error fetching global stats:", e);
    }
}

// Fetch contract config
export async function fetchContractConfig() {
    try {
        const response = await api.v1.chain.get_table_rows({
            code: CONTRACT_ACCOUNT,
            scope: CONTRACT_ACCOUNT,
            table: "config",
            limit: 1,
            json: true
        });

        if (response.rows.length > 0) {
            contractConfig.set({
                minAccountAgeDays: response.rows[0].min_account_age_days,
                inviteRateSeconds: response.rows[0].invite_rate_seconds,
                enabled: response.rows[0].enabled,
                maxReferralDepth: response.rows[0].max_referral_depth,
                multiplier: response.rows[0].multiplier,
                tokenContract: response.rows[0].token_contract,
                rewardSymbol: response.rows[0].reward_symbol,
                rewardRate: response.rows[0].reward_rate
            });
        }
    } catch (e) {
        console.error("Error fetching contract config:", e);
    }
}

// Register user action
export async function registerUser(account: string, inviter: string) {
    return {
        account: CONTRACT_ACCOUNT,
        name: "registeruser",
        authorization: [{
            actor: account,
            permission: "active"
        }],
        data: {
            user: account,
            inviter: inviter
        }
    };
}

// Claim reward action
export async function claimReward(account: string) {
    return {
        account: CONTRACT_ACCOUNT,
        name: "claimreward",
        authorization: [{
            actor: account,
            permission: "active"
        }],
        data: {
            user: account
        }
    };
}

// Refresh all data
export async function refreshAllData(account: string) {
    await Promise.all([
        fetchInviteData(account),
        fetchGlobalStats(),
        fetchContractConfig()
    ]);
} 