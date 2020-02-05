<template>
  <div id="app">
    
    <!-- Global Header -->
    <header class="sticky">
      <router-link class="logo" to="/">TV Lift</router-link>
      <label for="drawer-control" class="button drawer-toggle persistent" style="float:right;"></label>

      <!-- Menu -->
      <input type="checkbox" id="drawer-control" class="drawer persistent">
      <nav>
        <h3>Menu</h3>
        <label for="drawer-control" class="drawer-close"></label>
        <!-- <div>
          <input style="width: 100%; margin: 0px;" placeholder="Search..." type="search" id="search-bar" oninput="search()">
        </div> -->
        <router-link v-for="route in $router.options.routes" :key="route.path" :to="route.path">{{ route.meta.displayName }}</router-link>
      </nav>
    </header>

    <!-- Main View -->
    <main>
      <router-view/>
    </main>

    <!-- Global Footer -->
    <footer>
      <p>© 2020 Maarten Thomassen | <router-link to="/about">About</router-link></p>
    </footer>

  </div>
</template>

<script lang="ts">
import { Component, Vue, Ref } from "vue-property-decorator";
import "mini.css/dist/mini-dark.min.css";

@Component({})
export default class App extends Vue
{
}
</script>

<style lang="scss">

:root
{
  --link-color: #42b983;
  --border-color: grey;

  --info-color: rgb(13, 152, 194);
  --warning-color: #ffca28;
  --error-color: #b71c1c;

  --a-link-color: var(--link-color);
  --footer-link-color: var(--link-color);;

  --header-border-color: var(--border-color);
  --footer-border-color: var(--border-color);
  --card-border-color: var(--border-color);
}

html,
body
{
  height:100%;
  margin: 0;
}

div#app
{
  
  height: 100%;

  display: flex;
  flex-direction: column;
  flex-wrap: nowrap;
  justify-content: flex-start;
  align-items: stretch;

  header
  {
    flex-grow: 0;
    flex-shrink: 0;
    flex-basis: auto;
  }

  main
  {
    flex-grow: 1;
    flex-shrink: 1;
    flex-basis: 0;
    
    overflow: hidden;
  }

  footer
  {
    flex-grow: 0;
    flex-shrink: 0;
    flex-basis: 0;

    text-align: center;
  }
}
</style>
