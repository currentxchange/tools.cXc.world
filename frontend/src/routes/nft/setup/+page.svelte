<script lang="ts">
    import { API_NODE_URL, fetchBalances, fetchLinkId, waitFor } from "$lib";
    import { writable, get } from "svelte/store";
    import { Input } from "$lib/components/ui/input";
    import { Button } from "$lib/components/ui/button";
    import { Checkbox } from "$lib/components/ui/checkbox";
    import { cn } from "$lib/utils";
    import { Check, ChevronsUpDown } from "lucide-svelte";
    import * as Popover from "$lib/components/ui/popover";
    import * as Command from "$lib/components/ui/command";

    import { balances, session, transact, selectedCollection } from "$lib/store";
    import { APIClient, Asset, KeyType, PrivateKey } from "@wharfkit/session";

    import { ExplorerApi, RpcApi } from "atomicassets";
    import { onMount } from "svelte";

    let standardPickerOpen = false;
    let variationPickerOpen = false;
    const standards = ["literature", "music", "video", "photo", "image"];
    const variations = ["web4", "light"];
    let selectedStandard = "";
    let selectedVariation = "";
    let fieldList = [];
    let schemaName = "";
    const userInput = writable({});
    const isInstructionsOpen = writable(false);
    let collectionPickerOpen = false;
    let collections = []; // Array to store fetched collection names

    session.subscribe(async session => {
        if (session) {
            fetchCollections();
        }
    });

    $: if (selectedStandard && selectedVariation) {
        loadFieldsForStandard(selectedStandard, selectedVariation);
    }

    async function fetchCollections() {
        if (!$session.actor) return; // Return early if actor is not set
        const explorerApi = new ExplorerApi("https://atomic-wax.tacocrypto.io", "atomicassets", { fetch });

        try {
            const fetchedCollections = await explorerApi.getCollections({ author: $session.actor });
            collections = fetchedCollections.map(col => col.collection_name);
        } catch (error) {
            console.error("Error fetching collections:", error);
        }
    }

    // Function to load fields for a selected standard and variation
    async function loadFieldsForStandard(standard: string, variation: string) {
        const githubBaseUrl = "https://raw.githubusercontent.com/currentxchange/WAX-NFT-Metadata-Standards/main/json";

        if (standard && variation) {
            try {
                const url = `${githubBaseUrl}/${standard}-${variation}.json`;
                const response = await fetch(url);
                const data = await response.json();
                fieldList = data.map(field => ({ ...field, checked: true }));
            } catch (error) {
                console.error("Error reading field data from GitHub:", error);
            }
        }
    }

    // Function to handle changes in the field list
    function handleFieldChange(field, value, type: "check" | "rename") {
        if (type === "check") {
            field.checked = value;
        } else if (type === "rename") {
            field.name = value;
        }
    }

    // Function to create the NFT schema
    async function createSchema() {
        if (!$session) {
            return;
        }

        const collectionName = get(selectedCollection);

        const finalFields = fieldList
            .filter(field => field.checked) // Keep only checked fields
            .map(field => ({
                name: field.name,
                type: field.type,
            })); // Map each field to the required format

        console.log("finalFields", finalFields);

        const schemaData = {
            authorized_creator: $session.actor, // Replace with actual user data
            collection_name: $selectedCollection, // Replace with actual collection data
            schema_format: finalFields,
            schema_name: schemaName,
        };

        // TODO: Implement the call to atomicassets 'createschema' action
        console.log("Creating schema with:", schemaData);

        const makeSchemaTx = {
            account: "atomicassets",
            name: "createschema",
            authorization: [$session.permissionLevel],
            data: schemaData,
        };

        console.log("SchemaTx:", makeSchemaTx);

        transact([makeSchemaTx], { broadcast: true }).then(result => {
            if (result.resolved) {
                transactionResult = `Transaction broadcast! ${result.resolved.transaction.id}\n`;
            }
        });
    }
</script>

<section class="flex flex-col items-center justify-center px-4 py-2">
    <!-- NFT Setup Form HTML Structure https://flowbite.com/docs/forms/textarea/ -->

    <img src="/images/cxcmusicnfts-spinning-logo.gif" alt="cXc Logo" class="h-auto max-w-full" />

    <div class="m-4 flex flex-col gap-2">
        <button 
            class="cursor-pointer text-lg font-semibold text-gray-800 hover:text-gray-600 focus:outline-none focus:ring focus:ring-gray-300 rounded"
            aria-expanded={$isInstructionsOpen}
            on:click={() => isInstructionsOpen.update(n => !n)}
        >
            <h2 class="text-white">Instructions</h2>
        </button>
        {#if $isInstructionsOpen}
            <div class="mt-2 p-4rounded-md bg-gray-100 text-gray-700">
                <p>1. Select an existing collection, or <a href="https://atomichub.io/creator/create-collection">make one</a></p>
                <p>2. Name your new schema, sometimes called category (up to 12 lower-case letters + numbers 1-5)</p>
                <p>3. Pick a Media Standard + Version (<a href="https://standards.cXc.world">more info</a>)</p>
                <p>4. Uncheck unneeded fields + rename fields if desired</p>
                <p>5. Click Create Schema and Send it!</p>
            </div>
        {/if}
    </div>

    <!-- Collection Picker using Popover Component -->
    <div class="m-4 flex flex-col gap-2">
        <Popover.Root bind:open={collectionPickerOpen} let:ids>
            <Popover.Trigger asChild let:builder>
                <Button
                    builders={[builder]}
                    variant="outline"
                    role="combobox"
                    aria-expanded={collectionPickerOpen}
                    class="w-[200px] justify-between"
                >
                    {$selectedCollection || "Select a collection"}
                    <ChevronsUpDown class="ml-2 h-4 w-4 shrink-0 opacity-50" />
                </Button>
            </Popover.Trigger>
            <Popover.Content class="w-[200px] p-0">
                <Command.Root>
                    <Command.Input placeholder="Search collection..." />
                    <Command.Empty>No collections found.</Command.Empty>
                    <Command.Group>
                        {#each collections as collection}
                            <Command.Item
                                value={collection}
                                onSelect={() => {
                                    selectedCollection.set(collection);
                                    // Close the popover and refocus if needed
                                    collectionPickerOpen = false;
                                }}
                            >
                                <Check class={cn("mr-2 h-4 w-4", get(selectedCollection) !== collection && "text-transparent")} />
                                <span class="font-saira">{collection}</span>
                            </Command.Item>
                        {/each}
                    </Command.Group>
                </Command.Root>
            </Popover.Content>
        </Popover.Root>
    </div>

    <div class="m-4 flex flex-col gap-2">
        <Input type="text" bind:value={schemaName} placeholder="Name your new Schema aka Category" />
    </div>
    <div class="m-4 flex flex-col gap-2">
        <Popover.Root bind:open={standardPickerOpen} let:ids>
            <Popover.Trigger asChild let:builder>
                <Button
                    builders={[builder]}
                    variant="outline"
                    role="combobox"
                    aria-expanded={standardPickerOpen}
                    class="w-[200px] justify-between"
                >
                    {selectedStandard || "Select a Standard"}
                    <ChevronsUpDown class="ml-2 h-4 w-4 shrink-0 opacity-50" />
                </Button>
            </Popover.Trigger>
            <Popover.Content class="w-[200px] p-0">
                <Command.Root>
                    <Command.Input placeholder="Search Standard..." />
                    <Command.Empty>No standards found.</Command.Empty>
                    <Command.Group>
                        {#each standards as standard}
                            <Command.Item
                                value={standard}
                                onSelect={() => {
                                    selectedStandard = standard;
                                    standardPickerOpen = false;
                                }}
                            >
                                <span class="font-saira">{standard}</span>
                            </Command.Item>
                        {/each}
                    </Command.Group>
                </Command.Root>
            </Popover.Content>
        </Popover.Root>
    </div>

    <div class="m-4 flex flex-col gap-2">
        <Popover.Root bind:open={variationPickerOpen} let:ids>
            <Popover.Trigger asChild let:builder>
                <Button
                    builders={[builder]}
                    variant="outline"
                    role="combobox"
                    aria-expanded={variationPickerOpen}
                    class="w-[200px] justify-between"
                >
                    {selectedVariation || "Select a Variation"}
                    <ChevronsUpDown class="ml-2 h-4 w-4 shrink-0 opacity-50" />
                </Button>
            </Popover.Trigger>
            <Popover.Content class="w-[200px] p-0">
                <Command.Root>
                    <Command.Input placeholder="Search Variation..." />
                    <Command.Empty>No variations found.</Command.Empty>
                    <Command.Group>
                        {#each variations as variation}
                            <Command.Item
                                value={variation}
                                onSelect={() => {
                                    selectedVariation = variation;
                                    variationPickerOpen = false;
                                }}
                            >
                                <span class="font-saira">{variation}</span>
                            </Command.Item>
                        {/each}
                    </Command.Group>
                </Command.Root>
            </Popover.Content>
        </Popover.Root>
    </div>

    <div class="m-4 flex flex-col gap-2">
        {#each fieldList as field, index (field.name)}
            <div>
                <Checkbox bind:checked={field.checked} on:change={() => handleFieldChange(field, !field.checked, "check")} />
                <Input bind:value={field.name} on:input={e => handleFieldChange(field, e.target.value, "rename")} />
                <span>{field.type}</span>
            </div>
        {/each}
    </div>

    <Button on:click={createSchema}>Create Schema</Button>
</section>
