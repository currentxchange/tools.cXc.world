<script lang="ts">
    import { Button } from "$lib/components/ui/button";
    import { login, session } from "$lib/store";
    import { onMount } from 'svelte';

    onMount(async () => {
        colorizeHeaderText('.elegantshadowwht');
    });

    function colorizeHeaderText(selector) {
    const header = document.querySelector(selector);
    if (!header) return;
    const text = header.innerText;
    let coloredText = "";
    const colors = ["MediumOrchid", "VIOLET", "DEEPPINK", "AQUA", "LIGHTSKYBLUE", "CYAN", "DEEPSKYBLUE", "DodgerBlue", "MEDIUMSLATEBLUE", "YELLOWGREEN", "CornflowerBlue", "PALETURQUOISE", "lightSKYBLUE", "MEDIUMseaGREEN", "FORESTGREEN", "AQUAMARINE", "CORNSILK", "GOLDENROD", "GOLD", "ORANGE", "DARKORANGE", "ORANGERED", "CRIMSON", "RED", "SALMON", "PLUM", "LIGHTSALMON", "GAINSBORO", "SILVER", "TOMATO", "SEAGREEN", "SpringGreen", "GOLDENROD", "LIGHTCYAN", "LAVENDER", "FORESTGREEN", "LIGHTYELLOW", "INDIANRED", "TEAL", "PERU"];

    // Start at a random position in the colors array
    const startIndex = Math.floor(Math.random() * colors.length);
    for (let i = 0; i < text.length; i++) {
        // Use modulus to loop back to the start of the colors array if necessary
        const colorIndex = (startIndex + i) % colors.length;
        coloredText += `<span style="color: ${colors[colorIndex]}">${text[i]}</span>`;
    }
    header.innerHTML = coloredText;
}

</script>

<section class="flex flex-col items-center justify-center px-4">
    <p
        class="to-black-1000 font-saira bg-gradient-to-r from-yellow-600 bg-clip-text py-8 text-5xl font-bold text-transparent sm:text-6xl lg:text-8xl"
    >
        tools.cXc.world
    </p>

    {#if $session}
        <p class="font-saira text-center font-medium sm:text-lg lg:text-xl">You now can access the tools</p>
    {:else}
        <p class="font-saira text-center font-medium sm:text-lg lg:text-xl">Login to access the tools</p>
    {/if}

    <div class="mt-16 flex flex-col gap-4 justify-items-center items-center">
        {#if $session}
        <div class="m-4 flex flex-row max-w-full gap-4 justify-between">
            <Button class="bg-yellow-500" href="/nft/setup" title="Create a new Schema (aka Category) for an existing Atomic Assets collection" variant="primary">
                Launch your Media NFT Category
            </Button>
            <Button href="https://standards.cxc.world" target="_blank" title="Metadata standards for Music, Video, Photo, Literature and Image">
                Learn about our NFT standards
            </Button>
        </div>

        <div class="m-4 flex flex-row max-w-full gap-4 justify-between">
            <Button class="bg-green-500" href="https://docs.cxc.world/loot" target="_blank" title="Learn about Loot 💰 NFT Staking smart contract. Earn for your NFTs + earn more with friends">
                Loot 💰 Jucier NFT Staking 
            </Button>
            <Button class="flex row inline-flex"href="https://github.com/currentxchange/loot" target="_blank" title=" Deploy Loot 💰 NFT Staking smart contract. Earn for your NFTs + earn more with friends">
                Launch your NFT staking 🛠 Set up Loot
            </Button>

        </div>
        <div class="m-4 flex flex-row max-w-full gap-4 justify-between">

            <Button href="https://github.com/currentxchange/ups" target="_blank" title="Ups 🔺 Localized Reddit-like curation smart contract by cXc and Tetra for WAX + Antelope blockchains">
                Ups 🌎🔺 Localized curation contract for NFTs + Web4
            </Button>
        </div>

        <div class="m-4 flex flex-row max-w-full gap-4 justify-between">
            <Button href="https://www.youtube.com" target="_blank" title="Watch YouTube videos about NFT standards">
                Watch NFT Standards Videos
            </Button>
        </div>

        <div class="m-4 flex flex-row max-w-full gap-4 justify-between">
            <Button href="/stake" title="Stake PURPLE">
                Staking
            </Button>
        </div>

        {:else}
            <Button on:click={login} variant="outline">Login</Button>
        {/if}
    </div>
</section>

<style>

</style>
