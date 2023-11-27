<script lang="ts">
    import Button from "$lib/components/ui/button/button.svelte";
    import { logout, restoreSession, session } from "$lib/store";
    import { onMount } from "svelte";
    import "../app.pcss";
    import { CopyXIcon } from "lucide-svelte";

    let sessionX: Session | undefined;

    onMount(async () => {
        await restoreSession();
    });
</script>

<svelte:head>
    <title>tools.cXc.world - The future of NFTs</title>
    <meta name="description" content="Setup Web4 (spacetime-aware) NFT collection on WAX with Atomic Assets" />

    <link rel="author" href="https://github.com/currentxchange" />
    <meta name="author" content="cXc" />

    <link rel="icon" type="image/gif" href="/favicon.gif">
    <link rel="icon" type="image/png" href="/favicon.png">


    <meta name="keywords" content="antelope,wax,token,cXc,web4,atomicassets,nft" />

    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin="anonymous" />
    <link href="https://fonts.googleapis.com/css2?family=Saira:wght@300;400;600;700&display=swap" rel="stylesheet" />
</svelte:head>

<main class="relative flex min-h-screen flex-col font-saira dark:bg-black dark:text-white/90">
    <header
        class="border-orange sticky top-0 z-50 w-full border-b border-b-neutral-600 bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60"
    >
        <div class="container mx-auto flex h-14 items-center justify-between px-4">
            <a href="/" class="text-lg font-bold">tools.cXc.world</a>

            <div class="flex items-center gap-2">
                {#if $session}
                    <p class="font-saira">{$session.actor}</p>
                    <Button size="sm" on:click={logout} variant="outline">Logout</Button>
                {/if}
            </div>
        </div>
    </header>

    <slot />

    <footer class="mt-14">
        <div
            class="absolute bottom-0 flex h-14 w-full items-center justify-center border-t border-t-neutral-600 bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60"
        >
            <div class="container mx-auto flex items-center justify-between px-4">
                <a                         
                href="https://linktr.ee/cXc.world"
                target="_blank"
                rel="noreferrer" class="text-lg font-bold">More by cXc</a>

                <span class="text-sm">
                    Tools by <a
                        href="https://linktr.ee/cXc.world"
                        target="_blank"
                        rel="noreferrer"
                        class="font-semibold"
                    >
                        cXc 
                    </a>
                    & <a
                        href="https://linktr.ee/tetra.earth"
                        target="_blank"
                        rel="noreferrer"
                        class="font-semibold"
                    >
                        Tetra 
                    </a>
                </span>
            </div>
        </div>
    </footer>
</main>
