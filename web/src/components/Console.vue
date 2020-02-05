<template>
  <div class="console-window">
    <div class="console" ref="console">
      <span class="code-line" v-for="line in consoleLines" :key="line.index">
        <span v-for="(span, index) in line.spans" :key="index" :style="span.css">
          {{ span.text }}
        </span>
      </span>
    </div>
  </div>
</template>

<script lang="ts">
import { Component, Prop, Vue, Watch } from "vue-property-decorator";
import * as Ansicolor from "ansicolor";

interface ConsoleLine
{
  index: number;
  spans: any[];
}

@Component
export default class Console extends Vue
{
  public $refs!:
  {
    console: HTMLDivElement;
  };

  @Prop()
  private entries!: string[];

  private consoleLines: ConsoleLine[] = [];

  public constructor()
  {
    super();
  }

  public write(text: string): void
  {
    this.addConsoleLines(text);
  }

  public writeLine(text: string): void
  {
    this.write(text + "\n");
  }

  private async addConsoleLines(text: string): Promise<void>
  {
    const parsed = Ansicolor.parse(text);

    // Parse text so spaces will be preserved when rendering html
    parsed.spans.forEach((x) => { x.text = x.text.replace(" ", "\u00A0"); });

    this.consoleLines.push(
    {
      index: this.consoleLines.length,
      spans: parsed.spans,
    } as ConsoleLine);

    await this.$nextTick();

    this.$refs.console.lastElementChild?.scrollIntoView(true);
  }

  @Watch("entries")
  private onEntriesChanged(value: string[], oldValue: string[]): void
  {
    if (value.length === 0)
    {
      return;
    }

    value.forEach((x) => this.write(x));
    this.entries.splice(0, this.entries.length);
  }
}
</script>

<style scoped lang="scss">
.console-window
{
  flex-grow: 1;
  flex-shrink: 1;
  flex-basis: auto;

  overflow: auto;
  text-align: left;

  counter-reset: line;
  background: var(--secondary-back-color);
  padding: calc(1.5 * var(--universal-padding));
  margin: var(--universal-margin);
  border: .0625rem solid var(--secondary-border-color);
  border-left: .25rem solid var(--pre-color);
  border-radius: 0 var(--universal-border-radius) var(--universal-border-radius) 0;

  .console
  {
    .code-line
    {
      display: block;
      white-space: nowrap;
      font-size: .85rem;
      
      >*
      {
        font-family: Courier New, Courier, Lucida Sans Typewriter, Lucida Typewriter, monospace;
        line-height: 0;
      }

      &:before
      {
        background: transparent;
        // counter-increment: line;
        content: ">"; // counter(line);
        //display: inline-block;
        // border-right: .0625rem solid var(--pre-color);
        // padding: 0 var(--universal-padding);
        // margin-right: var(--universal-margin);
        color: var(--border-color);
        text-align: right;
      }
    }
  }
}
</style>
