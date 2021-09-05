<template>
    <div class="debug">
        <h2>Debug</h2>
        <div class="console-wrapper">
            <console></console>
        </div>
    </div>
</template>

<script lang="ts">
import { Vue, Component } from "vue-property-decorator";

import Console from "@/components/Console.vue";

@Component({
    components: {
        Console,
    },
})
export default class Debug extends Vue
{
}
</script>

<style lang="scss">
.debug
{
    height: 100%;
    overflow: hidden;

    display: flex;
    flex-direction: column;
    flex-wrap: nowrap;
    justify-content: flex-start;
    align-items: stretch;

    text-align: center;

    .console-wrapper
    {
        flex: 1 1 auto;
    }
}
</style>