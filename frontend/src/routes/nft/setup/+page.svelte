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

    let selectedStandard = "";
    let selectedVariation = "";
    let fieldList = [];
    let schemaName = "";
    const userInput = writable({});
    let collectionPickerOpen = false;
    let collections = []; // Array to store fetched collection names

    session.subscribe(async session => {
        if (session) {
            fetchCollections();
        }
    });

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

            console.log("finalFields",finalFields);

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

    <!-- Collection Picker using Popover Component -->
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

    <div
        class="m-4 block rounded-lg bg-gray-50 p-2.5 text-sm text-gray-900 focus:border-blue-500 focus:ring-blue-500 dark:border-gray-600 dark:bg-gray-700 dark:text-white dark:placeholder-gray-400 dark:focus:border-blue-500 dark:focus:ring-blue-500"
    >
        <Input type="text" bind:value={schemaName} placeholder="Name your new Schema aka Category" />
    </div>

    <div class="flex flex-col gap-2">
        <select
            class="m-2 block w-full rounded-lg border border-gray-300 bg-gray-50 p-2.5 text-sm text-gray-900 focus:border-blue-500 focus:ring-blue-500 dark:border-gray-600 dark:bg-gray-700 dark:text-white dark:placeholder-gray-400 dark:focus:border-blue-500 dark:focus:ring-blue-500"
            bind:value={selectedStandard}
            on:change={() => loadFieldsForStandard(selectedStandard, selectedVariation)}
        >
            <option value="">Select a Standard</option>
            <option value="literature">Literature</option>
            <option value="music">Music</option>
            <option value="video">Video</option>
            <option value="photo">Photo</option>
            <option value="image">Image</option>
        </select>
        <select
            class="m-2 block w-full rounded-lg border border-gray-300 bg-gray-50 p-2.5 text-sm text-gray-900 focus:border-blue-500 focus:ring-blue-500 dark:border-gray-600 dark:bg-gray-700 dark:text-white dark:placeholder-gray-400 dark:focus:border-blue-500 dark:focus:ring-blue-500"
            bind:value={selectedVariation}
            on:change={() => loadFieldsForStandard(selectedStandard, selectedVariation)}
        >
            <option value="">Select a Variation</option>
            <option value="web4">Web4</option>
            <option value="light">Light</option>
        </select>
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
