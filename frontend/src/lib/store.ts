import { goto } from "$app/navigation";
import { Chains, SessionKit, type AnyAction, type Session } from "@wharfkit/session";
import { TransactPluginAutoCorrect } from "@wharfkit/transact-plugin-autocorrect";
import { WalletPluginAnchor } from "@wharfkit/wallet-plugin-anchor";
import { WalletPluginCloudWallet } from "@wharfkit/wallet-plugin-cloudwallet";
import { WalletPluginWombat } from "@wharfkit/wallet-plugin-wombat";
import WebRenderer from "@wharfkit/web-renderer";
import { get, writable } from "svelte/store";

export const balances = writable<LightAPI.TokenBalance[]>([]);
export const session = writable<Session | undefined>(undefined);
export const selectedCollection = writable("");
export const tonomySession = writable<any | undefined>(undefined);


const sessionKit = new SessionKit(
    {
        appName: "tools.cXc.world",
        chains: [Chains.WAX],
        ui: new WebRenderer(),
        walletPlugins: [new WalletPluginAnchor(), new WalletPluginCloudWallet(), new WalletPluginWombat()],
    },
    {
        transactPlugins: [new TransactPluginAutoCorrect()],
    }
);

// Tonomy configuration for SSO login
const tonomyConfig = {
    ssoWebsiteOrigin: 'https://accounts.testnet.pangea.web4.world',
    returnUrl: window.location.origin + '/invitono'
};

export async function login() {
    const response = await sessionKit.login();
    session.set(response.session);
}

export async function loginWithTonomy() {
    // Redirect to Tonomy Accounts website for SSO login
    const loginUrl = `${tonomyConfig.ssoWebsiteOrigin}/login?returnUrl=${encodeURIComponent(tonomyConfig.returnUrl)}`;
    window.location.href = loginUrl;
}

export async function logout() {
    const current = get(session);
    const tonomyCurrent = get(tonomySession);

    if (current) {
        await sessionKit.logout(current);
        session.set(undefined);
    }
    if (tonomyCurrent) {
        // For now, just clear the session
        tonomySession.set(undefined);
    }

    goto("/");
}

export async function restoreSession() {
    try {
        const restored = await sessionKit.restore();

        if (restored) {
            session.set(restored);
        }
    } catch (e) {}
}

export async function transact(actions: AnyAction[]) {
    const current = get(session);

    if (!current) {
        throw new Error("Session not found, login first");
    }

    return current.transact({ actions }, { broadcast: true });
}
