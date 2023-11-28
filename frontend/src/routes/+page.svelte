<script lang="ts">
    import { Button } from "$lib/components/ui/button";
    import { login, session } from "$lib/store";
</script>

<section class="flex flex-col items-center justify-center px-4">
    <p
        class="bg-gradient-to-r from-yellow-600 to-grey-1000 bg-clip-text py-8 font-saira text-5xl font-bold text-transparent sm:text-6xl lg:text-8xl"
    >
        tools.cXc.world
    </p>
    {#if $session}
    <p class="text-center font-saira font-medium sm:text-lg lg:text-xl">Login to access the tools</p>
    {:else}
    <p class="text-center font-saira font-medium sm:text-lg lg:text-xl">Click below to access the tools</p>
    {/if}

    <div class="mt-16 flex flex-row gap-4">
        {#if $session}
            <Button href="/nft/setup" title="Create a new Schema (aka Category) for an existing Atomic Assets collection" variant="secondary">Create your Media NFT Category</Button>
            <Button href="https://standards.cxc.world" title="Metadata standards for Music, Video, Photo, Literature and Image" variant="secondary">Learn about the standards</Button>
        {:else}
            <Button on:click={login} variant="outline">Login</Button>
        {/if}
    </div>
</section>
