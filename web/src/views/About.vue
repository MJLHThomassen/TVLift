<template>
    <div class="about container">
      <div class="row">
          <div class="col-sm-12 col-lg-6 col-lg-offset-3">
            <h1>About</h1>
            <p>Made by Maarten Thomassen</p>
            <blockquote cite="Me">Don't look behind, look ahead, but remember what was and prepare for what's next.</blockquote>
          </div>
        </div>
        <div class="row">
          <div class="col-sm">
              <p>Build: {{version}}</p>
          </div>
        </div>
    </div>
</template>

<script lang="ts">
import { Component, Vue, Prop } from "vue-property-decorator";

@Component({})
export default class About extends Vue
{
  private version: string = `${process.env.VUE_APP_VERSION} (${process.env.NODE_ENV})`;

  public constructor() 
  {
    super();
  }
}
</script>

<style lang="scss">
.about
{
    text-align: center;
}
</style>