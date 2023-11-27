<script lang="ts">
	import { Checkbox as CheckboxPrimitive } from "bits-ui";
	import { Check, Minus } from "radix-icons-svelte";
    import { cn } from "$lib/utils";

	type $$Props = CheckboxPrimitive.Props;
	type $$Events = CheckboxPrimitive.Events;

	let className: $$Props["class"] = undefined;
	export let checked: $$Props["checked"] = false;
	export { className as class };
</script>

<CheckboxPrimitive.Root
	class={cn(
        "h-10 w-full rounded-md  bg-transparent px-3 py-2 text-sm ring-offset-background file:border-0  file:bg-transparent file:text-sm file:font-medium file:text-foreground placeholder:text-muted-foreground focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2 disabled:cursor-not-allowed disabled:opacity-50",
		className
	)}
	bind:checked
	on:click
	on:keydown
	{...$$restProps}
>
	<CheckboxPrimitive.Indicator
		class={cn("flex items-center justify-center text-current")}
		let:isChecked
		let:isIndeterminate
	>
		{#if isIndeterminate}
			<Minus class="h-4 w-4" />
		{:else}
			<Check class={cn("h-4 w-4", !isChecked && "text-transparent")} />
		{/if}
	</CheckboxPrimitive.Indicator>
</CheckboxPrimitive.Root>
