<template>
  <div class="card fluid home">
    <h2 class="section double-padded">Lift Controls</h2>
    <div class="section double-padded">
      <div class="col-12">
        <button v-on:click="liftGoUp"><arrow-up-icon size="5x"/></button>
      </div>
      <div class="col-12">
        <button v-on:click="liftStop"><square-icon size="5x" class="stop-button"/></button>
      </div>
      <div class="col-12">
        <button v-on:click="liftGoDown"><arrow-down-icon size="5x"/></button>
      </div>
    </div>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';
import { ArrowUpIcon, ArrowDownIcon, SquareIcon } from "vue-feather-icons";
import axios from "axios";

@Component({
  components: {
    ArrowUpIcon,
    ArrowDownIcon,
    SquareIcon
  },
})
export default class LiftControls extends Vue 
{
  private liftGoUp(): void
  {
    axios.post("/lift/up");
  }

  private liftGoDown(): void
  {
    axios.post("/lift/down");
  }

  private liftStop(): void
  {
    axios.post("/lift/stop");
  }
}
</script>

<style lang="scss">
.home
{
  text-align: center;
}

.stop-button
{
  color: var(--error-color);
}
</style>

