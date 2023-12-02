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
        <p class="font-saira text-center font-medium sm:text-lg lg:text-xl">Login to access the tools</p>
    {:else}
        <p class="font-saira text-center font-medium sm:text-lg lg:text-xl">Click below to access the tools</p>
    {/if}

    <div class="mt-16 flex flex-row gap-4">
        {#if $session}
            <Button href="/nft/setup" title="Create a new Schema (aka Category) for an existing Atomic Assets collection" variant="primary">
                Create your Media NFT Category
            </Button>
            <Button href="https://standards.cxc.world" target="_blank" title="Metadata standards for Music, Video, Photo, Literature and Image">
                Learn about the standards
            </Button>
        {:else}
            <Button on:click={login} variant="outline">Login</Button>
        {/if}
    </div>
</section>

<style>

</style>
