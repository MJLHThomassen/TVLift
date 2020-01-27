<template>

    <div class="about row">
      <div class="col-sm">
          <h1>About</h1>
          <p>Made by Maarten Thomassen</p>
          <blockquote cite="Me">Don't look behind, look ahead, but remember what was and prepare for what's next.</blockquote>
      </div>
    </div>
</template>

<script lang="ts">
import { Component, Vue, Prop } from "vue-property-decorator";

@Component({})
export default class About extends Vue {
  @Prop()
  private name: string;

  public constructor() {
    super();
    this.name = "Maarten Thomassen";
  }

  private doit(name: string): void {
    this.name = name;
  }
}
</script>

<style lang="scss">
.about
{
  text-align: center;
}
</style>