<template>
  <div class="console-wrapper"
      @mouseenter="consoleMouseEnter($event)"
      @mouseleave="consoleMouseLeave($event)"
      @wheel="consoleWheel($event)">

    <div class="menu" :class="{ show: isHoveringOverConsole }">
      <button class="small" @click="clear">
        <trash-2-icon size="1x"></trash-2-icon>
      </button>
      <button class="small" @click="toggleAutoscroll">
        <lock-icon v-if="autoScroll" size="1x"></lock-icon>
        <unlock-icon v-else size="1x"></unlock-icon>
      </button>
      <select v-model="selectedLevel">
        <option v-for="level in levels" :key="level" :value="level">
            {{ level }}
        </option>
      </select>
    </div>

    <div id="terminal" ref="terminal"></div>

  </div>
</template>

<script lang="ts">
import { Component, Vue, Inject, Watch } from "vue-property-decorator";
import { ConsoleService, consoleLevels, ConsoleLevels, ConsoleEntry } from "@/services/consoleService";

import { Terminal } from "xterm";
import { FitAddon } from "xterm-addon-fit";
import "xterm/css/xterm.css"

import { LockIcon, Trash2Icon, UnlockIcon } from "vue-feather-icons";
@Component({
  components: {
    LockIcon,
    Trash2Icon,
    UnlockIcon
  }
})
export default class Console extends Vue
{
  public $refs!:
  {
    terminal: HTMLDivElement
  };
  
  @Inject()
  private consoleService!: ConsoleService;

  private terminal!: Terminal;
  private fitAddon!: FitAddon;

  private levels = consoleLevels;
  private selectedLevelIdx = 0;
  private isHoveringOverConsole = false;
  private autoScroll = true;

  private get selectedLevel(): ConsoleLevels
  {
    return this.levels[this.selectedLevelIdx];
  }

  private set selectedLevel(level: ConsoleLevels)
  {
    this.selectedLevelIdx = consoleLevels.findIndex(x => x === level);
    
    // TODO: Sometimes the last written line appears after terminal.clear has finished
    this.terminal.clear();
    this.consoleService.entries
      .forEach(x =>
      {
        if(x.level.idx >= this.selectedLevelIdx)
        {
          this.terminal.write(x.text,() =>
          {
            console.log("wrote", x.text);
            if(this.autoScroll)
            {
              this.terminal.scrollToBottom();
            }
          });
        }
      });
  }

  private mounted(): void
  {
    this.terminal = new Terminal({
      disableStdin: true,
      cols: 1,
      rows: 1,
      scrollback: this.consoleService.scrollback,
      cursorStyle: "bar",
      theme:{
        background: getComputedStyle(document.documentElement).getPropertyValue("--secondary-back-color"),
        cursor: getComputedStyle(document.documentElement).getPropertyValue("--secondary-back-color"),
      }
    });

    this.fitAddon = new FitAddon();
    this.terminal.loadAddon(this.fitAddon);

    this.terminal.open(this.$refs.terminal);
    this.fitAddon.fit();

    window.addEventListener("resize", this.onWindowResize);
  }

  private beforeDestroy(): void
  {
    window.removeEventListener("resize", this.onWindowResize);
  }

  @Watch("consoleService.entries", { deep: true, immediate: true})
  private onConsoleServiceEntriesChanged(val: ConsoleEntry[]): void
  {
    if(this.terminal && val?.length)
    {
      const lastVal = val[val.length-1];

      if(lastVal.level.idx >= this.selectedLevelIdx)
      {
        this.terminal.write(lastVal.text,() =>
        {
          if(this.autoScroll)
          {
            this.terminal.scrollToBottom();
          }
        });
      }
    }
  }

  private onWindowResize(): void
  {
    this.fitAddon.fit();
  }

  private consoleMouseEnter(): void
  {
    this.isHoveringOverConsole = true;
  }

  private consoleMouseLeave(): void
  {
    this.isHoveringOverConsole = false;
  }

  private consoleWheel(): void
  {
    this.autoScroll = false;
  }

  private clear(): void
  {
    this.terminal.clear();
    this.consoleService.entries.splice(0);
  }

  private toggleAutoscroll(): void
  {
    this.autoScroll = !this.autoScroll;

    if(this.autoScroll)
    {
      this.terminal.scrollToBottom();
    }
    else
    {
      this.terminal.scrollToLine(0);
    }
  }
}
</script>

<style scoped lang="scss">
.console-wrapper
{
  position: relative;

  height: 100%;
  
  .menu
  {
    position: absolute;
    z-index: 10;
    
    right: calc(var(--universal-margin) + 18px);
    top: 0;

    background: transparent;
    opacity: 0.3;

    &.show
    {
      background: var(--back-color);
      opacity: 1;
    }

  }
}

#terminal
{
  min-height: 47px;
  height: 100%;
  z-index: 0;

  background: var(--secondary-back-color);
  margin: var(--universal-margin);
  border: .0625rem solid var(--secondary-border-color);
  border-left: .25rem solid var(--pre-color);
  border-radius: 0 var(--universal-border-radius) var(--universal-border-radius) 0;
}

</style>
